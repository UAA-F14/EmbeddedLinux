import time
import gpiod

from gpiod.line import Direction, Value

LED = 7  # GPIO line offset, not necessarily physical pin 7

try:
    with gpiod.request_lines(
        "/dev/gpiochip0",
        consumer="Python LED example",
        config={
            LED: gpiod.LineSettings(
                direction=Direction.OUTPUT,
                output_value=Value.INACTIVE,
            )
        },
    ) as gpio:
        print("LED is blinking. Press Ctrl+C to exit.")

        while True:
            gpio.set_value(LED, Value.ACTIVE)
            time.sleep(1)

            gpio.set_value(LED, Value.INACTIVE)
            time.sleep(1)

except KeyboardInterrupt:
    print("\nExiting program.")