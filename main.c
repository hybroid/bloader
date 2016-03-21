/*
 * main.c
 *
 *  Created on: 04 авг. 2015 г.
 *      Author: Tony Darko
 */

#define VERSION_MAJOR '0'
#define VERSION_MINOR '1'

#define BAUD 38400
#define WAIT_TIME 250 // 2.5s

#define BOOTSIZE 512

#define SUPPORT_EEPROM 0

#include "hal.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/setbaud.h>

#define DEVTYPE DEVCODE


__attribute__((naked,section(".init2")))
void __init(void)
{
    asm volatile ( "clr __zero_reg__" );        // r1 set to 0
    //SREG = 0;
    //asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );
#if defined(__AVR_ATmega8__)// || defined(__AVR_ATmega32__)
    SP = RAMEND;								// set stack pointer to RAMEND
#endif
}

int main(void) __attribute__((OS_main,section(".init9")));

static void (*jump_to_app)(void) = 0x0000;

static inline void uart_sendchar(uint8_t data)
{
	while (!(UCSRA & (1<<UDRE)));
	UDR = data;
}

static inline uint8_t uart_recvchar(void)
{
	while (!(UCSRA & (1<<RXC)));
	return UDR;
}

uint8_t buff[SPM_PAGESIZE];

static void uart_recv_buffer(pagebuf_t size)
{
	pagebuf_t count;
	uint8_t *tmp = buff;

	for (count = 0; count <sizeof(buff); count++)
	{
		*tmp++ = (count < size) ? uart_recvchar() : 0xFF;
	}
}

static inline void send_boot(void)
{
	uart_sendchar('A');
	uart_sendchar('V');
	uart_sendchar('R');
	uart_sendchar('1');
	uart_sendchar('0');
	uart_sendchar('9');
}

static inline uint16_t write_flash_page(uint16_t addr, pagebuf_t size)
{
	uint32_t pagestart = (uint32_t)addr << 1;
	uint32_t byte_addr = pagestart;
	uint16_t data;
	uint8_t *tmp = buff;

	do
	{
		data = *tmp++;
		data |= *tmp++ << 8;
		boot_page_fill(byte_addr, data);
		byte_addr += 2;
		size -= 2;
	} while (size);

	boot_page_write(pagestart);
	boot_spm_busy_wait();
	boot_rww_enable();

	return byte_addr >> 1;
}

static inline uint16_t read_flash_page(uint16_t addr, pagebuf_t size)
{
	uint32_t byte_addr = (uint32_t)addr << 1;
	uint8_t data;

	do
	{
		if (byte_addr < APP_END) // don't read bootloader
		{
#if defined(RAMPZ)
			data = pgm_read_byte_far(byte_addr);
#else
			data = pgm_read_byte_near( (uint16_t)byte_addr );
#endif
		}
		else
		{
			data = 0xFF; // empty out
		}

		uart_sendchar( data );

		byte_addr++;
		size--;
	} while (size);

	return byte_addr >> 1;
}

#if defined(SUPPORT_EEPROM) && (SUPPORT_EEPROM == 1)

static inline uint16_t write_eeprom_page(uint16_t addr, pagebuf_t size)
{
	uint8_t *tmp = buff;

	do
	{
		eeprom_write_byte( (uint8_t*)addr, *tmp++ );
		addr++;
		size--;
	} while (size);

	// TODO: Attention to this!
	//eeprom_busy_wait();

	return addr;
}

static inline uint16_t read_eeprom_page(uint16_t addr, pagebuf_t size)
{
	do
	{
		uart_sendchar( eeprom_read_byte((uint8_t*)addr) );

		addr++;
		size--;
	} while (size);

	return addr;
}

#endif

static inline void erase_flash(void)
{
	uint16_t addr = 0;
	while (addr < APP_END)
	{
			boot_page_erase(addr);
			boot_spm_busy_wait();
			addr += SPM_PAGESIZE;
	}
	boot_rww_enable();
}

int main(void)
{
	// disable watchdog
	wdt_disable();

	// disable interrupts
	cli();

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
#if USE_2X
	UCSRA |= (1<<U2X);
#endif
	UCSRB |= ((1<<RXEN) | (1<<TXEN));
	UCSRC |= ((1<<UCSZ1) | (1<<UCSZ0));

	// Waiting at start
	uint16_t wait_cnt = 0;

	while (1)
	{
		if (UCSRA & (1<<RXC))
		{
			if (UDR == WAIT_CHAR)
			{
				break;
			}
		}

		if (wait_cnt++ == WAIT_TIME)
		{
			jump_to_app();
		}

		_delay_ms(10);
	}

	send_boot();

	uint8_t val;
	uint16_t addr = 0;
	uint8_t device = 0;

	while(1)
	{
		val = uart_recvchar();

		if (val == 'a') // autoincrement?
		{
			uart_sendchar('Y');
		}
		else if (val == 'A') // write address
		{
			addr = uart_recvchar();
			addr = (addr<<8) | uart_recvchar();
			uart_sendchar('\r');
		}
		else if (val == 'b') // buffer load support
		{
			uart_sendchar('Y');
			uart_sendchar((sizeof(buff)>>8) & 0xFF);
			uart_sendchar(sizeof(buff) & 0xFF);
		}
		else if (val == 'B') // buffer load
		{
			pagebuf_t size;
			size = uart_recvchar() << 8;
			size |= uart_recvchar();
			val = uart_recvchar(); // load memory type ('E' or 'F')
			uart_recv_buffer(size);

			if (device == DEVTYPE)
			{
				if (val == 'F')
				{
					addr = write_flash_page(addr, size);
				}
#if defined(SUPPORT_EEPROM) && SUPPORT_EEPROM == 1
				else if (val == 'E')
				{
					addr = write_eeprom_page(addr, size);
				}
#endif
				uart_sendchar('\r');
			}
			else
			{
				uart_sendchar(0);
			}
		}
		else if (val == 'g') // block read
		{
			pagebuf_t size;
			size = uart_recvchar() << 8;
			size |= uart_recvchar();
			val = uart_recvchar(); // load memory type ('E' or 'F')

			if (val == 'F')
			{
				addr = read_flash_page(addr, size);
			}
#if defined(SUPPORT_EEPROM) && SUPPORT_EEPROM == 1
			else if (val == 'E')
			{
				addr = read_eeprom_page(addr, size);
			}
#endif
		}
		else if (val == 'e') // chip erase
		{
			if (device == DEVTYPE)
			{
				erase_flash();
			}
			uart_sendchar('\r');
		}
		else if (val == 'E') // exit upgrade
		{
			// TODO: reset at this?
			uart_sendchar('\r');
			jump_to_app();
		}
		else if (val == 'P' || val == 'L') // enter or leave programm mode
		{
			uart_sendchar('\r');
		}
		else if (val == 'p') // program type
		{
			uart_sendchar('S'); // serial programmer
		}
		else if (val == 't') // return device type
		{
			uart_sendchar(DEVTYPE);
			uart_sendchar(0);
		}
		else if (val == 'x' || val == 'y') // clear & set LED
		{
			uart_recvchar();
			uart_sendchar('\r');
		}
		else if (val == 'T') // set device
		{
			device = uart_recvchar();
			uart_sendchar('\r');
		}
		else if (val == 'S') // return software ID
		{
			send_boot();
		}
		else if (val == 'V') // software version
		{
			uart_sendchar(VERSION_MAJOR);
			uart_sendchar(VERSION_MINOR);
		}
		else if (val == 's') // signature bytes
		{
			uart_sendchar(SIGNATURE_2);
			uart_sendchar(SIGNATURE_1);
			uart_sendchar(SIGNATURE_0);
		}
		else if (val != 0x1b) // ESC??!
		{
			uart_sendchar('?');
		}
	}

	return 0;
}
