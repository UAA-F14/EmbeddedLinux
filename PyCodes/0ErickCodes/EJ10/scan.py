import gpiod
from gpiod.line import Direction

ok, bad = [], []
for n in range(352):
    try:
        with gpiod.request_lines(
            "/dev/gpiochip0",
            consumer="scan",
            config={n: gpiod.LineSettings(direction=Direction.INPUT)},
        ):
            ok.append(n)
    except OSError as e:
        bad.append((n, e.errno))

print("OK:", ok)
print("Failed:", bad)
