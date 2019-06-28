#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>

#include "modbus.hpp"

const int EXCEPTION_RC = 2;

enum { TCP, TCP_PI, RTU };

#define SERVER_ID 1

int main(int argc, char *argv[]) {
    int rc;
    uint8_t server_id = 247; // default is a unprogrammed device id
    uint8_t new_id = 0;
    modbus_t *ctx;
    FILE *fhex;
    int baudrate = 19200;
    char *device, *file;
    uint16_t pagesize, flashsize;
    uint16_t buf[256];
    int verbose = 0;

    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");
    printf("  AVR MODBUS Bootloader Flash Tool                          \n\r");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");

    /* getopt */
    int opt;
    while ((opt = getopt(argc, argv, "a:A:b:d:f:ht:v")) != -1) {
        switch (opt) {
        case 'a':
            server_id = strtol(optarg, NULL, 0);
            break;
        case 'A':
            new_id = strtol(optarg, NULL, 0);
            break;
        case 'b':
            baudrate = strtol(optarg, NULL, 0);
            break;
        case 'd':
            device = optarg;
            break;
        case 'f':
            file = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
        default:
            fprintf(stderr, "Usage: %s [...]\n\r", argv[0]);
            fprintf(stderr, "  -a : Address of Device\n\r");
            fprintf(stderr, "  -A : Set new address after programming\n\r");
            fprintf(stderr, "  -b : Baudrate\n\r");
            fprintf(stderr, "  -d : TTY Device\n\r");
            fprintf(stderr, "  -f : Firmware HEX-File\n\r");
            fprintf(stderr, "  -t : Check Identification\n\r");
            fprintf(stderr, "  -v : Verbose\n\r");
            exit(1);
        }
    }

    printf("  HEX-File:      %s\n\r", file);
    printf("  MODBUS Device: %s (%d, 8E1)\n\r", device, baudrate);
    printf("  MODBUS Slave:  0x%02x\n\r", server_id);
    if (new_id > 0) {
        printf("    Setting NEW MODBUS Slave Address to 0x%02x\n\r", new_id);
    }
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");


    ctx = my_modbus_connect(device, baudrate);
    if (ctx == NULL) {
        fprintf(stderr, "error connecting to device %s, %s\n", device, modbus_strerror(errno));
        exit(1);
    }

    fhex = fopen(file, "r");
    if (0 == fhex) {
        fprintf(stderr, "failed to open hexfile %s\n\r", file);
        exit(1);
    }

    if (verbose) {
        modbus_set_debug(ctx, TRUE);
    } else {
        modbus_set_debug(ctx, FALSE);
    }

    printf("Connecting to device with address 0x%02x\n\r", server_id);
    rc = my_modbus_check_identification(ctx, server_id, 0xFF00);
    if (rc > 0) { // device available but not a bootloader
        printf("  found device with ID=0x%04x, resetting\n\r", rc);
        rc = my_modbus_write_reg(ctx, server_id, 0xFF, 0x42);
        if (rc < 0) {
            fprintf(stderr, "  error resetting device, %d\n\r", rc);
            exit(1);
        }
    } else if (rc < 0) {
        fprintf(stderr, "  failed to connect\n\r");
        exit(1);
    }

    // recheck identification
    rc = my_modbus_check_identification(ctx, server_id, 0xFF00);
    if (rc) {
        fprintf(stderr, "  failed to connect after reset, 0x%04x\n\r", rc);
        exit(1);
    }

    // prevent bootloader from resetting to application
    rc = my_modbus_write_reg(ctx, server_id, 0x00, 0x42);
    if (rc < 0) {
        fprintf(stderr, "  error preventing bootloader from reset, %d\n\r", rc);
        exit(1);
    }

    // get page and flashsize
    rc = my_modbus_read_reg(ctx, server_id, 0x01);
    if (rc < 0) {
        fprintf(stderr, "Failed to read pagesize: %s\n", modbus_strerror(errno));
        exit(1);
    }
    pagesize = rc;
    rc = my_modbus_read_reg(ctx, server_id, 0x02);
    if (rc < 0) {
        fprintf(stderr, "Failed to read flashsize: %s\n", modbus_strerror(errno));
        exit(1);
    }
    flashsize = rc;


    // Read the hexfile to flash
    int chr;
    int addr = 0;
    std::vector<uint8_t> dataflash;
    int baseaddr = -1;
    char buf8[16];
    while ( (chr = fgetc(fhex)) != EOF ) {
        // Start of line
        if (chr == ':') {
            // get line length
            for (int i=0; i<2; i++) {
                buf8[i] = fgetc(fhex);
            }
            buf8[2] = '\0';
            int num = (int)strtol(buf8, NULL, 16);

            // get address
            for (int i=0; i<4; i++) {
                buf8[i] = fgetc(fhex);
            }
            buf8[4] = '\0';
            if ( (int)strtol(buf8, NULL, 16) < addr) {
                break;
            }

            // store first address
            addr = (int)strtol(buf8, NULL, 16);
            if (baseaddr == -1) {
                baseaddr = addr;
            }

            // get content type
            for (int i=0; i<2; i++) {
                buf8[i] = fgetc(fhex);
            }
            buf8[2] = '\0';
            int typehex = (int)strtol(buf8, NULL, 16);
            if (typehex > 1) {
                fprintf(stderr, "Only IHEX8 supported\n\r");
                exit(1);
            }

            buf8[2] = '\0';
            for (int i=0; i<num; i++) {
                buf8[0] = fgetc(fhex);
                buf8[1] = fgetc(fhex);
                uint8_t val = (uint8_t)strtol(buf8, NULL, 16);
                dataflash.push_back(val);
            }
        }
    }
    fclose(fhex);

    // pad buffer with '1'
    while (dataflash.size() % 128) {
        dataflash.push_back(0xFF);
    }

    if (pagesize != 128) {
        fprintf(stderr, "unsuported pagesize %d\n\r", pagesize);
        exit(1);
    }
    if (flashsize < dataflash.size()) {
        fprintf(stderr, "flash (%d) smaller then hexfile (%lu)\n\r",
                flashsize, dataflash.size());
        exit(1);
    }

    // send pages to bootloader
    printf("Flashing: ");
    addr = baseaddr;
    for (int f=0; f<dataflash.size(); ) {
        for (int i=0; i<64; i++) {
            buf[i] = dataflash[f++];
            buf[i] |= dataflash[f++] << 8;
        }
        rc = modbus_write_registers(ctx, addr, 64, buf);
        if (rc < 0) {
            fprintf(stderr, "Failed to write %d\n\r", addr);
            exit(1);
        }
        addr += 64;
        printf(".");
    }
    printf("\n\r");

    if (new_id > 0) {
        printf("Writing new MODBUS Address 0x%02x to EEPROM\n\r", new_id);
        rc = my_modbus_write_reg(ctx, server_id, 0x8000, new_id);
        if (rc < 0) {
            fprintf(stderr, "  failed to write new address, %d\n\r", rc);
            exit(1);
        }
    }

    printf("Resetting controller to load firmware\n\r");
    rc = my_modbus_write_reg(ctx, server_id, 0xFF, 0x42);
    if (rc < 0) {
        fprintf(stderr, "  failed to reset from bootloader, 0x%04x\n\r", rc);
        exit(1);
    }

    // if we programmed a new id this is valid after restart
    if (new_id) {
        server_id = new_id;
    }

    for (int i=0; i<10; i++) {
        rc = my_modbus_read_reg(ctx, server_id, 0x00);
        if (rc > 0) {
            break;
        }
    }
    if (rc < 0) {
        fprintf(stderr, "  failed to connect to application, is it broken?, %d\n\r", rc);
    }
    if (rc == 0xFF00) {
        fprintf(stderr, "  resettet but back in bootloader?\n\r");
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
