
import time
import serial
import wiringpi

UART_PORT = "/dev/ttyS7"
UART_BAUDRATE = 9600

HEADER = 0xA5
N_CHANNELS = 3
FRAME_LEN = 1 + 2 * N_CHANNELS

ADC_MAX_VALUE = 0x0FFF
ADC_REFERENCE_VOLTAGE = 3.31

PWM_PINS = (29, 33, 35)
PWM_RANGE = 100


def read_adcs(uart):
    """Sincroniza y recibe una trama UART de 3 canales ADC."""

    while True:
        header = uart.read(1)

        if not header:
            return None

        if header[0] == HEADER:
            break

    raw = uart.read(FRAME_LEN - 1)

    if len(raw) != FRAME_LEN - 1:
        return None

    return [
        int.from_bytes(raw[2*i:2*i+2], "little") & ADC_MAX_VALUE
        for i in range(N_CHANNELS)
    ]


def adc2pwm(adc):
    adc = max(0, min(ADC_MAX_VALUE, adc))
    return round(adc * PWM_RANGE / ADC_MAX_VALUE)


wiringpi.wiringPiSetupPhys()

for pin in PWM_PINS:
    wiringpi.softPwmCreate(pin, 0, PWM_RANGE)

uart = serial.Serial(
    UART_PORT,
    UART_BAUDRATE,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=0.2
)

try:
    while True:
        values = read_adcs(uart)

        if values is None:
            continue

        parts = []

        for i, (pin, adc) in enumerate(zip(PWM_PINS, values)):
            step = adc2pwm(adc)
            wiringpi.softPwmWrite(pin, step)

            voltage = adc / ADC_MAX_VALUE * ADC_REFERENCE_VOLTAGE

            parts.append(
                f"CH{i}: {adc:4d} "
                f"({voltage:.2f} V -> {step:3d}%)"
            )

        print(" | ".join(parts))

except KeyboardInterrupt:
    pass

finally:
    uart.close()

    for pin in PWM_PINS:
        wiringpi.softPwmWrite(pin, 0)
        wiringpi.softPwmStop(pin)
