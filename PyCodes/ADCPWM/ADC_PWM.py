import time
from smbus2 import SMBus, i2c_msg
from gpiozero import PWMLED

ADC_ADDRESS = 0x48
ADC_MAX_VALUE = 0x0FFF
ADC_REFERENCE_VOLTAGE = 3.3
PWM_PIN = 18

with SMBus(0) as bus, PWMLED(PWM_PIN) as pwm:
    try:
        while True:
            msg = i2c_msg.read(ADC_ADDRESS, 2)
            bus.i2c_rdwr(msg)
            data = list(msg)
            adc_value = int.from_bytes(bytes(data), "little") & ADC_MAX_VALUE

            pwm.value = adc_value / ADC_MAX_VALUE
            voltage = adc_value / ADC_MAX_VALUE * ADC_REFERENCE_VOLTAGE
            pwm_percentage = pwm.value * 100
            print(
                f"ADC: {adc_value:4d} | Voltaje: {voltage:.2f} V "
                f"| PWM: {pwm_percentage:.1f}%"
            )
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
