#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spi.h"

#define SPI_MAX_XFER 256

int spi_open(spi_t *spi, const char *dev, uint8_t mode, uint8_t bits, uint32_t speed_hz)
{
    spi->fd = open(dev, O_RDWR);
    if (spi->fd < 0)
        return -1;

    spi->mode     = mode;
    spi->bits     = bits;
    spi->speed_hz = speed_hz;

    if (ioctl(spi->fd, SPI_IOC_WR_MODE, &spi->mode) < 0 ||
        ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &spi->bits) < 0 ||
        ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi->speed_hz) < 0)
    {
        close(spi->fd);
        spi->fd = -1;
        return -1;
    }

    return 0;
}

int spi_transfer(spi_t *spi, const uint8_t *tx, uint8_t *rx, size_t len)
{
    struct spi_ioc_transfer t;

    memset(&t, 0, sizeof(t));
    t.tx_buf        = (unsigned long)tx;
    t.rx_buf        = (unsigned long)rx;
    t.len           = (uint32_t)len;
    t.speed_hz      = spi->speed_hz;
    t.bits_per_word = spi->bits;
    t.delay_usecs   = 0;
    t.cs_change     = 0;

    return ioctl(spi->fd, SPI_IOC_MESSAGE(1), &t) < 0 ? -1 : 0;
}

int spi_write(spi_t *spi, const uint8_t *tx, size_t len)
{
    return spi_transfer(spi, tx, NULL, len);
}

int spi_read(spi_t *spi, uint8_t *rx, size_t len)
{
    return spi_transfer(spi, NULL, rx, len);
}

/* Patrón típico de registros: 1 byte de dirección + datos, con CS bajo todo el tiempo. */
int spi_write_reg(spi_t *spi, uint8_t reg, const uint8_t *data, size_t len)
{
    uint8_t tx[SPI_MAX_XFER];

    if (len > sizeof(tx) - 1)
    {
        errno = EINVAL;
        return -1;
    }

    tx[0] = reg;
    if (len > 0)
        memcpy(&tx[1], data, len);

    return spi_transfer(spi, tx, NULL, len + 1);
}

int spi_read_reg(spi_t *spi, uint8_t reg, uint8_t *data, size_t len)
{
    uint8_t tx[SPI_MAX_XFER] = { 0 };
    uint8_t rx[SPI_MAX_XFER];

    if (len > sizeof(tx) - 1)
    {
        errno = EINVAL;
        return -1;
    }

    tx[0] = reg;                         // el resto va en 0 para generar reloj
    if (spi_transfer(spi, tx, rx, len + 1) < 0)
        return -1;

    memcpy(data, &rx[1], len);           // rx[0] es lo que llegó mientras se mandaba reg
    return 0;
}

void spi_close(spi_t *spi)
{
    if (spi->fd >= 0)
    {
        close(spi->fd);
        spi->fd = -1;
    }
}