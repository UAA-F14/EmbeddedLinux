#!/usr/bin/env python3
import time
import gpiod
from gpiod.line import Direction, Value, Bias

CHIP = "/dev/gpiochip0"
LEDS = [96, 97, 98, 99, 100, 101, 102, 103]
BTN_FASTER = 37   # PB5, physical pin 27
BTN_SLOWER = 36   # PB4, physical pin 28

# Delay per step in seconds, from slowest to fastest
SPEEDS = [0.40, 0.20, 0.10, 0.05, 0.025]
speed_idx = 2     # start at 0.10 s

SEQUENCE = list(range(len(LEDS))) + list(range(len(LEDS) - 2, 0, -1))

led_settings = gpiod.LineSettings(
    direction=Direction.OUTPUT,
    output_value=Value.INACTIVE,
)
btn_settings = gpiod.LineSettings(
    direction=Direction.INPUT,
    bias=Bias.PULL_UP,
)

config = {pin: led_settings for pin in LEDS}
config[BTN_FASTER] = btn_settings
config[BTN_SLOWER] = btn_settings

with gpiod.request_lines(CHIP, consumer="pendulo-8bit", config=config) as gpio:

    def show(position):
        gpio.set_values({
            pin: Value.ACTIVE if i == position else Value.INACTIVE
            for i, pin in enumerate(LEDS)
        })

    def pressed(pin):
        return gpio.get_value(pin) == Value.INACTIVE   # low = pressed

    print(f"Pendulum running. Delay: {SPEEDS[speed_idx]} s. Ctrl+C to exit.")

    step = 0
    last_step_time = time.monotonic()
    prev_faster = prev_slower = False

    try:
        while True:
            # Read buttons (react only on the press, not while held)
            faster = pressed(BTN_FASTER)
            slower = pressed(BTN_SLOWER)

            if faster and not prev_faster and speed_idx < len(SPEEDS) - 1:
                speed_idx += 1
                print(f"Faster -> delay {SPEEDS[speed_idx]} s")
            if slower and not prev_slower and speed_idx > 0:
                speed_idx -= 1
                print(f"Slower -> delay {SPEEDS[speed_idx]} s")

            prev_faster, prev_slower = faster, slower

            # Advance the pendulum when enough time has passed
            now = time.monotonic()
            if now - last_step_time >= SPEEDS[speed_idx]:
                show(SEQUENCE[step])
                step = (step + 1) % len(SEQUENCE)
                last_step_time = now

            time.sleep(0.01)   # poll buttons every 10 ms (also acts as debounce)
    except KeyboardInterrupt:
        print("\nExiting.")
    finally:
        gpio.set_values({pin: Value.INACTIVE for pin in LEDS})
