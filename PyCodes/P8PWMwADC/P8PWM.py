import time

import wiringpi
from smbus2 import SMBus, i2c_msg

ADC_ADDRESS = 0x48
ADC_MAX_VALUE = 0x0FFF
ADC_REFERENCE_VOLTAGE = 3.3
N_CHANNELS = 3

PWM_PINS = (29, 33, 35)    
PWM_RANGE = 100            


def read_pwm(bus):
    msg = i2c_msg.read(ADC_ADDRESS, 2 * N_CHANNELS)
    bus.i2c_rdwr(msg)
    raw = bytes(msg)
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

try:
    with SMBus(0) as bus:
        while True:
            values = read_pwm(bus)
            parts = []
            for i, (pin, adc) in enumerate(zip(PWM_PINS, values)):
                step = adc2pwm(adc)
                wiringpi.softPwmWrite(pin, step)
                voltage = adc / ADC_MAX_VALUE * ADC_REFERENCE_VOLTAGE
                parts.append(
                    f"CH{i}: {adc:4d} ({voltage:.2f} V -> {step / PWM_RANGE * 100:5.1f}%)"
                )
            time.sleep(0.1)
except KeyboardInterrupt:
    pass
finally:
    for pin in PWM_PINS:
        wiringpi.softPwmWrite(pin, 0)
        wiringpi.softPwmStop(pin)