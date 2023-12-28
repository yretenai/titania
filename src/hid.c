//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <stdio.h>

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
HANDLE _lock;
#define LIBRESENSE_THREAD_INIT() _lock = CreateMutex(NULL, FALSE, NULL); if(_lock == NULL) return LIBRESENSE_INVALID
#define LIBRESENSE_THREAD_LOCK() WaitForSingleObject(_lock, INFINITE)
#define LIBRESENSE_THREAD_UNLOCK() ReleaseMutex(_lock, INFINITE)
#define LIBRESENSE_THREAD_DEINIT() CloseHandle(_lock)
#else
#include <pthread.h>
pthread_mutex_t _lock;
#define LIBRESENSE_THREAD_INIT()
#define LIBRESENSE_THREAD_LOCK() pthread_mutex_lock(&_lock)
#define LIBRESENSE_THREAD_UNLOCK() pthread_mutex_unlock(&_lock)
#define LIBRESENSE_THREAD_DEINIT()
#endif

#include "structures.h"

const int32_t libresense_max_controllers = LIBRESENSE_MAX_CONTROLLERS;

static dualsense_state state[LIBRESENSE_MAX_CONTROLLERS];
static bool is_initialized = false;

static libresense_device_info device_infos[] = {
	{ 0x054C, 0x0CE6 }, // DualSense
	{ 0x054C, 0x0DF2 }, // DualSense Edge
};

libresense_result
libresense_init_checked(const int size) {
	if (size != sizeof(libresense_hid)) {
		return LIBRESENSE_INVALID_LIBRARY;
	}

	if (is_initialized) {
		return LIBRESENSE_OK;
	}

	if (hid_init() != 0) {
		return LIBRESENSE_HIDAPI_FAIL;
	}

	memset(&state, 0, sizeof(state));

	LIBRESENSE_THREAD_INIT();

	libresense_init_checksum();

	is_initialized = true;
	return LIBRESENSE_OK;
}

libresense_result
libresense_get_hids(libresense_hid *hids, const size_t hids_length) {
	CHECK_INIT();
	size_t index = 0;

	for (size_t i = 0; i < sizeof(device_infos) / sizeof(libresense_device_info); i++) {
		if (index >= hids_length) {
			break;
		}

		const libresense_device_info info = device_infos[i];

		struct hid_device_info *root = hid_enumerate(info.vendor_id, info.product_id);
		const struct hid_device_info *dev = root;
		while (dev) {
			if (wcslen(dev->serial_number) >= 0x100) {
				hid_free_enumeration(root);
				return LIBRESENSE_NOT_IMPLEMENTED;
			}

			if (dev->bus_type == HID_API_BUS_USB || dev->bus_type == HID_API_BUS_BLUETOOTH) {
				hids[index].handle = 0;
				hids[index].product_id = dev->product_id;
				hids[index].vendor_id = dev->vendor_id;
				hids[index].is_bluetooth = dev->bus_type == HID_API_BUS_BLUETOOTH;
				hids[index].is_edge = IS_EDGE(hids[index]);
				wcscpy(hids[index].hid_serial, dev->serial_number);

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
		hids[i].handle = LIBRESENSE_INVALID_HANDLE_ID;
	}

	return LIBRESENSE_OK;
}

size_t
libresense_get_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size) {
	if (handle == NULL || buffer == NULL || size < 2) {
		return 0;
	}
	buffer[0] = (uint8_t) report_id;
	const size_t ret = hid_get_feature_report(handle, buffer, size);
	return ret;
}

size_t
libresense_send_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size, const bool preserve) {
	if (handle == NULL || buffer == NULL || size < 2) {
		return 0;
	}
	const uint8_t old_byte = buffer[0];
	buffer[0] = (uint8_t) report_id;
	const size_t ret = hid_send_feature_report(handle, buffer, size);
	if (preserve) {
		buffer[0] = old_byte;
	}
	return ret;
}

size_t
libresense_debug_get_feature_report(const libresense_handle handle, const int report_id, uint8_t *buffer, const size_t size) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	return libresense_get_feature_report(state[handle].hid, report_id, buffer, size);
}

#define CALIBRATION_GYRO(slot, type) \
	(calibration.gyro[slot].type * calibration.gyro_speed.type / (float) DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX)

#define CALIBRATION_ACCELEROMETER(slot, type) \
	(calibration.accelerometer[slot].type / 8192.0f * (float) DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX)

#define CALIBRATION_ACCELEROMETER_BIAS(slot) \
	calibration.accelerometer[slot].max - (calibration.accelerometer[slot].max - calibration.accelerometer[slot].min) / 2

#define MAC6_TO_UINT64(v) \
	(((uint64_t) v[5] << 40) | ((uint64_t) v[4] << 32) | ((uint64_t) v[3] << 24) | ((uint64_t) v[2] << 16) | ((uint64_t) v[1] << 8) | (uint64_t) v[0])

libresense_result
libresense_open(libresense_hid *handle) {
	CHECK_INIT();

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		if (state[i].hid == NULL) {
			memset(&state[i], 0, sizeof(dualsense_state));
			handle->handle = i;
			state[i].hid = hid_open(handle->vendor_id, handle->product_id, handle->hid_serial);
			state[i].hid_info = *handle;
			state[i].output.data.id = DUALSENSE_REPORT_BLUETOOTH;
			state[i].output.data.msg.data.id = DUALSENSE_REPORT_OUTPUT;

			if(state[i].hid_info.is_bluetooth) { // on linux this also releases control from the kernel driver
				state[i].output.data.msg.data.flags.reset_led = true;
				libresense_push(&handle->handle, 1);
			}

			dualsense_firmware_info firmware;
			const size_t firmware_report_sz = libresense_get_feature_report(state[i].hid, DUALSENSE_REPORT_FIRMWARE, (uint8_t *) &firmware, sizeof(dualsense_firmware_info));
			if(firmware_report_sz == 64) {
				memset(handle->firmware.datetime, 0, sizeof(handle->firmware.datetime));
				memcpy(handle->firmware.datetime, firmware.date, sizeof(firmware.date));
				handle->firmware.datetime[sizeof(firmware.date)] = ' ';
				memcpy(handle->firmware.datetime + sizeof(firmware.date) + 1, firmware.time, sizeof(firmware.time));
				handle->firmware.datetime[sizeof(handle->firmware.datetime) - 1] = 0;

				for(int j = 0; j < LIBRESENSE_VERSION_MAX; ++j) {
					handle->firmware.versions[j] = (libresense_firmware_version) { firmware.versions[j].major, firmware.versions[j].minor };
				}
			} else {
				handle->firmware.datetime[0] = 0;
			}

			dualsense_serial_info serial;
			const size_t serial_report_sz = libresense_get_feature_report(state[i].hid, DUALSENSE_REPORT_SERIAL, (uint8_t *) &serial, sizeof(dualsense_serial_info));
			if(serial_report_sz == 20) {
				sprintf(handle->serial.mac, "%02x:%02x:%02x:%02x:%02x:%02x", serial.device_mac[5], serial.device_mac[4], serial.device_mac[3], serial.device_mac[2], serial.device_mac[1], serial.device_mac[0]);
				sprintf(handle->serial.paired_mac, "%02x:%02x:%02x:%02x:%02x:%02x", serial.pair_mac[5], serial.pair_mac[4], serial.pair_mac[3], serial.pair_mac[2], serial.pair_mac[1], serial.pair_mac[0]);
				handle->serial.mac[sizeof(handle->serial.mac) - 1] = 0;
				handle->serial.paired_mac[sizeof(handle->serial.paired_mac) - 1] = 0;
				handle->serial.unknown = (uint64_t) serial.unknown[0] << 16 | (uint64_t) serial.unknown[1] << 8 | (uint64_t) serial.unknown[2];
			} else {
				handle->serial.mac[0] = 0;
				handle->serial.paired_mac[0] = 0;
			}


			dualsense_calibration_info calibration;
			const size_t calibration_report_sz = libresense_get_feature_report(state[i].hid, DUALSENSE_REPORT_CALIBRATION, (uint8_t *) &calibration, sizeof(dualsense_calibration_info));
			if(calibration_report_sz == 41) {
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

			if (IS_EDGE(state[i].hid_info)) {
				const uint8_t profile_reports[LIBRESENSE_PROFILE_MAX] = {
					DUALSENSE_EDGE_REPORT_PROFILE_99_P1,
					DUALSENSE_EDGE_REPORT_PROFILE_TRIANGLE_P1,
					DUALSENSE_EDGE_REPORT_PROFILE_SQUARE_P1,
					DUALSENSE_EDGE_REPORT_PROFILE_CROSS_P1,
					DUALSENSE_EDGE_REPORT_PROFILE_CIRCLE_P1
				};

				for (int32_t j = 0; j < LIBRESENSE_PROFILE_MAX; ++j) {
					dualsense_profile_data profile_data[3];
					bool exit = false;
					for(int32_t k = 0; k < 3; ++k) {
						const size_t sz = libresense_get_feature_report(state[i].hid, profile_reports[j] + k, profile_data[i], sizeof(dualsense_profile_data));
						if(sz != sizeof(dualsense_profile_data)) {
							exit = true;
							break;
						}
					}
					if (exit) {
						break;
					}
					libresense_convert_edge_profile_input(profile_data, &handle->edge_profiles[j]);
				}
			}

#ifdef LIBRESENSE_DEBUG
			uint8_t report[HID_API_MAX_REPORT_DESCRIPTOR_SIZE];
			const int report_size = hid_get_report_descriptor(state[i].hid, report, HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

			memset(handle->report_ids, 0, sizeof(handle->report_ids));

			if (report_size > 7 && report_size < HID_API_MAX_REPORT_DESCRIPTOR_SIZE && report[0] == 0x05 && report[1] == 0x01 && // USAGE PAGE Generic Desktop
				report[2] == 0x09 && report[3] == 0x05 &&																		 // USAGE Game Pad
				report[4] == 0xA1 && report[5] == 0x01) {
				int report_id = 0;
				// COLLECTION Application
				for (int j = 6; j < report_size;) {
					const uint8_t op = report[j++];
					uint32_t value = 0;
					const uint8_t size = op & 0x3;
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
						default: continue;
					}

					const uint8_t op_value = op >> 2;
					if (op_value == 48) { // END COLLECTION
						break;
					}

					if (op_value == 33) { // REPORT ID
						if (report_id == UINT8_MAX) {
							break;
						}

						handle->report_ids[report_id++].id = (uint8_t) value;
						handle->report_ids[report_id - 1].size = 1;
					} else if(report_id > 0) {
						if (op_value == 37) { // REPORT COUNT
							handle->report_ids[report_id - 1].size = value;
						} else if (op_value == 32) { // INPUT
							handle->report_ids[report_id - 1].type = 0;
						} else if (op_value == 36) { // OUTPUT
							handle->report_ids[report_id - 1].type = 1;
						} else if (op_value == 44) { // FEATURE
							handle->report_ids[report_id - 1].type = 2;
						}
					}
				}
			}
#endif
			state[i].hid_info = *handle;

			// this is at the end so it's reasonably late<
			{
				libresense_led_update update;
				update.color.x = 1.0;
				update.color.y = 0.0;
				update.color.z = 1.0;
				update.brightness = LIBRESENSE_LEVEL_HIGH;
				update.effect = LIBRESENSE_LED_EFFECT_OFF; // RESET;
				update.led = LIBRESENSE_LED_PLAYER_1;
				libresense_update_led(handle->handle, update);
				libresense_push(&handle->handle, 1);
			}
			return LIBRESENSE_OK;
		}
	}

	return LIBRESENSE_NO_SLOTS;
}

libresense_result
libresense_pull(libresense_handle *handle, const size_t handle_count, libresense_data *data) {
	CHECK_INIT();
	if (handle == NULL || data == NULL) {
		return LIBRESENSE_INVALID_DATA;
	}

	if (handle_count <= 0) {
		return LIBRESENSE_OK;
	}

	libresense_data invalid = { 0 };
	invalid.hid.handle = LIBRESENSE_INVALID_HANDLE_ID;

	for (size_t i = 0; i < handle_count; i++) {
		CHECK_HANDLE_VALID(handle[i]);
		dualsense_state *hid_state = &state[handle[i]];
		uint8_t *buffer = hid_state->input.buffer;
		size_t size = sizeof(dualsense_input_msg_ex);
		if (!hid_state->hid_info.is_bluetooth) {
			buffer = hid_state->input.data.msg.buffer;
			size = sizeof(dualsense_input_msg);
		}
		// NOTE: should we clear the buffer? speed jitters a lot more if we clear memory
		// the controller polls at between 1KHz (BT) and 250Hz (USB), so we ideally want to be < 1ms
		memset(buffer + 1, 0, size - 1);
		hid_state->input.data.id = DUALSENSE_REPORT_BLUETOOTH;
		hid_state->input.data.msg.data.id = DUALSENSE_REPORT_INPUT;

		const int count = hid_read(hid_state->hid, buffer, size);

		if (count != (int) size) {
			handle[i] = LIBRESENSE_INVALID_HANDLE_ID;
			data[i] = invalid;

			libresense_close(handle[i]);
			continue; // invalid!
		}

		libresense_convert_input(hid_state->hid_info, hid_state->input.data.msg.data, &data[i], hid_state->calibration);
	}

	return LIBRESENSE_OK;
}

libresense_result
libresense_push(const libresense_handle *handle, const size_t handle_count) {
	CHECK_INIT();
	if (handle == NULL) {
		return LIBRESENSE_INVALID_DATA;
	}

	if (handle_count <= 0) {
		return LIBRESENSE_OK;
	}

	for (size_t i = 0; i < handle_count; i++) {
		CHECK_HANDLE_VALID(handle[i]);
		dualsense_state *hid_state = &state[handle[i]];
		hid_state->output.data.msg.data.state_id = hid_state->seq;

		const uint8_t *buffer = hid_state->output.buffer;
		size_t size = sizeof(dualsense_output_msg_ex);
		if (!hid_state->hid_info.is_bluetooth) {
			buffer = hid_state->output.data.msg.buffer;
			size = sizeof(dualsense_output_msg);
			if(!hid_state->hid_info.is_edge) {
				size -= 0x10;
			}
		} else {
			hid_state->output.data.bt_checksum = libresense_calc_checksum(crc_seed_output, buffer, size - 4);
		}

		hid_write(hid_state->hid, buffer, size);

		hid_state->output.data.id = DUALSENSE_REPORT_BLUETOOTH;
		hid_state->output.data.msg.data.id = DUALSENSE_REPORT_OUTPUT;
		hid_state->output.data.msg.data.flags.value = 0;
	}

	return LIBRESENSE_OK;
}

libresense_result
libresense_update_led(const libresense_handle handle, const libresense_led_update data) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	LIBRESENSE_THREAD_LOCK();

	dualsense_output_msg *hid_state = &state[handle].output.data.msg.data;

	if(data.color.x >= 0.0f && data.color.y >= 0.0f && data.color.z >= 0.0f) {
		hid_state->flags.control2 = true;
		hid_state->control2.led_color_control = true;
		hid_state->flags.led = true;
		hid_state->led.color.x = NORM_CLAMP_UINT8(data.color.x);
		hid_state->led.color.y = NORM_CLAMP_UINT8(data.color.y);
		hid_state->led.color.z = NORM_CLAMP_UINT8(data.color.z);
	}

	if(data.brightness != hid_state->led.brightness) {
		hid_state->flags.led = true;
		hid_state->flags.control2 = true;
		hid_state->control2.led_brightness_control = true;
		hid_state->led.brightness = data.brightness;
	}

	if(data.effect != hid_state->led.effect) {
		hid_state->flags.led = true;
		hid_state->led.effect = data.effect;
	}

	if(data.led != hid_state->led.led_id) {
		hid_state->flags.player_indicator_led = true;
		hid_state->led.led_id = data.led;
	}

	LIBRESENSE_THREAD_UNLOCK();

	return LIBRESENSE_OK;
}

libresense_result
libresense_update_audio(const libresense_handle handle, const libresense_audio_update data) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	LIBRESENSE_THREAD_LOCK();

	dualsense_output_msg *hid_state = &state[handle].output.data.msg.data;

	hid_state->flags.audio_output = true;
	hid_state->audio.flags.force_external_mic = (data.mic_selection & LIBRESENSE_MIC_EXTERNAL) == LIBRESENSE_MIC_EXTERNAL;
	hid_state->audio.flags.force_internal_mic = (data.mic_selection & LIBRESENSE_MIC_INTERNAL) == LIBRESENSE_MIC_INTERNAL;
	hid_state->audio.flags.balance_external_mic = (data.mic_balance & LIBRESENSE_MIC_EXTERNAL) == LIBRESENSE_MIC_EXTERNAL;
	hid_state->audio.flags.balance_internal_mic = (data.mic_balance & LIBRESENSE_MIC_INTERNAL) == LIBRESENSE_MIC_INTERNAL;
	hid_state->audio.flags.disable_jack = data.disable_audio_jack;
	hid_state->audio.flags.enable_speaker = data.force_enable_speaker;

	hid_state->flags.mic_led = true;
	hid_state->audio.mic_led_flags = (dualsense_audio_mic_flags) data.mic_led;

	hid_state->flags.jack = hid_state->flags.speaker = hid_state->flags.mic = true;
	hid_state->audio.jack = NORM_CLAMP_UINT8(data.jack_volume);
	hid_state->audio.speaker = NORM_CLAMP_UINT8(data.speaker_volume);
	hid_state->audio.mic = NORM_CLAMP_UINT8(data.microphone_volume);

	LIBRESENSE_THREAD_UNLOCK();

	return LIBRESENSE_OK;
}

libresense_result
libresense_update_control(const libresense_handle handle, const libresense_control_update data) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	LIBRESENSE_THREAD_LOCK();

	dualsense_output_msg *hid_state = &state[handle].output.data.msg.data;

	hid_state->flags.control1 = hid_state->flags.control2 = true;

	hid_state->led.brightness = 0;
	hid_state->control1.touch_powersave = data.touch_powersave;
	hid_state->control1.sensor_powersave = data.sensor_powersave;
	hid_state->control1.rumble_powersave = data.rumble_powersave;
	hid_state->control1.speaker_powersave = data.speaker_powersave;
	hid_state->control1.mute_jack = data.mute_jack;
	hid_state->control1.mute_speaker = data.mute_speaker;
	hid_state->control1.mute_mic = data.mute_mic;
	hid_state->control1.disable_rumble = data.disable_rumble;

	hid_state->control2.enable_beamforming = data.enable_beamforming;
	hid_state->control2.enable_lowpass_filter = data.enable_lowpass_filter;
	hid_state->control2.gain = data.gain;
	hid_state->control2.led_brightness_control = data.led_brightness_control;
	hid_state->control2.led_color_control = data.led_color_control;
	hid_state->control2.reserved1 = data.reserved1;
	hid_state->control2.reserved2 = data.reserved2;
	hid_state->control2.reserved3 = data.reserved3;
	if(IS_EDGE(state[handle].hid_info)) {
		hid_state->control2.edge_unknown1 = data.edge_unknown1;
		hid_state->control2.edge_unknown2 = data.edge_unknown2;
		hid_state->control2.update_edge_profile = data.update_edge_profile;
		hid_state->control2.edge_disable_switching_profiles = data.edge_disable_switching_profiles;

		hid_state->edge.flags.indicator = true;
		hid_state->edge.indicator.enable_led = data.edge_enable_led_indicators;
		hid_state->edge.indicator.enable_vibration = data.edge_enable_vibration_indicators;
	}

	LIBRESENSE_THREAD_UNLOCK();

	return LIBRESENSE_OK;
}


libresense_result
compute_effect(dualsense_effect_output *effect, const libresense_effect_update trigger) {
	// clear
	effect->mode = 0;
	effect->params.multiple.id = 0;
	effect->params.multiple.value = 0;

	switch(trigger.mode) {
		case LIBRESENSE_EFFECT_NONE: break;
		case LIBRESENSE_EFFECT_OFF:
			effect->mode = DUALSENSE_EFFECT_MODE_OFF;
			break;
		case LIBRESENSE_EFFECT_STOP_VIBRATING:
			effect->mode = DUALSENSE_EFFECT_MODE_STOP;
			break;
		case LIBRESENSE_EFFECT_UNIFORM:
			effect->mode = DUALSENSE_EFFECT_MODE_UNIFORM;
			effect->params.value[0] = NORM_CLAMP(trigger.effect.uniform.position, DUALSENSE_TRIGGER_MAX);
			effect->params.value[1] = NORM_CLAMP_UINT8(trigger.effect.uniform.resistance);
			break;
		case LIBRESENSE_EFFECT_SECTION:
			effect->mode = DUALSENSE_EFFECT_MODE_SECTION;
			effect->params.value[0] = NORM_CLAMP(trigger.effect.section.position.x, DUALSENSE_TRIGGER_MAX);
			effect->params.value[1] = NORM_CLAMP(trigger.effect.section.position.y, DUALSENSE_TRIGGER_MAX);
			effect->params.value[2] = NORM_CLAMP_UINT8(trigger.effect.section.resistance);
			break;
		case LIBRESENSE_EFFECT_VIBRATE:
			effect->mode = DUALSENSE_EFFECT_MODE_VIBRATE;
			effect->params.value[0] = trigger.effect.vibrate.frequency & UINT8_MAX;
			effect->params.value[1] = NORM_CLAMP(trigger.effect.vibrate.amplitude, DUALSENSE_TRIGGER_AMPLITUDE_MAX);
			effect->params.value[2] = NORM_CLAMP(trigger.effect.vibrate.position, DUALSENSE_TRIGGER_VIBRATION_MAX);
			break;
		// ReSharper disable CppRedundantParentheses
		case LIBRESENSE_EFFECT_SLOPE: {
			effect->mode = DUALSENSE_EFFECT_MODE_SLOPE;
			const uint8_t slope_start = NORM_CLAMP(trigger.effect.slope.position.x, LIBRESENSE_TRIGGER_GRANULARITY);
			const uint8_t slope_end = NORM_CLAMP(trigger.effect.slope.position.y, LIBRESENSE_TRIGGER_GRANULARITY);
			const uint8_t res_start = NORM_CLAMP(trigger.effect.slope.resistance.x, DUALSENSE_TRIGGER_STEP);
			const uint8_t res_end = NORM_CLAMP(trigger.effect.slope.resistance.y, DUALSENSE_TRIGGER_STEP);
			effect->params.multiple.id = (1 << (slope_start - 1)) | (1 << (slope_end - 1));
			effect->params.value[2] = res_start | res_end << DUALSENSE_TRIGGER_SHIFT;
			break;
		}
		case LIBRESENSE_EFFECT_TRIGGER: {
			effect->mode = DUALSENSE_EFFECT_MODE_TRIGGER;
			const uint8_t slope_start = NORM_CLAMP(trigger.effect.trigger.position.x, LIBRESENSE_TRIGGER_GRANULARITY);
			const uint8_t slope_end = NORM_CLAMP(trigger.effect.trigger.position.y, LIBRESENSE_TRIGGER_GRANULARITY);
			effect->params.multiple.id = (1 << (slope_start - 1)) | (1 << (slope_end - 1));
			effect->params.value[2] = NORM_CLAMP(trigger.effect.trigger.resistance, DUALSENSE_TRIGGER_STEP);
			break;
		}
		case LIBRESENSE_EFFECT_VIBRATE_SLOPE: {
			effect->mode = DUALSENSE_EFFECT_MODE_VIBRATE_SLOPE;
			const uint8_t slope_start = NORM_CLAMP(trigger.effect.vibrate_slope.position.x, LIBRESENSE_TRIGGER_GRANULARITY);
			const uint8_t slope_end = NORM_CLAMP(trigger.effect.vibrate_slope.position.y, LIBRESENSE_TRIGGER_GRANULARITY);
			const uint8_t res_start = NORM_CLAMP(trigger.effect.vibrate_slope.amplitude.x, DUALSENSE_TRIGGER_STEP);
			const uint8_t res_end = NORM_CLAMP(trigger.effect.vibrate_slope.amplitude.y, DUALSENSE_TRIGGER_STEP);
			effect->params.multiple.id = (1 << (slope_start - 1)) | (1 << (slope_end - 1));
			effect->params.value[2] = res_start | res_end << DUALSENSE_TRIGGER_SHIFT;
			effect->params.value[3] = trigger.effect.vibrate_slope.frequency & UINT8_MAX;
			effect->params.value[4] = trigger.effect.vibrate_slope.period & UINT8_MAX;
			break;
		}
		case LIBRESENSE_EFFECT_MUTIPLE_SECTIONS: {
			effect->mode = DUALSENSE_EFFECT_MODE_MUTIPLE_SECTIONS;
			for(int i = 0; i < LIBRESENSE_TRIGGER_GRANULARITY; ++i) {
				if(trigger.effect.multiple_sections.resistance[i] >= 0.01f) {
					effect->params.multiple.id |= 1 << i;
					effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_sections.resistance[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * i);
				}
			}
			break;
		}
		case LIBRESENSE_EFFECT_MUTIPLE_VIBRATE: {
			effect->mode = DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE;
			for(int i = 0; i < LIBRESENSE_TRIGGER_GRANULARITY; ++i) {
				if(trigger.effect.multiple_vibrate.amplitude[i] >= 0.01f) {
					effect->params.multiple.id |= 1 << i;
					effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_vibrate.amplitude[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * i);
				}
			}
			effect->params.multiple.value |= ((uint64_t)(trigger.effect.multiple_vibrate.frequency & UINT8_MAX)) << DUALSENSE_TRIGGER_FREQ_BITS;
			effect->params.multiple.value |= ((uint64_t)(trigger.effect.multiple_vibrate.period & UINT8_MAX)) << DUALSENSE_TRIGGER_PERD_BITS;
			break;
		}
		case LIBRESENSE_EFFECT_MUTIPLE_VIBRATE_SECTIONS: {
			effect->mode = DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE_SECTIONS;
			// 9 and 10 run into the 7 and 8th bytes, which are used for freq and period which are... not used? setup from previous frames?
			// freq seems to be based off the speed the trigger is going.
			for(int i = 0; i < LIBRESENSE_TRIGGER_GRANULARITY; ++i) {
				// this is likely incorrect, have yet to find a real user of this
				if(trigger.effect.multiple_vibrate_sections.resistance[i] >= 0.01f) {
					effect->params.multiple.id |= 1 << i;
					effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_vibrate_sections.resistance[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * (i * 2));
				}
				if(trigger.effect.multiple_vibrate_sections.amplitude[i] >= 0.01f) {
					effect->params.multiple.id |= 1 << i;
					effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_vibrate_sections.amplitude[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * (i * 2 + 1));
				}
			}
			break;
		}
		// ReSharper restore CppRedundantParentheses
		default:
			return LIBRESENSE_NOT_IMPLEMENTED;
	}

	return LIBRESENSE_OK;
}

// sanity check to make sure we don't (temporarily) brick the controller
libresense_result
check_if_trigger_state_bad(const libresense_handle handle, const uint8_t id) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	const dualsense_output_msg *hid_state = &state[handle].output.data.msg.data;
	if(hid_state->effects[id].mode >= 0xF0) { // these are calibration modes, will temporarily brick the controller!!
		return LIBRESENSE_INVALID_DATA;
	}

	return LIBRESENSE_OK;
}

libresense_result
libresense_update_effect(const libresense_handle handle, const libresense_effect_update left_trigger, const libresense_effect_update right_trigger) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	LIBRESENSE_THREAD_LOCK();

	dualsense_output_msg *hid_state = &state[handle].output.data.msg.data;
	hid_state->flags.left_trigger_motor = left_trigger.mode != LIBRESENSE_EFFECT_NONE;
	hid_state->flags.right_trigger_motor = right_trigger.mode != LIBRESENSE_EFFECT_NONE;

	libresense_result result = compute_effect(&hid_state->effects[ADAPTIVE_TRIGGER_LEFT], left_trigger);
	if(IS_LIBRESENSE_OKAY(result)) {
		result = check_if_trigger_state_bad(handle, ADAPTIVE_TRIGGER_LEFT);
	}

	if(IS_LIBRESENSE_BAD(result)) {
		memset(&hid_state->effects[ADAPTIVE_TRIGGER_RIGHT], 0, sizeof(hid_state->effects[ADAPTIVE_TRIGGER_RIGHT]));
		hid_state->flags.left_trigger_motor = false;
		return result;
	}

	result = compute_effect(&hid_state->effects[ADAPTIVE_TRIGGER_RIGHT], right_trigger);
	if(IS_LIBRESENSE_OKAY(result)) {
		result = check_if_trigger_state_bad(handle, ADAPTIVE_TRIGGER_RIGHT);
	}

	if(IS_LIBRESENSE_BAD(result)) {
		memset(&hid_state->effects[ADAPTIVE_TRIGGER_RIGHT], 0, sizeof(hid_state->effects[ADAPTIVE_TRIGGER_RIGHT]));
		hid_state->flags.right_trigger_motor = false;
		return result;
	}

	LIBRESENSE_THREAD_UNLOCK();

	return result;
}

libresense_result
libresense_update_rumble(const libresense_handle handle, const float large_motor, const float small_motor) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	LIBRESENSE_THREAD_LOCK();

	const libresense_hid hid = state[handle].hid_info;
	dualsense_output_msg *hid_state = &state[handle].output.data.msg.data;
	hid_state->flags.rumble = hid_state->flags.motor_power = hid_state->flags.control2 = true;
	if(hid.is_edge || hid.firmware.versions[LIBRESENSE_VERSION_FIRMWARE].major >= 0x224) {
		hid_state->control2.advanced_rumble_control = true;
		hid_state->flags.control2 = true;
	} else {
		hid_state->flags.haptics = true;
	}
	hid_state->rumble[DUALSENSE_LARGE_MOTOR] = NORM_CLAMP_UINT8(large_motor);
	hid_state->rumble[DUALSENSE_SMALL_MOTOR] = NORM_CLAMP_UINT8(small_motor);

	LIBRESENSE_THREAD_UNLOCK();

	return LIBRESENSE_NOT_IMPLEMENTED;
}

libresense_result
libresense_update_profile(const libresense_handle handle, const libresense_edge_profile_id id, const libresense_edge_profile profile) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	LIBRESENSE_THREAD_LOCK();

	LIBRESENSE_THREAD_UNLOCK();

	return LIBRESENSE_NOT_IMPLEMENTED;
}

libresense_result
libresense_delete_profile(const libresense_handle handle, const libresense_edge_profile_id id) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	LIBRESENSE_THREAD_LOCK();

	LIBRESENSE_THREAD_UNLOCK();

	return LIBRESENSE_NOT_IMPLEMENTED;
}

libresense_result
libresense_close(const libresense_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE(handle);

	hid_close(state[handle].hid);
	memset(&state[handle], 0, sizeof(dualsense_state));

	return LIBRESENSE_OK;
}

void
libresense_exit(void) {
	if (!is_initialized) {
		return;
	}

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		libresense_close(i);
	}

	LIBRESENSE_THREAD_DEINIT();

	is_initialized = false;
}
