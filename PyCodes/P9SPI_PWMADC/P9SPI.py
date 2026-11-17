import time

import spidev
import wiringpi

SPI_BUS = 3                 # SPI3 (pines 19, 21, 23, 24)
SPI_DEVICE = 0              # CE0 (pin 24)
SPI_SPEED_HZ = 1_000_000
HEADER = 0xA5
N_CHANNELS = 3
FRAME_LEN = 1 + 2 * N_CHANNELS

ADC_MAX_VALUE = 0x0FFF
ADC_REFERENCE_VOLTAGE = 3.31

PWM_PINS = (29, 33, 35)
PWM_RANGE = 100


def read_adcs(spi):
    """Lee una trama [HEADER, 3 x 12 bits LE]. Devuelve None si es inválida."""
    rx = spi.xfer2([0x00] * FRAME_LEN)
    if rx[0] != HEADER:
        return None
    raw = bytes(rx[1:])
    return [
        int.from_bytes(raw[2 * i : 2 * i + 2], "little") & ADC_MAX_VALUE
        for i in range(N_CHANNELS)
    ]


def adc2pwm(adc):
    """ADC de 12 bits (0-4095) -> paso de softPwm (0-PWM_RANGE)."""
    adc = max(0, min(ADC_MAX_VALUE, adc))
    return round(adc * PWM_RANGE / ADC_MAX_VALUE)


wiringpi.wiringPiSetupPhys()
for pin in PWM_PINS:
    wiringpi.softPwmCreate(pin, 0, PWM_RANGE)

spi = spidev.SpiDev()
spi.open(SPI_BUS, SPI_DEVICE)
spi.max_speed_hz = SPI_SPEED_HZ
spi.mode = 0b00             # CPOL=0, CPHA=0

try:
    while True:
        values = read_adcs(spi)
        if values is None:
            print("Trama inválida")
            time.sleep(0.1)
            continue

        parts = []
        for i, (pin, adc) in enumerate(zip(PWM_PINS, values)):
            step = adc2pwm(adc)
            wiringpi.softPwmWrite(pin, step)
            voltage = adc / ADC_MAX_VALUE * ADC_REFERENCE_VOLTAGE
            parts.append(
                f"CH{i}: {adc:4d} ({voltage:.2f} V -> {step / PWM_RANGE * 100:5.1f}%)"
            )
        print(" | ".join(parts))
        time.sleep(0.1)
except KeyboardInterrupt:
    pass
finally:
    spi.close()
    for pin in PWM_PINS:
        wiringpi.softPwmWrite(pin, 0)
        wiringpi.softPwmStop(pin)