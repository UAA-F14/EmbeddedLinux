#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stddef.h>

int  i2c_open(const char *dev, uint8_t addr);   // devuelve fd o -1
int  i2c_write_reg(int fd, uint8_t reg, const uint8_t *data, size_t len);
int  i2c_read_reg(int fd, uint8_t reg, uint8_t *data, size_t len);
void i2c_close(int fd);

#endif