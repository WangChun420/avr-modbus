# Description of Modbus Registers used

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
