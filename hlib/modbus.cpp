#include "modbus.hpp"

modbus_t* my_modbus_connect(char* tty, int baud)
{
    modbus_t* ctx = modbus_new_rtu(tty, baud, 'E', 8, 1);
    if (ctx == NULL) {
        return NULL;
    }
    int rc = modbus_connect(ctx);
    if (rc < 0) {
        modbus_free(ctx);
        return NULL;
    }
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    modbus_set_response_timeout(ctx, &timeout);
    return ctx;
}

int my_modbus_read_reg(modbus_t *ctx, uint8_t addr, uint16_t reg)
{
    int rc;
    uint16_t buf;
    rc = modbus_set_slave(ctx, addr);
    if (rc < 0) {
        return rc;
    }
    rc = modbus_read_registers(ctx, reg, 1, &buf);
    if (rc < 0) {
        return rc;
    }
    return buf;
}

int my_modbus_write_reg(modbus_t *ctx, uint8_t addr, uint16_t reg, uint16_t val)
{
    int rc;
    rc = modbus_set_slave(ctx, addr);
    if (rc < 0) {
        return rc;
    }
    rc = modbus_write_register(ctx, reg, val);
    return rc;
}

int my_modbus_check_identification(modbus_t *ctx, uint8_t addr, uint16_t ident)
{
    int rc;
    for (int i=0; i<10; i++) {
        rc = my_modbus_read_reg(ctx, addr, 0x00);
        if (rc > 0) {
            break;
        }
    }
    if (rc < 0) {
        return rc;
    }
    if (rc != ident) {
        return rc;
    }
    return 0;
}
