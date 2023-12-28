# libresense

cross-platform userland driver for the sony playstation dualsense controller

## caveats

libresense is NOT THREAD SAFE, though the library does not use any thread locals.
there are no protections against race conditions.

this is done as a consideration for speed, if you use libresense across thread bounaries,
it is ultimately up to the library user to implement mutex guards around the calls.

it's good practice to only let one thread (i.e. an "input" thread) call libresense functions.

### notice

this project is not authorized, affiliated or endorsed by sony interactive entertainment.

## build requirements

- hidapi 0.14 or newer (meson wrap subproject, will compile if system is missing)
- a C2x capable compiler (GCC 13 or newer, Clang 16 or newer, Visual Studio 2022 or newer)
- meson 1.2.0 or newer
- ninja 1.10.0 or newer

## building

### for visual studio

```shell
mkdir build
cd build
meson setup .. -Dbuildtype=release --backend ninja --genvslite vs2022
meson compile
```

### for everything else

```shell
mkdir build
cd build
meson setup .. -Dbuildtype=release
meson compile
```
### notice

while the library is built on c2x, the `libresense.h` header is c17 (maybe c11) compatible.

## usage

TODO, see src/cli.c for the source of `libresense-log` (which is a "test function")

## acknowledgements

- [this archived](https://gist.github.com/stealth-alex/10a8e7cc6027b78fa18a7f48a0d3d1e4) [reddit post](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) for the initial research on the output report
- [pydualsense](https://github.com/flok/pydualsense) for guidance on bluetooth
- [controllers wiki](https://controllers.fandom.com/wiki/Sony_DualSense) for report ids and some of the missing report values
- [eleccelerator wiki](https://eleccelerator.com/wiki/index.php?title=DualShock_4) for guidance on crc32
- [dsremap's research](https://dsremap.readthedocs.io/en/latest/reverse.html) for some of the DS4 reports that were reused for the DSense
- [linux kernel playstation driver](https://github.com/torvalds/linux/blob/master/drivers/hid/hid-playstation.c) for guidance on how to read calibration data

## license

this project is licensed under MPL-2.0 (Mozilla Public License Version 2.0), you should have received a copy of this license.
if not, an up to date copy is always available [here](https://www.mozilla.org/en-US/MPL/2.0/) and the repository copy is [here](https://nothg.chronovore.dev/library/libresense/tree/LICENSE).
