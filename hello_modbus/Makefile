# Makefile for modbus application

include ../common.mk

MY_MAGIC = 0xFF01 # hello magic test tool

TARGET = $(lastword $(subst /, ,$(CURDIR)))
SOURCES := $(wildcard $(LIBS)/*.c) main.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SOURCES:.c=.h)

all: $(TARGET).hex size client

%.o: %.c $(HEADERS) Makefile
	@ $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	@ $(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@

%.hex: %.elf
	@ $(OBJCOPY) -j .text -j .data -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

size: $(TARGET).elf
	$(AVRSIZE) -C --mcu=$(MCU) $(TARGET).elf

clean:
	rm -rf *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom
	rm -rf ../lib/*.o
	rm -f client

#### Build Client
client: client.cpp ../hlib/modbus.cpp ../hlib/modbus.hpp
	gcc -o client client.cpp ../hlib/modbus.cpp -I../hlib -lmodbus -lstdc++
