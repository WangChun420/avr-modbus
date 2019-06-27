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
    if (argc != 2) {
        exit(1);
    }

    modbus_t *ctx = my_modbus_connect(argv[1], 19200);
    if (ctx == NULL) {
        fprintf(stderr, "error connecting to %s\n\r", argv[1]);
        exit(1);
    }

    uint8_t addr = 0x12;

    fprintf(stdout, "Connecting to device with address 0x%02x\n\r", addr);
    rc = my_modbus_check_identification(ctx, addr, 0xFF01);
    if (rc) {
        fprintf(stderr, "  failed to connect with correct id, %d\n\r", rc);
        exit(1);
    }

    fprintf(stdout, "LED Dancing\n\r");
    while (1) {
        for (int i=0; i<100; i++) {
            rc = my_modbus_write_reg(ctx, addr, 0x01, 17);
            if (rc < 0) {
                fprintf(stderr, "error writing reg, %d\n\r", rc);
            }
            usleep(i * 500);
        }
        for (int i=100; i>0; i--) {
            rc = my_modbus_write_reg(ctx, addr, 0x01, 17);
            if (rc < 0) {
                fprintf(stderr, "error writing reg, %d\n\r", rc);
            }
            usleep(i * 500);
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
