import time 
import gpiod
from gpiod.line import Direction, Value, Bias

Boton = 22

with gpiod.request_lines(
"/dev/gpiochip0",
    consumer="lectura-boton",
    config={
        Boton: gpiod.LineSettings(
            direction=Direction.INPUT,
            bias=Bias.PULL_UP
        )
    },
)as gpio:
    print("Presiona el boton. Ctrl+C para salir.")

    try:
        while True:
            estado = gpio.get_value(Boton)
            if estado == Value.INACTIVE:
                print("Boton presionado")
            else:
                print("Boton no presionado")
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("Saliendo del programa.")