# Common definitions for whole project

PROGRAMMER = -c usbtiny -P usb
AVRDUDE = avrdude $(PROGRAMMER) -p $(MCU)

MCU = atmega328p
F_CPU = 16000000
BAUD = 19200
RXTX_PIN = PB5

BOOT_BBASE = 0x7800
BOOT_WBASE = 0x3C00

LFUSE = 0xFF
HFUSE = 0xCA
EFUSE = 0xFF
FUSES = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

CC = avr-gcc
CXX = $(CC)
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size

LIBS = ../lib

CPPFLAGS = -DMY_MAGIC=$(MY_MAGIC) -DBOOT_WBASE=$(BOOT_WBASE) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I. -I$(LIBS)
CFLAGS = -Os -g -std=gnu99 -Wall
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections -flto

LDFLAGS = -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--relax
TARGET_ARCH = -mmcu=$(MCU)

.PHONY: default
default: all

#### Download application
.PHONY: load
load: all
	make -C ../bootloader-flash
	../bootloader-flash/bootloader-flash -d /dev/ttyUSB0 -f $(TARGET).hex -a $(ADDRESS)

#### Flash application
.PHONY: flash
flash: all
	$(AVRDUDE) -F -U flash:w:$(TARGET).hex:i

#### FUSE and ERASE
.PHONY: fuse
fuse:
	$(AVRDUDE) $(FUSES) -U lock:w:0xFF:m -e

.PHONY: erase
erase:
	$(AVRDUDE) -e
