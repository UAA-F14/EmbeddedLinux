import time
import wiringpi

# Cambia estos números por los wPi de 8 pines libres (ver `gpio readall`)
LEDS = [0, 1, 2, 3, 4, 5, 6, 7]
DELAY = 0.08  # segundos por paso

wiringpi.wiringPiSetup()          # numeración wPi
for p in LEDS:
    wiringpi.pinMode(p, wiringpi.OUTPUT)
    wiringpi.digitalWrite(p, 0)

def mostrar(byte):
    """Escribe un byte de 8 bits en los LEDs (bit 0 = LEDS[0])."""
    for i, p in enumerate(LEDS):
        wiringpi.digitalWrite(p, (byte >> i) & 1)

# 0 -> 7 -> 1 (sin repetir los extremos para que no se "pegue")
secuencia = list(range(8)) + list(range(6, 0, -1))

try:
    while True:
        for pos in secuencia:
            mostrar(1 << pos)
            time.sleep(DELAY)
except KeyboardInterrupt:
    mostrar(0)
