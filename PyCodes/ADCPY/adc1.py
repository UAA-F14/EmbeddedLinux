import time
from smbus2 import SMBus, i2c_msg

with SMBus(0) as bus:
    while True:
        msg = i2c_msg.read(0x48, 6)
        bus.i2c_rdwr(msg)
        d = list(msg)
        ch = [int.from_bytes(bytes(d[i:i+2]), "little") & 0x0FFF for i in (0, 2, 4)]
        print(ch)
