# WISP_Payload_2026_27
WISP Payload Teams 2026/2027 Codebase

## Building and running

The flight code runs on a Teensy 4.1 and is built with
[PlatformIO](https://platformio.org/). Install its command-line tool into a
Python virtual environment in the repo root, then activate it (once per
terminal) so `pio` is on your PATH:

```
python3 -m venv .venv
.venv/bin/pip install platformio
source .venv/bin/activate
```

On Linux, also install the Teensy udev rules so the board can be flashed
without root: https://www.pjrc.com/teensy/00-teensy.rules

Quick version, from the repo root:

```
make build    # compile
make upload   # compile and flash the Teensy over USB
make monitor  # open the serial monitor (Ctrl+C to stop)
make clean    # delete build output
```

Or call PlatformIO directly:

```
pio run               # compile
pio run -t upload     # compile and flash
pio device monitor    # serial monitor
```

The first build downloads the Teensy toolchain, so it takes a few minutes.
