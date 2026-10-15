#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

int i2c_open(const char *dev, uint8_t addr)
{
    int fd = open(dev, O_RDWR);
    if (fd < 0)
        return -1;

    if (ioctl(fd, I2C_SLAVE, addr) < 0)
    {
        close(fd);
        return -1;
    }
    return fd;
}

int i2c_write_reg(int fd, uint8_t reg, const uint8_t *data, size_t len)
{
    uint8_t buf[256];

    if (len > sizeof(buf) - 1)
    {
        errno = EINVAL;
        return -1;
    }

    buf[0] = reg;
    if (len > 0)
        memcpy(&buf[1], data, len);

    if (write(fd, buf, len + 1) != (ssize_t)(len + 1))
    {
        if (errno == 0) errno = EIO;
        return -1;
    }
    return 0;
}

int i2c_read_reg(int fd, uint8_t reg, uint8_t *data, size_t len)
{
    if (write(fd, &reg, 1) != 1)
        return -1;

    if (read(fd, data, len) != (ssize_t)len)
    {
        if (errno == 0) errno = EIO;
        return -1;
    }
    return 0;
}

void i2c_close(int fd)
{
    if (fd >= 0)
        close(fd);
}