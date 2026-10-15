#include <stdio.h>
#include <string.h>
#include "spi.h"

int main(void)
{
    spi_t spi;

    if (spi_open(&spi, "/dev/spidev1.0", 0, 8, 1000000) < 0)   // tu bus/CS
    {
        perror("spi_open");
        return 1;
    }

    uint8_t tx[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    uint8_t rx[4] = { 0 };

    if (spi_transfer(&spi, tx, rx, sizeof(tx)) < 0)
    {
        perror("spi_transfer");
        spi_close(&spi);
        return 1;
    }

    printf("rx: %02X %02X %02X %02X\n", rx[0], rx[1], rx[2], rx[3]);
    printf("loopback %s\n", memcmp(tx, rx, sizeof(tx)) == 0 ? "OK" : "FALLO");

    spi_close(&spi);
    return 0;
}