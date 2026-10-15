#include <stdio.h>
#include <unistd.h>
#include "spi.h"

#define FRAME_LEN 6

int main(void)
{
    spi_t spi;

    if (spi_open(&spi, "/dev/spidev1.0", 0, 8, 500000) < 0)   // tu bus/CS
    {
        perror("spi_open");
        return 1;
    }

    while (1)
    {
        uint8_t tx[FRAME_LEN] = { 0 };   // el maestro manda ceros; solo genera reloj
        uint8_t rx[FRAME_LEN];

        if (spi_transfer(&spi, tx, rx, FRAME_LEN) < 0)
        {
            perror("spi_transfer");
            break;
        }

        uint8_t cs = 0;
        for (int i = 0; i < FRAME_LEN - 1; i++)
            cs ^= rx[i];

        if (rx[0] == 0xA5 && cs == rx[FRAME_LEN - 1])
            printf("seq=%3u  value=%5u  errs=%u\n",
                   rx[1], (rx[2] << 8) | rx[3], rx[4]);
        else
            printf("frame invalido\n");

        usleep(10000);                   // 10 ms entre frames
    }

    spi_close(&spi);
    return 0;
}