#include "structures.h"

#define PI_CONST 3.14159265358979323846f
#define G_CONST 9.80665f

#define CHECK_DPAD(A, B, C) \
	input.buttons.dpad == DUALSENSE_DPAD_ ## A || \
	input.buttons.dpad == DUALSENSE_DPAD_ ## B || \
	input.buttons.dpad == DUALSENSE_DPAD_ ## C

#define CALIBRATE(value, slot) \
	(value < 0 ? value / calibration[slot].min : value / calibration[slot].max)

#define CALIBRATE_BIAS(value, slot) \
	CALIBRATE(value - calibration[slot].bias, slot)

void
libresense_convert_input(const dualsense_input_msg input, libresense_data *data, libresense_calibration_bit calibration[6]) {
	*data = (libresense_data) { 0 };

	data->time.sequence = input.sequence;
	data->time.touch_sequence = input.touch_sequence;
	data->time.system = input.firmware_time;
	data->time.sensor = input.sensors.time.value;
	data->time.checksum = input.checksum;

	data->buttons = *(libresense_buttons *) (uint32_t *) &input.buttons;
	data->buttons.dpad_up = CHECK_DPAD(U, UR, UL);
	data->buttons.dpad_down = CHECK_DPAD(D, DR, DL);
	data->buttons.dpad_left = CHECK_DPAD(L, UL, DL);
	data->buttons.dpad_right = CHECK_DPAD(R, UR, DR);

	data->triggers[LIBRESENSE_LEFT].level = input.triggers[DUALSENSE_LEFT] / (float) UINT8_MAX;
	data->triggers[LIBRESENSE_LEFT].id = input.adaptive_triggers[ADAPTIVE_TRIGGER_LEFT].id;
	data->triggers[LIBRESENSE_LEFT].effect = input.adaptive_triggers[ADAPTIVE_TRIGGER_LEFT].level;
	data->triggers[LIBRESENSE_RIGHT].level = input.triggers[DUALSENSE_RIGHT] / (float) UINT8_MAX;
	data->triggers[LIBRESENSE_RIGHT].id = input.adaptive_triggers[ADAPTIVE_TRIGGER_RIGHT].id;
	data->triggers[LIBRESENSE_RIGHT].effect = input.adaptive_triggers[ADAPTIVE_TRIGGER_RIGHT].level;

	data->sticks[LIBRESENSE_LEFT].x = input.sticks[DUALSENSE_LEFT].x / (float) INT8_MAX / 2.0f;
	data->sticks[LIBRESENSE_LEFT].y = input.sticks[DUALSENSE_LEFT].y / (float) INT8_MAX / 2.0f;
	data->sticks[LIBRESENSE_RIGHT].x = input.sticks[DUALSENSE_RIGHT].x / (float) INT8_MAX / 2.0f;
	data->sticks[LIBRESENSE_RIGHT].y = input.sticks[DUALSENSE_RIGHT].y / (float) INT8_MAX / 2.0f;

	data->touch[LIBRESENSE_PRIMARY].id = input.touch[DUALSENSE_LEFT].id.value;
	data->touch[LIBRESENSE_PRIMARY].active = !input.touch[DUALSENSE_LEFT].id.idle;
	data->touch[LIBRESENSE_PRIMARY].coords.x = input.touch[DUALSENSE_LEFT].coord.x;
	data->touch[LIBRESENSE_PRIMARY].coords.y = input.touch[DUALSENSE_LEFT].coord.y;
	data->touch[LIBRESENSE_SECONDARY].id = input.touch[DUALSENSE_RIGHT].id.value;
	data->touch[LIBRESENSE_SECONDARY].active = !input.touch[DUALSENSE_RIGHT].id.idle;
	data->touch[LIBRESENSE_SECONDARY].coords.x = input.touch[DUALSENSE_RIGHT].coord.x;
	data->touch[LIBRESENSE_SECONDARY].coords.y = input.touch[DUALSENSE_RIGHT].coord.y;

	data->sensors.accelerometer.x = CALIBRATE(input.sensors.accelerometer.x, CALIBRATION_ACCELEROMETER_X) / DUALSENSE_ACCELEROMETER_RESOLUTION / G_CONST;
	data->sensors.accelerometer.y = CALIBRATE(input.sensors.accelerometer.y, CALIBRATION_ACCELEROMETER_Y) / DUALSENSE_ACCELEROMETER_RESOLUTION / G_CONST;
	data->sensors.accelerometer.z = CALIBRATE(input.sensors.accelerometer.z, CALIBRATION_ACCELEROMETER_Z) / DUALSENSE_ACCELEROMETER_RESOLUTION / G_CONST;
	data->sensors.gyro.x = CALIBRATE_BIAS(input.sensors.gyro.x, CALIBRATION_GYRO_X) / DUALSENSE_GYRO_RESOLUTION * PI_CONST;
	data->sensors.gyro.y = CALIBRATE_BIAS(input.sensors.gyro.y, CALIBRATION_GYRO_Y) / DUALSENSE_GYRO_RESOLUTION * PI_CONST;
	data->sensors.gyro.z = CALIBRATE_BIAS(input.sensors.gyro.z, CALIBRATION_GYRO_Z) / DUALSENSE_GYRO_RESOLUTION * PI_CONST;

	if(input.state.battery.state < 0x2) {
		data->battery.level = input.state.battery.level < 10 ? input.state.battery.level * 10 + 5 : 100;
		data->battery.state = input.state.battery.state + 1;
	} else if(input.state.battery.state == 0x2) {
		data->battery.level = 100;
		data->battery.state = LIBRESENSE_BATTERY_FULL;
	} else {
		data->battery.battery_error = input.state.battery.state;
	}

	data->device = input.state.device;
	data->edge_device = input.state.edge_device;

	data->state = *(uint64_t*)&input.state;
	data->reserved = input.reserved;
}
