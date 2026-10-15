#include <stdio.h>
#include <stdint.h>
#include "i2c.h"

#define I2C_DEV "/dev/i2c-0"   // sudo i2cdetect -l
#define ADDR    0x48           // Adddress of the I2C device

int main(void)
{
    int fd = i2c_open(I2C_DEV, ADDR);
    if (fd < 0)
    {
        perror("i2c_open");
        return 1;
    }

    uint8_t v;
    if (i2c_read_reg(fd, 0x00, &v, 1) == 0)
        printf("reg 0x00 = 0x%02X\n", v);
    else
        perror("read");

    uint8_t val = 0x80;
    if (i2c_write_reg(fd, 0x01, &val, 1) < 0)
        perror("write");

    i2c_close(fd);
    return 0;
}