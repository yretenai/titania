# changelog

## v1.0.0

- Initial release

## v1.0.1

- Added stick templates

## v1.0.2

- BT Fixes
- Stubbed Gyro calculation

## v1.1.0

- Better BT disconnect/connect support
- Various macOS and win32 bugfixes

## v1.5.0

- Dualsense Edge profile importing and exporting
- Fixed various out of bounds errors
- Renamed from Libresense to Titania
- Rumble emulation fixes
- Windows fixes
- Partial Playstation Access support

## v1.5.1

- Fixed profile ID generation

## v1.5.2 

- Replaced profile ID generation with a hash of the profile

## v2.0.0

- Playstation Access profile importing and exporting

## v2.1.0

- Fix Adaptive Trigger mode 0x27

## v2.2.0

- Fix Calibration again

## v2.2.1

- Versioning fixes
- Do not use -Werror on release builds

## v2.2.2

- Fix profiles not being imported under certain circumstances

## v2.2.3

- Optimize sensor conversion

## v2.2.4

- titaniactl will output json by default if stdout is not a tty

## v2.2.5

- Fixed windows not respecting bitfields

## v2.2.6

- Added better checks for consistency

## v2.2.7

- Internal housekeeping

## v3.0.0 (Pre-Release)

- USB Haptics
- BT Haptics
- benchmarking now reports miliseconds
- fix titania_pull being slow
- added more serial information
- fixed dualsense edge profile switching being disabled by default
- fixed calibration (for the last time hopefully)
- 