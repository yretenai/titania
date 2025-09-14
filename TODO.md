# TODO

- [x] input reading
- [x] output writing
- [x] adaptive triggers
- [x] LEDs
- [x] audio
- [x] rumble
- [x] touchpad
- [x] sensors
- [x] device flags
- [x] bluetooth
- [x] force bluetooth pairing
- [x] MMU calibration
- [x] firmware version reading
- [x] serial number reading
- [x] dualsense edge profile report reading
- [x] dualsense edge profile report writing
- [x] playstation access input reading
- [x] playstation access output reading
- [x] playstation access profile report reading
- [x] playstation access profile report writing
- [x] haptics (macOS usb)
- [ ] haptics (windows usb)
- [ ] haptics (linux usb)
- [ ] haptics (bt)
- [x] audio control
- [ ] documentation

### titaniactl

- [x] log report (`titaniactl report`)
- [x] test suite (`titaniactl test`)
- [x] report dumping (`titaniactl dump`)
- [x] speed benchmarking (`titaniactl bench`)
- [x] update led state (`titania led #rrggbb|off [player-led]`)
- [x] dualsense edge merged profile conversion (`titaniactl profile convert`)
- [x] dualsense edge profile importing (`titaniactl profile import [square, cross, circle]`)
- [x] dualsense edge profile exporting (`titaniactl profile export [triangle, square, cross, circle]`)
- [x] playstation access merged profile conversion (`titaniactl profile convert`)
- [x] playstation access profile importing (`titaniactl profile import [1, 2, 3]`)
- [x] playstation access profile exporting (`titaniactl profile export [0, 1, 2, 3]`)
- [x] pair bluetooth (`titaniactl pair`)
- [ ] shell autocomplete profiles

### under consideration

- DualShock 3 Support - Massively different from the DualShock 4, DualSense, and Access, but is interesting enough to be considered (pressure sensitive buttons! gyro is real!)
- DualShock 4 Support - A lot of structrues are iterated for the DualSense, with some entirely identical (BT Pairing, Version info, Calibration.)
