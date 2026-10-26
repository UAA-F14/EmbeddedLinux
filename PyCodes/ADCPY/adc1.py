import time
from smbus2 import SMBus, i2c_msg

with SMBus(0) as bus:
    while True:
        msg = i2c_msg.read(0x48, 2)
        bus.i2c_rdwr(msg)
        data = list(msg)
        adc_value = int.from_bytes(bytes(data), "little") & 0x0FFF
        print(f"Bytes: 0x{data[0]:02X} 0x{data[1]:02X} | ADC: {adc_value}")
        time.sleep(0.1)
