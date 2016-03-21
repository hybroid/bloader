/*
 * hal.h
 *
 *  Created on: 05 авг. 2015 г.
 *      Author: Tony Darko
 */

#ifndef HAL_H_INCLUDED
#define HAL_H_INCLUDED

#include <avr/io.h>

#if !defined(F_CPU)
#define F_CPU 8000000
#endif

#if !defined(BAUD)
#define BAUD 38400
#endif

#if !defined(SUPPORT_EEPROM)
#define SUPPORT_EEPROM 1
#endif

#if !defined(SIGNATURE_0) && !defined(SIGNATURE_1) && !defined(SIGNATURE_2)
#	error "No definition for MCU!"
#endif

#if defined(SPMCSR)
#define SPM_REG SPMCSR

#elif defined(SPMCR)
#define SPM_REG SPMCR

#else
#error "MCU does not provide bootloader support!"
#endif

#if (SPM_PAGESIZE > UINT8_MAX)
typedef uint16_t pagebuf_t;
#else
typedef uint8_t pagebuf_t;
#endif

#if !defined(BOOTSIZE)
#define BOOTSIZE 512	// 512 words (1024 bytes)
#endif

#define APP_END (FLASHEND - (BOOTSIZE * 2))

#if !defined(WAIT_TIME)
#define WAIT_TIME 100 // Wait WAIT_TIME * 10ms, by default 1000ms = 1s
#endif

#if !defined(WAIT_CHAR)
#define WAIT_CHAR 'S'
#endif

// UART registers
#if defined(__AVR_ATmega88__) || defined(__AVR_ATmega88A__)
#define DEVCODE 0x73
#define UDR			UDR0
#define UCSRA		UCSR0A
#	define RXC		RXC0
#	define TXC		TXC0
#	define UDRE		UDRE0
#	define FE		FE0
#	define DOR		DOR0
#	define UPE		UPE0
#	define U2X		U2X0
#	define MPCM		MPCM0
#define UCSRB		UCSR0B
#	define RXCIE	RXCIE0
#	define TXCIE	TXCIE0
#	define UDRIE	UDRIE0
#	define RXEN		RXEN0
#	define TXEN		TXEN0
#	define UCSZ2	UCSZ02
#	define RXB8		RXB80
#	define TXB8		TXB80
#define UCSRC		UCSR0C
#	define UMSEL1	UMSEL01
#	define UMSEL0	UMSEL00
#	define UPM1		UPM01
#	define UPM0		UPM00
#	define USBS		USBS0
#	define UCSZ1	UCSZ01
#	define UCSZ0	UCSZ00
#	define UCPOL	UCPOL0
#define UBRR		UBRR0
#	define UBRRH	UBRR0H
#	define UBRRL	UBRR0L

#else
#	error "Bootloader code does not support this MCU"
#endif



#endif /* HAL_H_INCLUDED */
