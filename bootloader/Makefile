# Makefile for Modbus Bootloader

MCU = atmega328p
F_CPU = 16000000
BAUD = 19200
RXTX_PIN = PB5
MY_MAGIC = 0xFF00 # bootloader

LFUSE = 0xFF
HFUSE = 0xCA
EFUSE = 0xFF
FUSES = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m
BOOTLOADER_BADDRESS = 0x7800
PROGRAMMER = -c usbtiny -P usb
AVRDUDE = avrdude $(PROGRAMMER) -p $(MCU)



LIBS = ../lib

CC = avr-gcc
CXX = $(CC)
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size

TARGET = $(lastword $(subst /, ,$(CURDIR)))
SOURCES = main.c ../lib/modbus.c ../lib/crc16.c ../lib/common.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SOURCES:.c=.h)

CPPFLAGS = -DMY_MAGIC=$(MY_MAGIC) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I. -I$(LIBS)
CFLAGS = -Os -g -std=gnu99 -Wall
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections -flto

LDFLAGS = -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--relax
LDFLAGS += -Wl,--section-start=.text=$(BOOTLOADER_BADDRESS)

TARGET_ARCH = -mmcu=$(MCU)

#### Build bootloader
all: $(TARGET).hex size

%.o: %.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

size: $(TARGET).elf
	$(AVRSIZE) -C --mcu=$(MCU) $(TARGET).elf

clean:
	rm -rf *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom
	rm -rf ../lib/*.o


#### Flash the bootloader
flash: all
	$(AVRDUDE) -F -U flash:w:$(TARGET).hex:i

fuse:
	$(AVRDUDE) $(FUSES) -U lock:w:0xFF:m -e

erase:
	$(AVRDUDE) -e
