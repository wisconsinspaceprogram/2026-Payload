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
without root, then unplug and replug the Teensy:

```
wget -O /tmp/00-teensy.rules https://www.pjrc.com/teensy/00-teensy.rules
sudo cp /tmp/00-teensy.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Quick version, from the repo root:

```
make build    # compile
make upload   # compile and flash the Teensy over USB
make monitor  # open the serial monitor (Ctrl+C to stop)
make clean    # delete build output
```

The first build downloads the Teensy toolchain, so it takes a few minutes.

Flashing uses the command-line Teensy loader, so it works headless (e.g. on
the Pi). If an upload can't reboot the board on its own, press the white
program button on the Teensy.

## Telemetry output

Every control cycle (10 Hz) the Teensy prints the whole blackboard over USB
serial as one line, in the same format the radio sends:

```
clock_time=1546301247,uptime_ms=7130,cycle_count=39,imu.yaw_deg=0.000,...,radio.healthy=0,...
```

Values are `name=value` pairs separated by commas. Floats have 3 decimals and
bools are `0`/`1`. Channels appear in the order they were registered, and each
driver's `*.healthy` channel shows whether its hardware is responding. Over
the radio, channels that don't fit in a 255-byte LoRa packet are left off the
end; serial always sends the full line.

## Testing without hardware (dummy data)

The `dummy` build adds a fake sensor that publishes `sim.*` channels, so
telemetry and ground software can be tested with nothing wired to the Teensy:

| Channel | Description |
| --- | --- |
| `sim.altitude_m` | 60 s flight arc up to 3000 m and back down, repeating |
| `sim.vertical_speed_mps` | Rate of change of the altitude |
| `sim.temperature_c` | 20 °C at the ground, standard lapse rate |
| `sim.pressure_hpa` | Barometric pressure for the altitude |

All values have a little noise added.

```
pio run -e dummy -t upload   # flash the dummy build
make monitor                 # watch the output
make upload                  # put flight firmware back
```

`make build` and `make upload` only build flight firmware, so the fake sensor
never ends up in a flight build by accident. The dummy code lives in
`src/drivers/dummy/` and is compiled in only when `DUMMY_DATA` is defined.

With no sensors wired, `imu.healthy` and `radio.healthy` read `0`, and
`clock_time` starts in 2019 because the command-line loader doesn't set the
Teensy's real-time clock.
