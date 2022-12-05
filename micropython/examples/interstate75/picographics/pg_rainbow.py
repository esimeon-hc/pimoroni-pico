import hub75
import time
import math
from picographics import PicoGraphics, DISPLAY_INTERSTATE75 as DISPLAY
WIDTH, HEIGHT = 32, 32

hub = hub75.Hub75(WIDTH, HEIGHT, panel_type=hub75.PANEL_GENERIC)
graphics = PicoGraphics(DISPLAY)
hub.start()

width = WIDTH
height = HEIGHT


@micropython.native  # noqa: F821
def from_hsv(h, s, v):
    i = math.floor(h * 6.0)
    f = h * 6.0 - i
    v *= 255.0
    p = v * (1.0 - s)
    q = v * (1.0 - f * s)
    t = v * (1.0 - (1.0 - f) * s)

    i = int(i) % 6
    if i == 0:
        return int(v), int(t), int(p)
    if i == 1:
        return int(q), int(v), int(p)
    if i == 2:
        return int(p), int(v), int(t)
    if i == 3:
        return int(p), int(q), int(v)
    if i == 4:
        return int(t), int(p), int(v)
    if i == 5:
        return int(v), int(p), int(q)


@micropython.native  # noqa: F821
def draw():
    global hue_offset, phase
    phase_percent = phase / 15
    for x in range(width):
        colour = hue_map[int((x + (hue_offset * width)) % width)]
        for y in range(height):
            v = ((math.sin((x + y) / stripe_width + phase_percent) + 1.5) / 2.5)

            graphics.set_pen(graphics.create_pen(int(colour[0] * v), int(colour[1] * v), int(colour[2] * v)))
            graphics.pixel(x, y)

    hub.update(graphics)


hue_map = [from_hsv(x / width, 1.0, 0.5) for x in range(width)]
hue_offset = 0.0

animate = True
stripe_width = 3.0
speed = 5.0


phase = 0
while True:

    if animate:
        phase += speed

    start = time.ticks_ms()
    time.sleep(0.2)
    draw()

    print("total took: {} ms".format(time.ticks_ms() - start))