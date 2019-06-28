BOOTLOADER := bootloader bootloader-flash
APPS := hello_modbus zisterne_fuellstand

all: $(BOOTLOADER) $(APPS)

$(BOOTLOADER):
	make -C $@

$(APPS):
	make -C $@

.PHONY: all $(BOOTLOADER) $(APPS)
