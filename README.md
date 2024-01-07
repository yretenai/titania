# titania

cross-platform userland driver for the sony playstation dualsense controller

## why the name

The PS5 memory controller is a Marvell Titania 2, that's why.
I was looking for a replacement project name, not because there's something wrong with the name "libresense" but because the shared object name would have been "liblibresense.so."
Now, it's libtitania.so, or titania.dll which is much nicer in my opinion.

## caveats

titania is NOT THREAD SAFE, though the library does not use any thread locals.
there are no protections against race conditions.

this is done as a consideration for speed, if you use titania across thread bounaries,
it is ultimately up to the library user to implement mutex guards around the calls.

it's good practice to only let one thread (i.e. an "input" thread) call titania functions.

### notice

this project is not authorized, affiliated or endorsed by sony interactive entertainment.

## build requirements

- hidapi 0.14 or newer (meson wrap subproject, will compile if system is missing)
- icu 73.2 or newer (meson wrap subproject, will compile if system is missing)
- pandoc 3.0.0 or newer (optional, only used for man page generation)
- a C23/C2x capable compiler (GCC 13 or newer, Clang 16 or newer, MSVC 19.30 or newer)
- meson 1.2.0 or newer
- ninja 1.10.0 or newer

## building

### for windows

```powershell
mkdir build
cd build
meson setup .. -buildtype=release --backend ninja --Dc_std=c17
meson compile
```

#### caveats

windows builds still use c17, as the windows compiler does not support c2x yet.
this will change when the windows compiler supports c2x.

### for everything else

```shell
mkdir build
cd build
meson setup .. -buildtype=release
meson compile
```
### notice

while the library is built on c2x, the `titania.h` header is c17 (maybe c11) compatible.

## usage

TODO, see src/ctl/ for the source of `titaniactl`

## acknowledgements

- [this archived](https://gist.github.com/stealth-alex/10a8e7cc6027b78fa18a7f48a0d3d1e4) [reddit post](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) for the initial research on the output report
- [pydualsense](https://github.com/flok/pydualsense) for guidance on bluetooth
- [controllers wiki](https://controllers.fandom.com/wiki/Sony_DualSense) for report ids and some of the missing report values
- [eleccelerator wiki](https://eleccelerator.com/wiki/index.php?title=DualShock_4) for guidance on crc32
- [dsremap's research](https://dsremap.readthedocs.io/en/latest/reverse.html) for some of the DS4 reports that were reused for the DSense
- [linux kernel playstation driver](https://github.com/torvalds/linux/blob/master/drivers/hid/hid-playstation.c) for guidance on how to read calibration data

## license

this project is licensed under MPL-2.0 (Mozilla Public License Version 2.0), you should have received a copy of this license.
if not, an up-to-date copy is always available [here](https://www.mozilla.org/en-US/MPL/2.0/) and the repository copy is [here](https://nothg.chronovore.dev/library/titania/tree/LICENSE).
