# avr-modbus
Simple AVR Modbus Bootloader + Applications for "smarthome"


For all my little tiny sensors at home I decided to use good old MODBUS.

Why? I wanted to use RS485 in any case and there are multiple sensors out there talking MODBUS.

# Protocol Definitions

Uses a basic MODBUS implementation (only the very basic commandset).

## Implemented in modbus library per default

### Register Read (0x03)
        * 0x00: Identification
                * 0xFF00: Bootlader
                * 0xFF01: hello_modbus

### Write Single Register (0x06)
        * 0xFE: Set Modbus Address

## MUST be implemented by every firmware

### Write Single Register (0x06)
        * 0xFF: Reset (Magic Value = 0x42)

# bootloader

Very basic MODBUS bootloader to upload firmware

## Usasge

```bash
$ make
$ make erase
$ make fuse
$ make flash
```

# bootloader-flash

Simple host side tool to download HEX-Files (applications) using the bootloader

## Usage

```bash
$ make
$ ./bootloader-flash -d /dev/ttyUSB0 -f ../hello_modbus/hello_modbus.hex -A 12 # Flash firmware and set MODBUS address
$ ./bootloader-flash -d /dev/ttyUSB0 -f ../hello_modbus/hello_modbus.hex -a 12 # Flash firmware to specified MODBUS address
```

# hello_modbus

Very basic MODBUS App example where a LED can be toggled (... magic!)
There is a simple client in there also which fades the blinking.

## Usasge

```bash
$ make
# Upload using bootloader
$ make client
$ ./client /dev/ttyUSB0
```
