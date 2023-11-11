#include "structures.h"

#define CHECK_DPAD(A, B, C) \
	input.buttons.dpad == DUALSENSE_DPAD_ ## A || \
	input.buttons.dpad == DUALSENSE_DPAD_ ## B || \
	input.buttons.dpad == DUALSENSE_DPAD_ ## C

#define SNORM(value, max) \
	(value < 0 ? value / ((float) (max + 1)) : value / (float) max)

void
libresense_convert_input(const dualsense_input_msg input, libresense_data *data) {
	*data = (libresense_data) { 0 };

	data->time.sequence = input.sequence;
	data->time.touch_sequence = input.touch_sequence;
	data->time.system = input.firmware_time;
	data->time.sensor = input.sensors.time.value;
	data->time.checksum = input.checksum;

	data->buttons = *((libresense_buttons*) (uint32_t*) &input.buttons);
	data->buttons.dpad_up = CHECK_DPAD(U, UR, UL);
	data->buttons.dpad_down = CHECK_DPAD(D, DR, DL);
	data->buttons.dpad_left = CHECK_DPAD(L, UL, DL);
	data->buttons.dpad_right = CHECK_DPAD(R, UR, DR);

	data->triggers[0].level = input.triggers[0] / (float) UINT8_MAX;
	data->triggers[0].id = input.adaptive_triggers[1].id;
	data->triggers[0].effect = input.adaptive_triggers[1].level;
	data->triggers[1].level = input.triggers[1] / (float) UINT8_MAX;
	data->triggers[1].id = input.adaptive_triggers[0].id;
	data->triggers[1].effect = input.adaptive_triggers[0].level;

	data->sticks[0].x = input.sticks[0].x / (float) INT8_MAX / 2.0f;
	data->sticks[0].y = input.sticks[0].y / (float) INT8_MAX / 2.0f;
	data->sticks[1].x = input.sticks[1].x / (float) INT8_MAX / 2.0f;
	data->sticks[1].y = input.sticks[1].y / (float) INT8_MAX / 2.0f;

	data->touch[0].id = input.touch[0].id.value;
	data->touch[0].active = !input.touch[0].id.idle;
	data->touch[0].coords.x = input.touch[0].coord.x;
	data->touch[0].coords.y = input.touch[0].coord.y;
	data->touch[1].id = input.touch[1].id.value;
	data->touch[1].active = !input.touch[1].id.idle;
	data->touch[1].coords.x = input.touch[1].coord.x;
	data->touch[1].coords.y = input.touch[1].coord.y;

	data->sensors.accelerometer.x = SNORM(input.sensors.accelerometer.x, INT16_MAX);
	data->sensors.accelerometer.y = SNORM(input.sensors.accelerometer.y, INT16_MAX);
	data->sensors.accelerometer.z = SNORM(input.sensors.accelerometer.z, INT16_MAX);
	data->sensors.gyro.x = SNORM(input.sensors.gyro.x, INT16_MAX);
	data->sensors.gyro.y = SNORM(input.sensors.gyro.y, INT16_MAX);
	data->sensors.gyro.z = SNORM(input.sensors.gyro.z, INT16_MAX);
	// todo: calibration, sixaxis.

	if(input.state.battery.state < 0x2) {
		data->battery.level = input.state.battery.level < 10 ? input.state.battery.level * 10 + 5 : 100;
		data->battery.state = input.state.battery.state + 1;
	} else if(input.state.battery.state == 0x2) {
		data->battery.level = 100;
		data->battery.state = ELIBRESENSE_BATTERY_FULL;
	} else {
		data->battery.battery_error = input.state.battery.state;
	}

	data->device = input.state.device;

	data->state = *(uint64_t*)&input.state;
	data->reserved = input.reserved;
}
