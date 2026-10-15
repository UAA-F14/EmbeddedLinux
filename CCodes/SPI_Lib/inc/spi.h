#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    int      fd;
    uint8_t  mode;        // 0..3 (CPOL/CPHA)
    uint8_t  bits;        // bits por palabra, normalmente 8
    uint32_t speed_hz;    // velocidad máxima del reloj
} spi_t;

int  spi_open(spi_t *spi, const char *dev, uint8_t mode, uint8_t bits, uint32_t speed_hz);
int  spi_transfer(spi_t *spi, const uint8_t *tx, uint8_t *rx, size_t len);
int  spi_write(spi_t *spi, const uint8_t *tx, size_t len);
int  spi_read(spi_t *spi, uint8_t *rx, size_t len);
int  spi_write_reg(spi_t *spi, uint8_t reg, const uint8_t *data, size_t len);
int  spi_read_reg(spi_t *spi, uint8_t reg, uint8_t *data, size_t len);
void spi_close(spi_t *spi);

#endif