################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
bloader.lss \
bloader.lss \

FLASH_IMAGE += \
bloader.hex \
bloader.hex \

EEPROM_IMAGE += \
bloader.eep \
bloader.eep \

SIZEDUMMY += \
sizedummy \
sizedummy \


# All Target
all: bloader.elf secondary-outputs

# Tool invocations
bloader.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,bloader.map -Wl,--gc-sections -Wl,--relax -nostartfiles -Wl,--section-start=.text=0x1C00 -mmcu=atmega88a -o "bloader.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

bloader.lss: bloader.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S bloader.elf  >"bloader.lss"
	@echo 'Finished building: $@'
	@echo ' '

bloader.hex: bloader.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex bloader.elf  "bloader.hex"
	@echo 'Finished building: $@'
	@echo ' '

bloader.eep: bloader.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex bloader.elf  "bloader.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: bloader.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega88a bloader.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) bloader.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
