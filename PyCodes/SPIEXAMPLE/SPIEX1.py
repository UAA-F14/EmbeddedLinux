import time
import spidev

spi = spidev.SpiDev()

spi.open(0,0)
spi.mode = 0
spi.max_speed_hz = 100000
spi.bits_per_word = 8

print("Waiting data from Black Pill")

try:
    while True:
        answer = spi.xfer2([0x00])
        data=answer[0]
        letter=chr(data)
        print(f"Received: {letter}")
except KeyboardInterrupt:
    print("Interrupted")
finally:
    spi.close()
