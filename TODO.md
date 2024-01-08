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
- [ ] playstation access profile report reading
- [ ] playstation access profile report writing
- [ ] documentation

### titaniactl

- [x] log report (`titaniactl report`)
- [x] test suite (`titaniactl test`)
- [x] report dumping (`titaniactl dump`)
- [x] speed benchmarking (`titaniactl bench`)
- [x] update led state (`titania led #rrggbb|off [player-led]`)
- [ ] dualsense edge merged profile conversion (`titaniactl profile convert`)
- [x] dualsense edge profile importing (`titaniactl profile import [square, cross, triangle]`)
- [x] dualsense edge profile exporting (`titaniactl profile export [triangle, square, cross, triangle]`)
- [ ] playstation access merged profile conversion (`titaniactl profile convert`)
- [ ] playstation access profile importing (`titaniactl profile import [1, 2, 3]`)
- [ ] playstation access profile exporting (`titaniactl profile export [0, 1, 2, 3]`)
- [x] pair bluetooth (`titaniactl pair`)
- [ ] shell autocomplete profiles

## planned version milestones

- 1.0.0: feature complete reading for DualSense and DualSense Edge
- 1.0.1: profile utilities
- 1.0.2: bluetooth pairing
- 2.0.0: playstation access support

### titaniactl

- 1.1.0: base functionality
- 1.5.0: profile importing/exporting
- 2.0.0: playstation access profile importing/exporting

### under consideration

- DualShock 3 Support - Massively different from the DualShock 4, DualSense, and Access, but is interesting enough to be considered (pressure sensitive buttons! gyro is real!)
- DualShock 4 Support - A lot of structrues are iterated for the DualSense, with some entirely identical (BT Pairing, Version info, Calibration.)
