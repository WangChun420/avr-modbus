# Makefile for modbus application

MCU = atmega328p
F_CPU = 16000000
BAUD = 19200
RXTX_PIN = PB5
MY_MAGIC = 0xFF01 # hello magic test tool

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

CPPFLAGS = -DMY_MAGIC=$(MY_MAGIC) -DBOOTLOADER=0x3C00 -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I. -I$(LIBS)
CFLAGS = -Os -g -std=gnu99 -Wall
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections -flto

LDFLAGS = -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--relax

TARGET_ARCH = -mmcu=$(MCU)

#### Build application
all: $(TARGET).hex size

client: client.cpp ../hlib/modbus.cpp ../hlib/modbus.hpp
	gcc -o client client.cpp ../hlib/modbus.cpp -I../hlib -lmodbus -lstdc++


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


#### Flash the application
flash: all
	$(AVRDUDE) -F -U flash:w:$(TARGET).hex:i

bootload: all
	../bootloader-flash/bootloader-flash -d /dev/ttyUSB0 -f $(TARGET).hex
