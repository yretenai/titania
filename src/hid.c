#include <stdlib.h>

#include "structures.h"

const int libresense_max_controllers = LIBRESENSE_MAX_CONTROLLERS;

static dualsense_state state[LIBRESENSE_MAX_CONTROLLERS];
bool is_initialized = false;

static libresense_device_info device_infos[] = {
	{ 0x054C, 0x0CE6 }, // DualSense
	{ 0x054C, 0x0DF2 }, // DualSense Edge
};

void
libresense_init(void) {
	if (is_initialized) {
		return;
	}

	if (hid_init() != 0) {
		return;
	}

	is_initialized = true;
}

libresense_result
libresense_get_hids(libresense_hid *hids, size_t hids_length) {
	CHECK_INIT;
	size_t index = 0;

	for (size_t i = 0; i < sizeof(device_infos) / sizeof(libresense_device_info); i++) {
		if (index >= hids_length) {
			break;
		}

		libresense_device_info info = device_infos[i];

		struct hid_device_info *root = hid_enumerate(info.vendor_id, info.product_id);
		struct hid_device_info *dev = root;
		while (dev) {
			if (wcslen(dev->serial_number) >= 0x100) {
				hid_free_enumeration(root);
				return ELIBRESENSE_NOT_IMPLEMENTED;
			}

			if (dev->bus_type == HID_API_BUS_USB || dev->bus_type == HID_API_BUS_BLUETOOTH) {
				hids[index].handle = 0;
				hids[index].product_id = dev->product_id;
				hids[index].vendor_id = dev->vendor_id;
				hids[index].is_bluetooth = dev->bus_type == HID_API_BUS_BLUETOOTH;
				wcscpy(hids[index].serial, dev->serial_number);
				// todo: request calibration, firmware, and profiles.

				index += 1;

				if (index >= hids_length) {
					break;
				}
			}

			dev = dev->next;
		}
		hid_free_enumeration(root);
	}

	for (size_t i = index; i < hids_length; i++) {
		hids[i].handle = LIBRESENSE_INVALID_HANDLE;
	}

	return ELIBRESENSE_OK;
}

size_t
libresense_get_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size, const bool preserve) {
	if (handle == NULL || buffer == NULL || size < 2) {
		return 0;
	}
	uint8_t old_byte = buffer[0];
	buffer[0] = (uint8_t) report_id;
	size_t ret = hid_get_feature_report(handle, buffer, size);
	if (preserve) {
		buffer[0] = old_byte;
	}
	return ret;
}

size_t
libresense_send_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size, const bool preserve) {
	if (handle == NULL || buffer == NULL || size < 2) {
		return 0;
	}
	uint8_t old_byte = buffer[0];
	buffer[0] = (uint8_t) report_id;
	size_t ret = hid_send_feature_report(handle, buffer, size);
	if (preserve) {
		buffer[0] = old_byte;
	}
	return ret;
}

size_t
libresense_debug_get_feature_report(const libresense_handle handle, int report_id, uint8_t *buffer, size_t size) {
	CHECK_INIT;
	CHECK_HANDLE_VALID(handle);

	return libresense_get_feature_report(state[handle].hid, report_id, buffer, size, false);
}

#define CALIBRATION_GYRO(slot, type) \
	(calibration.gyro[slot].type * calibration.gyro_speed.type / (float) DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX)

#define CALIBRATION_ACCELEROMETER(slot, type) \
	(calibration.accelerometer[slot].type / 8192.0f * (float) DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX)

#define CALIBRATION_ACCELEROMETER_BIAS(slot) \
	calibration.accelerometer[slot].max - (calibration.accelerometer[slot].max - calibration.accelerometer[slot].min) / 2

static bool use_calibration = true;

libresense_result
libresense_open(libresense_hid *handle) {
	CHECK_INIT;
	
	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		if (state[i].hid == NULL) {
			handle->handle = i;
			state[i].hid = hid_open(handle->vendor_id, handle->product_id, handle->serial);

			dualsense_calibration_info calibration;
			size_t report_sz = libresense_get_feature_report(state[i].hid, DUALSENSE_REPORT_CALIBRATION, (uint8_t*)&calibration, 41, false);
			if(use_calibration && report_sz > 1 && report_sz <= 41) {
				state[i].calibration[CALIBRATION_GYRO_X].max = CALIBRATION_GYRO(CALIBRATION_RAW_X, max);
				state[i].calibration[CALIBRATION_GYRO_Y].max = CALIBRATION_GYRO(CALIBRATION_RAW_Y, max);
				state[i].calibration[CALIBRATION_GYRO_Z].max = CALIBRATION_GYRO(CALIBRATION_RAW_Z, max);
				
				state[i].calibration[CALIBRATION_GYRO_X].min = CALIBRATION_GYRO(CALIBRATION_RAW_X, min);
				state[i].calibration[CALIBRATION_GYRO_Y].min = CALIBRATION_GYRO(CALIBRATION_RAW_Y, min);
				state[i].calibration[CALIBRATION_GYRO_Z].min = CALIBRATION_GYRO(CALIBRATION_RAW_Z, min);

				state[i].calibration[CALIBRATION_GYRO_X].bias = calibration.gyro_bias.x;
				state[i].calibration[CALIBRATION_GYRO_Y].bias = calibration.gyro_bias.y;
				state[i].calibration[CALIBRATION_GYRO_Z].bias = calibration.gyro_bias.z;

				state[i].calibration[CALIBRATION_GYRO_X].speed = calibration.gyro_speed.min;
				state[i].calibration[CALIBRATION_GYRO_Y].speed = calibration.gyro_speed.min;
				state[i].calibration[CALIBRATION_GYRO_Z].speed = calibration.gyro_speed.min;

				state[i].calibration[CALIBRATION_ACCELEROMETER_X].max = CALIBRATION_ACCELEROMETER(CALIBRATION_RAW_X, max);
				state[i].calibration[CALIBRATION_ACCELEROMETER_Y].max = CALIBRATION_ACCELEROMETER(CALIBRATION_RAW_Y, max);
				state[i].calibration[CALIBRATION_ACCELEROMETER_Z].max = CALIBRATION_ACCELEROMETER(CALIBRATION_RAW_Z, max);
				
				state[i].calibration[CALIBRATION_ACCELEROMETER_X].min = CALIBRATION_ACCELEROMETER(CALIBRATION_RAW_X, min);
				state[i].calibration[CALIBRATION_ACCELEROMETER_Y].min = CALIBRATION_ACCELEROMETER(CALIBRATION_RAW_Y, min);
				state[i].calibration[CALIBRATION_ACCELEROMETER_Z].min = CALIBRATION_ACCELEROMETER(CALIBRATION_RAW_Z, min);

				state[i].calibration[CALIBRATION_ACCELEROMETER_X].bias = CALIBRATION_ACCELEROMETER_BIAS(CALIBRATION_RAW_X);
				state[i].calibration[CALIBRATION_ACCELEROMETER_Y].bias = CALIBRATION_ACCELEROMETER_BIAS(CALIBRATION_RAW_Y);
				state[i].calibration[CALIBRATION_ACCELEROMETER_Z].bias = CALIBRATION_ACCELEROMETER_BIAS(CALIBRATION_RAW_Z);
			} else {
				state[i].calibration[CALIBRATION_GYRO_X] = (libresense_calibration_bit) { DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX, DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX, 0, 540 };
				state[i].calibration[CALIBRATION_GYRO_Y] = (libresense_calibration_bit) { DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX, DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX, 0, 540 };
				state[i].calibration[CALIBRATION_GYRO_Z] = (libresense_calibration_bit) { DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX, DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX, 0, 540 };
				state[i].calibration[CALIBRATION_ACCELEROMETER_X] = (libresense_calibration_bit) { DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX, DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX, 0, 1 };
				state[i].calibration[CALIBRATION_ACCELEROMETER_Y] = (libresense_calibration_bit) { DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX, DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX, 0, 1 };
				state[i].calibration[CALIBRATION_ACCELEROMETER_Z] = (libresense_calibration_bit) { DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX, DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX, 0, 1 };
			}

#ifdef LIBRESENSE_DEBUG
			uint8_t report[HID_API_MAX_REPORT_DESCRIPTOR_SIZE];
			int report_size = hid_get_report_descriptor(state[i].hid, report, HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

			memset(handle->report_ids, 0, sizeof(libresense_hid_report_id) * 0xFF);

			int report_id = 0;
			if (report_size > 7 && report_size < HID_API_MAX_REPORT_DESCRIPTOR_SIZE && report[0] == 0x05 && report[1] == 0x01 && // USAGE PAGE Generic Desktop
				report[2] == 0x09 && report[3] == 0x05 &&																		 // USAGE Game Pad
				report[4] == 0xA1 && report[5] == 0x01) {																		 // COLLECTION Application
				for (int j = 6; j < report_size;) {
					uint8_t op = report[j++];
					uint32_t value = 0;
					uint8_t size = op & 0x3;
					switch (size) {
						case 0: continue;
						case 1:
							if (j > report_size) {
								j += 1;
								break;
							}
							value = report[j];
							j += 1;
							break;
						case 2:
							if (j + 2 > report_size) {
								j += 2;
								break;
							}
							value = *(uint16_t *) (report + j);
							j += 2;
							break;
						case 3:
							if (j + 4 > report_size) {
								j += 4;
								break;
							}
							value = *(uint32_t *) (report + j);
							j += 4;
							break;
					}

					uint8_t op_value = op >> 2;
					if (op_value == 48) { // END COLLECTION
						break;
					}

					if (op_value == 33) { // REPORT ID
						if (report_id == 0xFF) {
							break;
						}
						if (value == 0xf7) {
							//
						}
						handle->report_ids[report_id++].id = (uint8_t) value;
						handle->report_ids[report_id - 1].size = 1;
					}

					if (op_value == 37 && report_id > 0) {				 // REPORT COUNT
						handle->report_ids[report_id - 1].size += value; // report_id_size * value;
					}
				}
			}
#endif
			state[i].hid_info = *handle;
			return ELIBRESENSE_OK;
		}
	}

	return ELIBRESENSE_NO_SLOTS;
}

libresense_result
libresense_poll(libresense_handle *handle, const size_t handle_count, libresense_data *data) {
	CHECK_INIT;
	if (handle == NULL || data == NULL) {
		return ELIBRESENSE_INVALID_DATA;
	}

	if (handle_count <= 0) {
		return ELIBRESENSE_OK;
	}

	libresense_data invalid = { 0 };
	invalid.hid.handle = LIBRESENSE_INVALID_HANDLE;

	for (size_t i = 0; i < handle_count; i++) {
		CHECK_HANDLE_VALID(handle[i]);
		dualsense_state *hid_state = &state[handle[i]];
		uint8_t *buffer = hid_state->input.buffer;
		size_t size = sizeof(dualsense_input_msg_ex);
		if (!hid_state->hid_info.is_bluetooth) {
			buffer = hid_state->input.data.msg.buffer;
			size = sizeof(dualsense_input_msg);
		}
		memset(buffer + 1, (char) 0, size - 1);

		int count = hid_read_timeout(hid_state->hid, buffer, size, 16);

		if (count < 0) {
			handle[i] = LIBRESENSE_INVALID_HANDLE;
			data[i] = invalid;

			libresense_close(handle[i]);
			continue; // invalid!
		}

		if (count != (int) size) {
			continue; // truncated?
		}

		libresense_convert_input(hid_state->input.data.msg.data, &data[i], hid_state->calibration);
		data[i].hid = hid_state->hid_info;
		data[i].time.driver_sequence = ++hid_state->in_sequence;
	}

	return ELIBRESENSE_OK;
}

libresense_result
libresense_close(const libresense_handle handle) {
	CHECK_INIT;
	CHECK_HANDLE(handle);

	hid_close(state[handle].hid);
	state[handle] = (dualsense_state) { 0 };

	return ELIBRESENSE_OK;
}

void
libresense_exit(void) {
	if (!is_initialized) {
		return;
	}

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		libresense_close(i);
	}

	is_initialized = false;
}
