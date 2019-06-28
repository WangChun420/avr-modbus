# Makefile for Modbus Bootloader

include ../common.mk

MY_MAGIC = 0xFF00 # Bootloader

TARGET = $(lastword $(subst /, ,$(CURDIR)))
SOURCES = $(wildcard $(LIBS)/*.c) main.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SOURCES:.c=.h)


LDFLAGS += -Wl,--section-start=.text=$(BOOT_BBASE)
#LDFLAGS += -nostartfiles -nodefaultlibs

#### Build bootloader
all: $(TARGET).hex size

%.o: %.c $(HEADERS) Makefile
	@ $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	@ $(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

%.hex: %.elf
	@ $(OBJCOPY) -j .text -j .data -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

size: $(TARGET).elf
	$(AVRSIZE) -C --mcu=$(MCU) $(TARGET).elf

clean:
	rm -rf *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom
	rm -rf ../lib/*.o

