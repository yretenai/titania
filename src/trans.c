//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"

#define PI_CONST 3.14159265358979323846f
#define G_CONST 9.80665f

#define CHECK_DPAD(V, A, B, C) V.dpad == DUALSENSE_DPAD_##A || V.dpad == DUALSENSE_DPAD_##B || V.dpad == DUALSENSE_DPAD_##C

#define CALIBRATE(value, slot) (value < 0 ? value / calibration[slot].min : value / calibration[slot].max)

#define CALIBRATE_BIAS(value, slot) CALIBRATE(value - calibration[slot].bias, slot)

void libresense_convert_input(const libresense_hid hid_info, const dualsense_input_msg input, libresense_data* data, libresense_calibration_bit calibration[6]) {
	*data = (libresense_data) { 0 };
	data->hid = hid_info;

	data->time.sequence = input.sequence;
	data->time.touch_sequence = input.touch_sequence;
	data->time.system = input.firmware_time;
	data->time.sensor = input.sensors.time;
	data->time.checksum = input.checksum;
	data->time.driver_sequence = input.state_id;
	data->time.battery = input.state.battery_time;

	data->buttons.dpad_up = CHECK_DPAD(input.buttons, U, UR, UL);
	data->buttons.dpad_down = CHECK_DPAD(input.buttons, D, DR, DL);
	data->buttons.dpad_left = CHECK_DPAD(input.buttons, L, UL, DL);
	data->buttons.dpad_right = CHECK_DPAD(input.buttons, R, UR, DR);
	data->buttons.square = input.buttons.square;
	data->buttons.cross = input.buttons.cross;
	data->buttons.circle = input.buttons.circle;
	data->buttons.triangle = input.buttons.triangle;
	data->buttons.l1 = input.buttons.l1;
	data->buttons.r1 = input.buttons.r1;
	data->buttons.l2 = input.buttons.l2;
	data->buttons.r2 = input.buttons.r2;
	data->buttons.share = input.buttons.share;
	data->buttons.option = input.buttons.option;
	data->buttons.l3 = input.buttons.l3;
	data->buttons.r3 = input.buttons.r3;
	data->buttons.playstation = input.buttons.playstation;
	data->buttons.touch = input.buttons.touch;
	data->buttons.mute = input.buttons.mute;
	data->buttons.edge_f1 = input.buttons.edge_f1;
	data->buttons.edge_f2 = input.buttons.edge_f2;
	data->buttons.edge_left_paddle = input.buttons.edge_left_paddle;
	data->buttons.edge_right_paddle = input.buttons.edge_right_paddle;
	data->buttons.reserved = input.buttons.reserved;
	data->buttons.edge_reserved = input.buttons.edge_reserved;

	data->triggers[LIBRESENSE_LEFT].level = DENORM_CLAMP_UINT8(input.triggers[DUALSENSE_LEFT]);
	data->triggers[LIBRESENSE_LEFT].id = input.adaptive_triggers[ADAPTIVE_TRIGGER_LEFT].id;
	data->triggers[LIBRESENSE_LEFT].section = input.adaptive_triggers[ADAPTIVE_TRIGGER_LEFT].level;
	data->triggers[LIBRESENSE_LEFT].effect = input.state.trigger.left;
	data->triggers[LIBRESENSE_RIGHT].level = DENORM_CLAMP_UINT8(input.triggers[DUALSENSE_RIGHT]);
	data->triggers[LIBRESENSE_RIGHT].id = input.adaptive_triggers[ADAPTIVE_TRIGGER_RIGHT].id;
	data->triggers[LIBRESENSE_RIGHT].section = input.adaptive_triggers[ADAPTIVE_TRIGGER_RIGHT].level;
	data->triggers[LIBRESENSE_RIGHT].effect = input.state.trigger.right;

	data->sticks[LIBRESENSE_LEFT].x = DENORM_CLAMP_INT8(input.sticks[DUALSENSE_LEFT].x);
	data->sticks[LIBRESENSE_LEFT].y = DENORM_CLAMP_INT8(input.sticks[DUALSENSE_LEFT].y);
	data->sticks[LIBRESENSE_RIGHT].x = DENORM_CLAMP_INT8(input.sticks[DUALSENSE_RIGHT].x);
	data->sticks[LIBRESENSE_RIGHT].y = DENORM_CLAMP_INT8(input.sticks[DUALSENSE_RIGHT].y);

	data->touch[LIBRESENSE_PRIMARY].id = input.touch[DUALSENSE_LEFT].id.value;
	data->touch[LIBRESENSE_PRIMARY].active = !input.touch[DUALSENSE_LEFT].id.idle;
	data->touch[LIBRESENSE_PRIMARY].pos.x = input.touch[DUALSENSE_LEFT].pos.x;
	data->touch[LIBRESENSE_PRIMARY].pos.y = input.touch[DUALSENSE_LEFT].pos.y;
	data->touch[LIBRESENSE_SECONDARY].id = input.touch[DUALSENSE_RIGHT].id.value;
	data->touch[LIBRESENSE_SECONDARY].active = !input.touch[DUALSENSE_RIGHT].id.idle;
	data->touch[LIBRESENSE_SECONDARY].pos.x = input.touch[DUALSENSE_RIGHT].pos.x;
	data->touch[LIBRESENSE_SECONDARY].pos.y = input.touch[DUALSENSE_RIGHT].pos.y;
	data->buttons.touchpad = data->touch[LIBRESENSE_PRIMARY].active || data->touch[LIBRESENSE_SECONDARY].active;

	data->sensors.accelerometer.x = CALIBRATE(input.sensors.accelerometer.x, CALIBRATION_ACCELEROMETER_X) / DUALSENSE_ACCELEROMETER_RESOLUTION / G_CONST;
	data->sensors.accelerometer.y = CALIBRATE(input.sensors.accelerometer.y, CALIBRATION_ACCELEROMETER_Y) / DUALSENSE_ACCELEROMETER_RESOLUTION / G_CONST;
	data->sensors.accelerometer.z = CALIBRATE(input.sensors.accelerometer.z, CALIBRATION_ACCELEROMETER_Z) / DUALSENSE_ACCELEROMETER_RESOLUTION / G_CONST;
	data->sensors.gyro.x = CALIBRATE_BIAS(input.sensors.gyro.x, CALIBRATION_GYRO_X) / DUALSENSE_GYRO_RESOLUTION * PI_CONST;
	data->sensors.gyro.y = CALIBRATE_BIAS(input.sensors.gyro.y, CALIBRATION_GYRO_Y) / DUALSENSE_GYRO_RESOLUTION * PI_CONST;
	data->sensors.gyro.z = CALIBRATE_BIAS(input.sensors.gyro.z, CALIBRATION_GYRO_Z) / DUALSENSE_GYRO_RESOLUTION * PI_CONST;
	data->sensors.temperature = input.sensors.temperature;

	data->device.headphones = input.state.device.headphones;
	data->device.headset = input.state.device.headset;
	data->device.muted = input.state.device.muted;
	data->device.usb_data = input.state.device.usb_data;
	data->device.usb_power = input.state.device.usb_power;
	data->device.external_mic = input.state.device.external_mic;
	data->device.haptic_filter = input.state.device.haptic_filter;
	data->device.reserved = (uint16_t) input.state.device.reserved1 | (uint16_t) input.state.device.reserved2 << 3;

	data->state_id = input.state_id;

	data->battery.level = input.state.battery.level * 0.1 + 0.10;
	data->battery.state = input.state.battery.state + 1;

	data->bt.has_hid = input.bt.has_hid;
	data->bt.unknown = input.bt.unknown;
	data->bt.unknown2 = input.bt.unknown2;
	data->bt.unknown3 = input.bt.unknown3;
	data->bt.seq = input.bt.seq;

	if (IS_EDGE(hid_info)) {
		data->time.battery = 0;
		data->edge_device.raw_buttons.dpad_up = CHECK_DPAD(input.state.edge.override, U, UR, UL);
		data->edge_device.raw_buttons.dpad_down = CHECK_DPAD(input.state.edge.override, D, DR, DL);
		data->edge_device.raw_buttons.dpad_left = CHECK_DPAD(input.state.edge.override, L, UL, DL);
		data->edge_device.raw_buttons.dpad_right = CHECK_DPAD(input.state.edge.override, R, UR, DR);
		data->edge_device.raw_buttons.square = input.state.edge.override.square;
		data->edge_device.raw_buttons.cross = input.state.edge.override.cross;
		data->edge_device.raw_buttons.circle = input.state.edge.override.circle;
		data->edge_device.raw_buttons.triangle = input.state.edge.override.triangle;
		data->edge_device.raw_buttons.playstation = input.state.edge.override.playstation;
		data->edge_device.raw_buttons.share = input.state.edge.override.share;
		data->edge_device.raw_buttons.option = input.state.edge.override.option;
		data->edge_device.stick.disconnected = input.state.edge.input.stick_disconnected;
		data->edge_device.stick.errored = input.state.edge.input.stick_error;
		data->edge_device.stick.calibrating = input.state.edge.input.stick_calibrating;
		data->edge_device.stick.unknown = input.state.edge.input.stick_unknown;
		data->edge_device.trigger_levels[LIBRESENSE_LEFT] = input.state.edge.input.left_trigger_level;
		data->edge_device.trigger_levels[LIBRESENSE_RIGHT] = input.state.edge.input.right_trigger_level;
		data->edge_device.current_profile_id = input.state.edge.profile.id;
		data->edge_device.profile_indicator.switching_disabled = input.state.edge.profile.disable_switching;
		data->edge_device.profile_indicator.led = input.state.edge.profile.led_indicator;
		data->edge_device.profile_indicator.vibration = input.state.edge.profile.vibrate_indicator;
		data->edge_device.brightness = input.state.edge.override.brightness_override;
		data->edge_device.emulating_rumble = input.state.edge.override.emulating_rumble;
		data->edge_device.profile_indicator.unknown1 = input.state.edge.profile.unknown1;
		data->edge_device.profile_indicator.unknown2 = input.state.edge.profile.unknown2;
		data->edge_device.unknown = (uint8_t) input.state.edge.override.unknown3 | (uint8_t) input.state.edge.override.unknown4 << 1;
	}
}
