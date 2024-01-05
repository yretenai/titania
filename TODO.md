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

### libresensectl

- [x] log report (`libresensectl report`)
- [x] test suite (`libresensectl test`)
- [x] report dumping (`libresensectl dump`)
- [x] speed benchmarking (`libresensectl bench`)
- [x] update led state (`libresense led #rrggbb|off [player-led]`)
- [ ] dualsense edge merged profile conversion (`libresensectl profile convert`)
- [ ] dualsense edge profile importing (`libresensectl profile import [square, cross, triangle]`)
- [ ] dualsense edge profile exporting (`libresensectl profile export [triangle, square, cross, triangle]`)
- [ ] playstation access merged profile conversion (`libresensectl profile convert`)
- [ ] playstation access profile importing (`libresensectl profile import [1, 2, 3]`)
- [ ] playstation access profile exporting (`libresensectl profile export [0, 1, 2, 3]`)
- [x] pair bluetooth (`libresensectl pair`)
- [ ] shell autocomplete profiles

## planned version milestones

- 1.0.0: feature complete reading for DualSense and DualSense Edge
- 1.0.1: profile utilities
- 1.0.2: bluetooth pairing
- 2.0.0: playstation access support

### libresensectl

- 1.1.0: base functionality
- 1.5.0: profile importing/exporting
- 2.0.0: playstation access profile importing/exporting

### under consideration

- DualShock 3 Support - Massively different from the DualShock 4, DualSense, and Access, but is interesting enough to be considered (pressure sensitive buttons! gyro is real!)
- DualShock 4 Support - A lot of structrues are iterated for the DualSense, with some entirely identical (BT Pairing, Version info, Calibration.)
