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

	data->time.system = input.firmware_time;
	data->time.sensor = input.sensor_time;
	data->time.audio = input.audio_time;
	data->time.checksum = input.checksum;

	data->buttons = *((libresense_buttons*) (uint32_t*) &input.buttons);
	data->buttons.dpad_up = CHECK_DPAD(U, UR, UL);
	data->buttons.dpad_down = CHECK_DPAD(D, DR, DL);
	data->buttons.dpad_left = CHECK_DPAD(L, UL, DL);
	data->buttons.dpad_right = CHECK_DPAD(R, UR, DR);

	data->triggers[0].level = input.triggers[0] / (float) UINT8_MAX;
	data->triggers[0].adaptive_point = input.adaptive_triggers[0];
	data->triggers[1].level = input.triggers[1] / (float) UINT8_MAX;
	data->triggers[1].adaptive_point = input.adaptive_triggers[1];

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

	data->sensors.accelerometer.x = SNORM(input.accelerometer.x, INT16_MAX);
	data->sensors.accelerometer.y = SNORM(input.accelerometer.y, INT16_MAX);
	data->sensors.accelerometer.z = SNORM(input.accelerometer.z, INT16_MAX);
	data->sensors.gyro.x = SNORM(input.gyro.x, INT16_MAX);
	data->sensors.gyro.y = SNORM(input.gyro.y, INT16_MAX);
	data->sensors.gyro.z = SNORM(input.gyro.z, INT16_MAX);
	// todo: calibration, sixaxis.

	if(input.battery.state <= 0x2) {
		data->battery.level = input.battery.level < 9 ? input.battery.level * 10 + 5 : 100;
		data->battery.state = input.battery.state + 1;
		data->internal.battery_error = 0;
	} else {
		data->battery.level = 0;
		data->battery.state = 0;
		data->internal.battery_error = input.battery.state;
	}

	data->state = input.state;

	data->internal.sensor_reserved = input.sensor_reserved;
	data->internal.adaptive_trigger_reserved = input.adaptive_trigger_reserved;
	data->internal.adaptive_trigger_state = input.adaptive_trigger_state;
	data->internal.audio_reserved = input.audio_reserved;
	data->internal.state_reserved = input.state_reserved;
	data->internal.audio.speaker_volume = input.speaker_volume;
	data->internal.audio.mic_volume = input.mic_volume;
	data->internal.audio.jack_volume = input.jack_volume;
}
