#!/usr/bin/env python

import sys
import time

IN1 = 32
IN2 = 33
OUT1 = 34
OUT2 = 35
OUT3 = 37

# shift
DATA = OUT1
CLOCK = OUT3
LATCH = OUT2

shift_str = "SHIFT %d %d %d" % (DATA, CLOCK, LATCH)


def s(shift_this_1337_value, sleep_delay=0):
    print shift_str, shift_this_1337_value
    sys.stdout.flush()
    time.sleep(sleep_delay)

while True:
    d = 0.03
    l = 0.3
    for i in range(5):
        s(255, l)
        s(170, l)
        s(85, l)
        s(0, l)

    for i in range(10):
        s((1 << (i % 6)), d)

    for i in range(50):
        for i in range(6):
            s((1 << i), d)
        for i in range(6):
            s((1 << 5 - i), d)
