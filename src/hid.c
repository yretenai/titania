//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <stdio.h>
#include <string.h>

#include "structures.h"

#include <libresense_config.h>

#ifdef __APPLE__
#include <hidapi_darwin.h>
#endif

const int32_t libresense_max_controllers = LIBRESENSE_MAX_CONTROLLERS;

dualsense_state state[LIBRESENSE_MAX_CONTROLLERS];
bool is_initialized = false;

static libresense_device_info device_infos[] = {
	{ 0x054C, 0x0CE6 }, // DualSense
	{ 0x054C, 0x0CE7 }, // DualSense Prototype (who even has this?)
	{ 0x054C, 0x0DF2 }, // DualSense Edge
	{ 0x054C, 0x0E5F }, // Access
};

#define CALIBRATION_GYRO(slot, type) (calibration.gyro[slot].type * calibration.gyro_speed.type / (float) DUALSENSE_GYRO_RESOLUTION / (float) INT16_MAX)

#define CALIBRATION_ACCELEROMETER(slot, type) (calibration.accelerometer[slot].type / 8192.0f * (float) DUALSENSE_ACCELEROMETER_RESOLUTION / (float) INT16_MAX)

#define CALIBRATION_ACCELEROMETER_BIAS(slot) calibration.accelerometer[slot].max - (calibration.accelerometer[slot].max - calibration.accelerometer[slot].min) / 2

#define COPY_VERSION_HARDWARE(name)                                       \
	handle->firmware.name.reserved = firmware.name.hardware.reserved;     \
	handle->firmware.name.variation = firmware.name.hardware.variation;   \
	handle->firmware.name.generation = firmware.name.hardware.generation; \
	handle->firmware.name.revision = firmware.name.hardware.revision

#define COPY_VERSION_UPDATE(name)                             \
	handle->firmware.name.major = firmware.name.update.major; \
	handle->firmware.name.minor = firmware.name.update.minor; \
	handle->firmware.name.revision = firmware.name.update.revision

#define COPY_VERSION_FIRMWARE(name)                             \
	handle->firmware.name.major = firmware.name.firmware.major; \
	handle->firmware.name.minor = firmware.name.firmware.minor; \
	handle->firmware.name.revision = firmware.name.firmware.revision

libresense_result libresense_init_checked(const size_t size) {
	if (size != sizeof(libresense_hid)) {
		return LIBRESENSE_INVALID_LIBRARY;
	}

	if (is_initialized) {
		return LIBRESENSE_OK;
	}

	if (hid_init() != 0) {
		return LIBRESENSE_HIDAPI_FAIL;
	}

#ifdef __APPLE__
	hid_darwin_set_open_exclusive(0);
#endif

	memset(&state, 0, sizeof(state));

	libresense_init_checksum();

	is_initialized = true;
	return LIBRESENSE_OK;
}

libresense_result libresense_get_hids(libresense_query* hids, const size_t hids_length) {
	CHECK_INIT();
	size_t index = 0;

	for (size_t i = 0; i < sizeof(device_infos) / sizeof(libresense_device_info); i++) {
		if (index >= hids_length) {
			break;
		}

		const libresense_device_info info = device_infos[i];

		struct hid_device_info* root = hid_enumerate(info.vendor_id, info.product_id);
		const struct hid_device_info* dev = root;
		while (dev) {
			if (wcslen(dev->serial_number) >= 0x100) {
				hid_free_enumeration(root);
				return LIBRESENSE_INVALID_DATA;
			}

			if (strlen(dev->path) >= 0x200) {
				hid_free_enumeration(root);
				return LIBRESENSE_INVALID_DATA;
			}

			if (dev->bus_type == HID_API_BUS_USB || dev->bus_type == HID_API_BUS_BLUETOOTH) {
				hids[index].product_id = dev->product_id;
				hids[index].vendor_id = dev->vendor_id;
				hids[index].is_bluetooth = dev->bus_type == HID_API_BUS_BLUETOOTH;
				hids[index].is_edge = IS_EDGE(hids[index]);
				hids[index].is_access = IS_ACCESS(hids[index]);
				wcscpy(hids[index].hid_serial, dev->serial_number);
				strcpy(hids[index].hid_path, dev->path);

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
		hids[i].hid_serial[0] = 0;
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_open(const libresense_hid_path path, const bool is_bluetooth, libresense_hid* handle, const bool use_calibration, const bool blocking) {
	CHECK_INIT();

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		if (state[i].hid == nullptr) {
			memset(&state[i], 0, sizeof(dualsense_state));
			handle->handle = i;
			state[i].hid = hid_open_path(path);
			if (state[i].hid == nullptr) {
				return LIBRESENSE_HIDAPI_FAIL;
			}

			hid_set_nonblocking(state[i].hid, !blocking);
			struct hid_device_info* info = hid_get_device_info(state[i].hid);
			if (info != nullptr) {
				handle->product_id = info->product_id;
				handle->vendor_id = info->vendor_id;
			} else {
				handle->product_id = 0x0CE6; // DualSense
				handle->vendor_id = 0x054C; // Sony
			}
			handle->is_bluetooth = is_bluetooth;
			state[i].hid_info = *handle;
			handle->is_edge = IS_EDGE(state[i].hid_info);
			handle->is_access = IS_ACCESS(state[i].hid_info);
			state[i].output.data.report_id = DUALSENSE_REPORT_BLUETOOTH;
			state[i].output.data.msg.data.report_id = DUALSENSE_REPORT_OUTPUT;

			if (state[i].hid_info.is_bluetooth) { // this is needed to reset LEDs from controller firmware
				if (IS_ACCESS(state[i].hid_info)) {
					state[i].output.data.msg.access.flags.reset_led = true;
				} else {
					state[i].output.data.msg.data.flags.reset_led = true;
				}

				libresense_push(&handle->handle, 1);
			}

			dualsense_firmware_info firmware;
			firmware.report_id = DUALSENSE_REPORT_FIRMWARE;
			if (HID_PASS(hid_get_feature_report(state[i].hid, (uint8_t*) &firmware, sizeof(dualsense_firmware_info)))) {
				memset(handle->firmware.datetime, 0, sizeof(handle->firmware.datetime));
				memcpy(handle->firmware.datetime, firmware.date, sizeof(firmware.date));
				handle->firmware.datetime[sizeof(firmware.date)] = ' ';
				memcpy(handle->firmware.datetime + sizeof(firmware.date) + 1, firmware.time, sizeof(firmware.time));
				handle->firmware.datetime[sizeof(handle->firmware.datetime) - 1] = 0;

				handle->firmware.type = firmware.type;
				handle->firmware.series = firmware.series;
				COPY_VERSION_HARDWARE(hardware);
				COPY_VERSION_UPDATE(update);
				COPY_VERSION_FIRMWARE(firmware);
				COPY_VERSION_FIRMWARE(firmware2);
				COPY_VERSION_FIRMWARE(firmware3);
				COPY_VERSION_FIRMWARE(device);
				COPY_VERSION_FIRMWARE(device2);
				COPY_VERSION_FIRMWARE(device3);
				COPY_VERSION_FIRMWARE(mcu_firmware);
			} else {
				handle->firmware.datetime[0] = 0;
			}

			dualsense_serial_info serial;
			serial.report_id = DUALSENSE_REPORT_SERIAL;
			if (HID_PASS(hid_get_feature_report(state[i].hid, (uint8_t*) &serial, sizeof(dualsense_serial_info)))) {
				sprintf(handle->serial.mac,
					"%02x:%02x:%02x:%02x:%02x:%02x",
					serial.device_mac[5],
					serial.device_mac[4],
					serial.device_mac[3],
					serial.device_mac[2],
					serial.device_mac[1],
					serial.device_mac[0]);
				sprintf(handle->serial.paired_mac,
					"%02x:%02x:%02x:%02x:%02x:%02x",
					serial.pair_mac[5],
					serial.pair_mac[4],
					serial.pair_mac[3],
					serial.pair_mac[2],
					serial.pair_mac[1],
					serial.pair_mac[0]);
				handle->serial.mac[sizeof(handle->serial.mac) - 1] = 0;
				handle->serial.paired_mac[sizeof(handle->serial.paired_mac) - 1] = 0;
				handle->serial.unknown = (uint64_t) serial.unknown[0] << 16 | (uint64_t) serial.unknown[1] << 8 | (uint64_t) serial.unknown[2];
			} else {
				handle->serial.mac[0] = 0;
				handle->serial.paired_mac[0] = 0;
			}

			if (!state[i].hid_info.is_access) {
				dualsense_calibration_info calibration;
				calibration.report_id = DUALSENSE_REPORT_CALIBRATION;
				if (use_calibration && HID_PASS(hid_get_feature_report(state[i].hid, (uint8_t*) &calibration, sizeof(dualsense_calibration_info)))) {
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
					state[i].calibration[CALIBRATION_GYRO_X] =
						(libresense_calibration_bit) { DENORM_CLAMP(DUALSENSE_GYRO_RESOLUTION, INT16_MAX), DENORM_CLAMP(DUALSENSE_GYRO_RESOLUTION, INT16_MAX), 0, 540 };
					state[i].calibration[CALIBRATION_GYRO_Y] =
						(libresense_calibration_bit) { DENORM_CLAMP(DUALSENSE_GYRO_RESOLUTION, INT16_MAX), DENORM_CLAMP(DUALSENSE_GYRO_RESOLUTION, INT16_MAX), 0, 540 };
					state[i].calibration[CALIBRATION_GYRO_Z] =
						(libresense_calibration_bit) { DENORM_CLAMP(DUALSENSE_GYRO_RESOLUTION, INT16_MAX), DENORM_CLAMP(DUALSENSE_GYRO_RESOLUTION, INT16_MAX), 0, 540 };
					state[i].calibration[CALIBRATION_ACCELEROMETER_X] =
						(libresense_calibration_bit) { DENORM_CLAMP(DUALSENSE_ACCELEROMETER_RESOLUTION, INT16_MAX), DENORM_CLAMP(DUALSENSE_ACCELEROMETER_RESOLUTION, INT16_MAX), 0, 1 };
					state[i].calibration[CALIBRATION_ACCELEROMETER_Y] =
						(libresense_calibration_bit) { DENORM_CLAMP(DUALSENSE_ACCELEROMETER_RESOLUTION, INT16_MAX), DENORM_CLAMP(DUALSENSE_ACCELEROMETER_RESOLUTION, INT16_MAX), 0, 1 };
					state[i].calibration[CALIBRATION_ACCELEROMETER_Z] =
						(libresense_calibration_bit) { DENORM_CLAMP(DUALSENSE_ACCELEROMETER_RESOLUTION, INT16_MAX), DENORM_CLAMP(DUALSENSE_ACCELEROMETER_RESOLUTION, INT16_MAX), 0, 1 };
				}
			}

			state[i].hid_info = *handle;

			// this is at the end so it's reasonably late<
			{
				libresense_led_update update = { 0 };
				update.color.x = 1.0;
				update.color.y = 0.0;
				update.color.z = 1.0;
				update.led = LIBRESENSE_LED_PLAYER_1;
				update.access.enable_profile_led = true;
				update.access.enable_center_led = true;
				update.access.enable_second_center_led = false;
				update.access.update_profile = false;
				libresense_update_led(handle->handle, update);
				libresense_push(&handle->handle, 1);
			}

			return LIBRESENSE_OK;
		}
	}

	return LIBRESENSE_NO_SLOTS;
}

libresense_result libresense_pull(libresense_handle* handle, const size_t handle_count, libresense_data* data) {
	CHECK_INIT();
	if (handle == nullptr || data == nullptr) {
		return LIBRESENSE_INVALID_ARGUMENT;
	}

	if (handle_count <= 0) {
		return LIBRESENSE_OK;
	}

	if (handle_count > LIBRESENSE_MAX_CONTROLLERS) {
		return LIBRESENSE_NO_SLOTS;
	}

	libresense_data invalid = { 0 };
	invalid.hid.handle = LIBRESENSE_INVALID_HANDLE_ID;

	for (size_t i = 0; i < handle_count; i++) {
		CHECK_HANDLE_VALID(handle[i]);
		dualsense_state* hid_state = &state[handle[i]];
		uint8_t* buffer = hid_state->input.buffer;
		size_t size = sizeof(dualsense_input_msg_ex);
		if (!hid_state->hid_info.is_bluetooth) {
			buffer = hid_state->input.data.msg.buffer;
			size = sizeof(dualsense_input_msg);
		}

		hid_state->input.data.report_id = DUALSENSE_REPORT_BLUETOOTH;
		hid_state->input.data.msg.data.report_id = DUALSENSE_REPORT_INPUT;
		const int report_size = hid_read(hid_state->hid, buffer, size);

		if (HID_PASS(report_size)) {
			libresense_convert_input(hid_state->hid_info, hid_state->input.data.msg.data, &data[i], hid_state->calibration);
		} else if (HID_FAIL(report_size)) {
			libresense_close(handle[i]);
			handle[i] = LIBRESENSE_INVALID_HANDLE_ID;
			data[i] = invalid;
		}
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_push(libresense_handle* handle, const size_t handle_count) {
	CHECK_INIT();
	if (handle == nullptr) {
		return LIBRESENSE_INVALID_HANDLE;
	}

	if (handle_count <= 0) {
		return LIBRESENSE_OK;
	}

	if (handle_count > LIBRESENSE_MAX_CONTROLLERS) {
		return LIBRESENSE_NO_SLOTS;
	}

	for (size_t i = 0; i < handle_count; i++) {
		CHECK_HANDLE_VALID(handle[i]);
		dualsense_state* hid_state = &state[handle[i]];
		if (!hid_state->hid_info.is_access) { // this likely exists on access as well, idk where yet.
			hid_state->output.data.msg.data.state_id = ++hid_state->seq;
		}

		const uint8_t* buffer = hid_state->output.buffer;
		size_t size = sizeof(dualsense_output_msg_ex);
		if (!hid_state->hid_info.is_bluetooth) {
			buffer = hid_state->output.data.msg.buffer;
			size = sizeof(dualsense_output_msg);
			// Regular: 48 bytes, Edge: 64 bytes, Access: 32 bytes.
			// why.
			if (hid_state->hid_info.is_access) {
				size -= 0x20;
			} else if (!hid_state->hid_info.is_edge) {
				size -= 0x10;
			}
		} else {
			hid_state->output.data.msg.data.report_id = 0;
			hid_state->output.data.msg.data.bt.enable_hid = true;
			hid_state->output.data.msg.data.bt.seq = hid_state->seq & 0xF;
			hid_state->output.data.bt_checksum = libresense_calc_checksum(crc_seed_output, buffer, size - 4);
		}

		if (HID_FAIL(hid_write(hid_state->hid, buffer, size))) {
			libresense_close(handle[i]);
			handle[i] = LIBRESENSE_INVALID_HANDLE_ID;
			continue; // invalid!
		}

		hid_state->output.data.report_id = DUALSENSE_REPORT_BLUETOOTH;
		hid_state->output.data.msg.data.report_id = DUALSENSE_REPORT_OUTPUT;
		hid_state->output.data.msg.data.flags.value = 0;
		const bool edge_enable = hid_state->output.data.msg.data.edge.flags.enable_switching;
		hid_state->output.data.msg.data.edge.flags.value = 0;
		hid_state->output.data.msg.data.edge.flags.enable_switching = edge_enable;
		hid_state->output.data.bt_checksum = 0;
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_update_led(const libresense_handle handle, const libresense_led_update data) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return libresense_update_access_led(handle, data);
	}

	dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;

	if (data.color.x >= 0.0f && data.color.y >= 0.0f && data.color.z >= 0.0f) {
		hid_state->flags.led = true;
		hid_state->led.color.x = NORM_CLAMP_UINT8(data.color.x);
		hid_state->led.color.y = NORM_CLAMP_UINT8(data.color.y);
		hid_state->led.color.z = NORM_CLAMP_UINT8(data.color.z);
	}

	if (data.led != LIBRESENSE_LED_NO_UPDATE) {
		hid_state->flags.player_indicator_led = true;
		hid_state->led.led_id = data.led;
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_update_audio(const libresense_handle handle, const libresense_audio_update data) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return LIBRESENSE_NOT_SUPPORTED;
	}

	dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;

	hid_state->flags.audio_output = true;
	hid_state->audio.flags.force_external_mic = (data.mic_selection & LIBRESENSE_MIC_EXTERNAL) == LIBRESENSE_MIC_EXTERNAL;
	hid_state->audio.flags.force_internal_mic = (data.mic_selection & LIBRESENSE_MIC_INTERNAL) == LIBRESENSE_MIC_INTERNAL;
	hid_state->audio.flags.balance_external_mic = (data.mic_balance & LIBRESENSE_MIC_EXTERNAL) == LIBRESENSE_MIC_EXTERNAL;
	hid_state->audio.flags.balance_internal_mic = (data.mic_balance & LIBRESENSE_MIC_INTERNAL) == LIBRESENSE_MIC_INTERNAL;
	hid_state->audio.flags.disable_jack = data.disable_audio_jack;
	hid_state->audio.flags.enable_speaker = data.force_enable_speaker;

	hid_state->flags.mic_led = true;
	hid_state->audio.mic_led_flags = data.mic_led;

	hid_state->flags.jack = hid_state->flags.speaker = hid_state->flags.mic = true;
	hid_state->audio.jack = NORM_CLAMP_UINT8(data.jack_volume);
	hid_state->audio.speaker = NORM_CLAMP_UINT8(data.speaker_volume);
	hid_state->audio.mic = NORM_CLAMP_UINT8(data.microphone_volume);

	return LIBRESENSE_OK;
}

libresense_result libresense_update_control(const libresense_handle handle, const libresense_control_update data) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return LIBRESENSE_NOT_SUPPORTED;
	}

	dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;

	hid_state->flags.control1 = hid_state->flags.control2 = true;

	hid_state->control1.touch_powersave = data.touch_powersave;
	hid_state->control1.sensor_powersave = data.sensor_powersave;
	hid_state->control1.rumble_powersave = data.rumble_powersave;
	hid_state->control1.speaker_powersave = data.speaker_powersave;
	hid_state->control1.mute_jack = data.mute_jack;
	hid_state->control1.mute_speaker = data.mute_speaker;
	hid_state->control1.mute_mic = data.mute_mic;
	hid_state->control1.disable_rumble = data.disable_rumble;

	hid_state->control2.enable_beamforming = !data.disable_beamforming;
	hid_state->control2.enable_lowpass_filter = data.enable_lowpass_filter;
	hid_state->control2.gain = data.gain;
	hid_state->control2.advanced_rumble_control = !data.disable_rumble_emulation;

	hid_state->control2.led_effect_control = !data.disable_led_effect_control;
	hid_state->control2.led_brightness_control = !data.disable_led_brightness_control;
	hid_state->led.brightness = data.led_brightness;
	hid_state->led.effect = data.led_effect;

	hid_state->control2.reserved1 = data.reserved1;
#ifdef _WIN32
	hid_state->control2.reserved3a = data.reserved3 & 0x7F;
	hid_state->control2.reserved3b = data.reserved3 >> 7;
#else
	hid_state->control2.reserved3 = data.reserved3;
#endif

	if (IS_EDGE(state[handle].hid_info)) {
		hid_state->control2.has_edge_flag = true;
		hid_state->control2.edge_extension = true;
		hid_state->control2.edge_disable_switching = data.edge_disable_switching_profiles;
		hid_state->edge.flags.enable_switching = !data.edge_disable_switching_profiles;

		hid_state->edge.flags.indicator = true;
		hid_state->edge.indicator.enable_led = !data.edge_disable_led_indicators;
		hid_state->edge.indicator.enable_vibration = !data.edge_disable_vibration_indicators;
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_get_control(const libresense_handle handle, libresense_control_update* control) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return LIBRESENSE_NOT_SUPPORTED;
	}

	const dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;

	control->touch_powersave = hid_state->control1.touch_powersave;
	control->sensor_powersave = hid_state->control1.sensor_powersave;
	control->rumble_powersave = hid_state->control1.rumble_powersave;
	control->speaker_powersave = hid_state->control1.speaker_powersave;
	control->mute_jack = hid_state->control1.mute_jack;
	control->mute_speaker = hid_state->control1.mute_speaker;
	control->mute_mic = hid_state->control1.mute_mic;
	control->disable_rumble = hid_state->control1.disable_rumble;

	control->disable_beamforming = !hid_state->control2.enable_beamforming;
	control->enable_lowpass_filter = hid_state->control2.enable_lowpass_filter;
	control->gain = hid_state->control2.gain;
	control->disable_rumble_emulation = !hid_state->control2.advanced_rumble_control;

	control->disable_led_effect_control = !hid_state->control2.led_effect_control;
	control->disable_led_brightness_control = !hid_state->control2.led_brightness_control;
	control->led_brightness = hid_state->led.brightness;
	control->led_effect = hid_state->led.effect;

	control->reserved1 = hid_state->control2.reserved1;
#ifdef _WIN32
	control->reserved3 = hid_state->control2.reserved3a | (hid_state->control2.reserved3b << 7);
#else
	control->reserved3 = hid_state->control2.reserved3;
#endif

	if (IS_EDGE(state[handle].hid_info)) {
		control->edge_disable_switching_profiles = !hid_state->edge.flags.enable_switching;
		control->edge_disable_led_indicators = !hid_state->edge.indicator.enable_led;
		control->edge_disable_vibration_indicators = !hid_state->edge.indicator.enable_vibration;
	}

	return LIBRESENSE_OK;
}

libresense_result compute_effect(dualsense_effect_output* effect, dualsense_output_msg* msg, const libresense_effect_update trigger, const float power_reduction) {
	// clear
	effect->mode = 0;
	effect->params.multiple.id = 0;
	effect->params.multiple.value = 0;

	switch (trigger.mode) {
		case LIBRESENSE_EFFECT_NONE: break;
		case LIBRESENSE_EFFECT_OFF: effect->mode = DUALSENSE_EFFECT_MODE_OFF; break;
		case LIBRESENSE_EFFECT_STOP_VIBRATING: effect->mode = DUALSENSE_EFFECT_MODE_STOP; break;
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
			msg->flags.motor_power = true;
			msg->motor_flags.trigger_power_reduction = NORM_CLAMP(power_reduction, 7);
			break;
		// ReSharper disable CppRedundantParentheses
		case LIBRESENSE_EFFECT_SLOPE:
			{
				effect->mode = DUALSENSE_EFFECT_MODE_SLOPE;
				const uint8_t slope_start = NORM_CLAMP(trigger.effect.slope.position.x, LIBRESENSE_TRIGGER_GRANULARITY);
				const uint8_t slope_end = NORM_CLAMP(trigger.effect.slope.position.y, LIBRESENSE_TRIGGER_GRANULARITY);
				const uint8_t res_start = NORM_CLAMP(trigger.effect.slope.resistance.x, DUALSENSE_TRIGGER_STEP);
				const uint8_t res_end = NORM_CLAMP(trigger.effect.slope.resistance.y, DUALSENSE_TRIGGER_STEP);
				effect->params.multiple.id = (1 << (slope_start - 1)) | (1 << (slope_end - 1));
				effect->params.value[2] = res_start | res_end << DUALSENSE_TRIGGER_SHIFT;
				break;
			}
		case LIBRESENSE_EFFECT_TRIGGER:
			{
				effect->mode = DUALSENSE_EFFECT_MODE_TRIGGER;
				const uint8_t slope_start = NORM_CLAMP(trigger.effect.trigger.position.x, LIBRESENSE_TRIGGER_GRANULARITY);
				const uint8_t slope_end = NORM_CLAMP(trigger.effect.trigger.position.y, LIBRESENSE_TRIGGER_GRANULARITY);
				effect->params.multiple.id = (1 << (slope_start - 1)) | (1 << (slope_end - 1));
				effect->params.value[2] = NORM_CLAMP(trigger.effect.trigger.resistance, DUALSENSE_TRIGGER_STEP);
				break;
			}
		case LIBRESENSE_EFFECT_VIBRATE_SLOPE:
			{
				effect->mode = DUALSENSE_EFFECT_MODE_VIBRATE_SLOPE;
				const uint8_t slope_start = NORM_CLAMP(trigger.effect.vibrate_slope.position.x, LIBRESENSE_TRIGGER_GRANULARITY);
				const uint8_t slope_end = NORM_CLAMP(trigger.effect.vibrate_slope.position.y, LIBRESENSE_TRIGGER_GRANULARITY);
				const uint8_t res_start = NORM_CLAMP(trigger.effect.vibrate_slope.amplitude.x, DUALSENSE_TRIGGER_STEP);
				const uint8_t res_end = NORM_CLAMP(trigger.effect.vibrate_slope.amplitude.y, DUALSENSE_TRIGGER_STEP);
				effect->params.multiple.id = (1 << (slope_start - 1)) | (1 << (slope_end - 1));
				effect->params.value[2] = res_start | res_end << DUALSENSE_TRIGGER_SHIFT;
				effect->params.value[3] = trigger.effect.vibrate_slope.frequency & UINT8_MAX;
				effect->params.value[4] = trigger.effect.vibrate_slope.period & UINT8_MAX;
				msg->flags.motor_power = true;
				msg->motor_flags.trigger_power_reduction = NORM_CLAMP(power_reduction, 7);
				break;
			}
		case LIBRESENSE_EFFECT_MUTIPLE_SECTIONS:
			{
				effect->mode = DUALSENSE_EFFECT_MODE_MUTIPLE_SECTIONS;
				for (int i = 0; i < LIBRESENSE_TRIGGER_GRANULARITY; ++i) {
					if (trigger.effect.multiple_sections.resistance[i] >= 0.01f) {
						effect->params.multiple.id |= 1 << i;
						effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_sections.resistance[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * i);
					}
				}
				break;
			}
		case LIBRESENSE_EFFECT_MUTIPLE_VIBRATE:
			{
				effect->mode = DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE;
				for (int i = 0; i < LIBRESENSE_TRIGGER_GRANULARITY; ++i) {
					if (trigger.effect.multiple_vibrate.amplitude[i] >= 0.01f) {
						effect->params.multiple.id |= 1 << i;
						effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_vibrate.amplitude[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * i);
					}
				}
				effect->params.multiple.value |= ((uint64_t) (trigger.effect.multiple_vibrate.frequency & UINT8_MAX)) << DUALSENSE_TRIGGER_FREQ_BITS;
				effect->params.multiple.value |= ((uint64_t) (trigger.effect.multiple_vibrate.period & UINT8_MAX)) << DUALSENSE_TRIGGER_PERD_BITS;
				msg->flags.motor_power = true;
				msg->motor_flags.trigger_power_reduction = NORM_CLAMP(power_reduction, 7);
				break;
			}
		case LIBRESENSE_EFFECT_MUTIPLE_VIBRATE_SECTIONS:
			{
				effect->mode = DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE_SECTIONS;
				// 9 and 10 run into the 7 and 8th bytes, which are used for freq and period which are... not used? setup from previous frames?
				// freq seems to be based off the speed the trigger is going.
				for (int i = 0; i < LIBRESENSE_TRIGGER_GRANULARITY; ++i) {
					// this is likely incorrect, have yet to find a real user of this
					if (trigger.effect.multiple_vibrate_sections.resistance[i] >= 0.01f) {
						effect->params.multiple.id |= 1 << i;
						effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_vibrate_sections.resistance[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * (i * 2));
					}
					if (trigger.effect.multiple_vibrate_sections.amplitude[i] >= 0.01f) {
						effect->params.multiple.id |= 1 << i;
						effect->params.multiple.value |= NORM_CLAMP(trigger.effect.multiple_vibrate_sections.amplitude[i], DUALSENSE_TRIGGER_STEP) << (DUALSENSE_TRIGGER_SHIFT * (i * 2 + 1));
					}
				}
				msg->flags.motor_power = true;
				msg->motor_flags.trigger_power_reduction = NORM_CLAMP(power_reduction, 7);
				break;
			}
		// ReSharper restore CppRedundantParentheses
		default: return LIBRESENSE_NOT_IMPLEMENTED;
	}

	return LIBRESENSE_OK;
}

// sanity check to make sure we don't (temporarily) brick the controller
libresense_result check_if_trigger_state_bad(const libresense_handle handle, const uint8_t id) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	const dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;
	if (hid_state->effects[id].mode >= 0xF0) { // these are calibration modes, will temporarily brick the controller!!
		return LIBRESENSE_INVALID_DATA;
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_update_effect(const libresense_handle handle, const libresense_effect_update left_trigger, const libresense_effect_update right_trigger, const float power_reduction) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return LIBRESENSE_NOT_SUPPORTED;
	}

	dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;
	hid_state->flags.left_trigger_motor = left_trigger.mode != LIBRESENSE_EFFECT_NONE;
	hid_state->flags.right_trigger_motor = right_trigger.mode != LIBRESENSE_EFFECT_NONE;

	libresense_result result = compute_effect(&hid_state->effects[ADAPTIVE_TRIGGER_LEFT], hid_state, left_trigger, power_reduction);
	if (IS_LIBRESENSE_OKAY(result)) {
		result = check_if_trigger_state_bad(handle, ADAPTIVE_TRIGGER_LEFT);
	}

	if (IS_LIBRESENSE_BAD(result)) {
		memset(&hid_state->effects[ADAPTIVE_TRIGGER_RIGHT], 0, sizeof(hid_state->effects[ADAPTIVE_TRIGGER_RIGHT]));
		hid_state->flags.left_trigger_motor = false;
		return result;
	}

	result = compute_effect(&hid_state->effects[ADAPTIVE_TRIGGER_RIGHT], hid_state, right_trigger, power_reduction);
	if (IS_LIBRESENSE_OKAY(result)) {
		result = check_if_trigger_state_bad(handle, ADAPTIVE_TRIGGER_RIGHT);
	}

	if (IS_LIBRESENSE_BAD(result)) {
		memset(&hid_state->effects[ADAPTIVE_TRIGGER_RIGHT], 0, sizeof(hid_state->effects[ADAPTIVE_TRIGGER_RIGHT]));
		hid_state->flags.right_trigger_motor = false;
		return result;
	}

	return result;
}

libresense_result libresense_update_rumble(const libresense_handle handle, const float large_motor, const float small_motor, const float power_reduction, const bool emulate_legacy_behavior) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return LIBRESENSE_NOT_SUPPORTED;
	}

	const libresense_hid hid = state[handle].hid_info;
	dualsense_output_msg* hid_state = &state[handle].output.data.msg.data;
	hid_state->flags.rumble = hid_state->flags.motor_power = true;

	if (hid.is_edge || hid.firmware.update.major >= 0x224) {
		hid_state->flags.control2 = true;
		hid_state->control2.advanced_rumble_control = emulate_legacy_behavior;
		hid_state->flags.haptics = !emulate_legacy_behavior;
	} else {
		hid_state->flags.haptics = true;
	}
	hid_state->motor_flags.rumble_power_reduction = NORM_CLAMP(power_reduction, 0x7);
	hid_state->rumble[DUALSENSE_LARGE_MOTOR] = NORM_CLAMP_UINT8(large_motor);
	hid_state->rumble[DUALSENSE_SMALL_MOTOR] = NORM_CLAMP_UINT8(small_motor);

	return LIBRESENSE_OK;
}

libresense_result libresense_bt_pair(const libresense_handle handle, const libresense_mac mac, const libresense_link_key link_key) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	dualsense_bt_pair_msg msg = { 0 };
	memcpy(&msg.link_key, link_key, sizeof(libresense_link_key));
	uint32_t pair_mac[6];
	const int test = sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", &pair_mac[0], &pair_mac[1], &pair_mac[2], &pair_mac[3], &pair_mac[4], &pair_mac[5]);
	if (test != 6) {
		return LIBRESENSE_INVALID_ARGUMENT;
	}

	msg.pair_mac[0] = pair_mac[0] & 0xFF;
	msg.pair_mac[1] = pair_mac[1] & 0xFF;
	msg.pair_mac[2] = pair_mac[2] & 0xFF;
	msg.pair_mac[3] = pair_mac[3] & 0xFF;
	msg.pair_mac[4] = pair_mac[4] & 0xFF;
	msg.pair_mac[5] = pair_mac[5] & 0xFF;
	msg.report_id = DUALSENSE_REPORT_PAIR;
	msg.checksum = libresense_calc_checksum(crc_seed_feature, (uint8_t*) &msg, sizeof(dualsense_bt_pair_msg) - 4);

	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &msg, sizeof(dualsense_bt_pair_msg)))) {
		return LIBRESENSE_HIDAPI_FAIL; // really only happens with bluetooth due to failed checksum
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_bt_connect(const libresense_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	dualsense_bt_command_msg msg = { 0 };
	msg.report_id = DUALSENSE_REPORT_COMMAND_BT;
	msg.command = DUALSENSE_BT_COMMAND_CONNECT;
	msg.checksum = libresense_calc_checksum(crc_seed_feature, (uint8_t*) &msg, sizeof(dualsense_bt_command_msg) - 4);

	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &msg, sizeof(dualsense_bt_command_msg)))) {
		return LIBRESENSE_HIDAPI_FAIL; // really only happens with bluetooth due to failed checksum
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_bt_disconnect(const libresense_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	dualsense_bt_command_msg msg = { 0 };
	msg.report_id = DUALSENSE_REPORT_COMMAND_BT;
	msg.command = DUALSENSE_BT_COMMAND_DISCONNECT;
	msg.checksum = libresense_calc_checksum(crc_seed_feature, (uint8_t*) &msg, sizeof(dualsense_bt_command_msg) - 4);

	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &msg, sizeof(dualsense_bt_command_msg)))) {
		return LIBRESENSE_HIDAPI_FAIL; // really only happens with bluetooth due to failed checksum
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_update_edge_profile(const libresense_handle handle, const libresense_profile_id id, const libresense_edge_profile profile) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	if (id == LIBRESENSE_PROFILE_NONE) {
		return LIBRESENSE_OK;
	}

	if (id <= LIBRESENSE_PROFILE_TRIANGLE || id > LIBRESENSE_PROFILE_CIRCLE) {
		return LIBRESENSE_INVALID_PROFILE;
	}

	dualsense_edge_profile_blob output[3];
	const libresense_result result = libresense_convert_edge_profile_output(profile, output);
	if (IS_LIBRESENSE_BAD(result)) {
		return result;
	}

	output[0].profile_part = 0;
	output[1].profile_part = 1;
	output[2].profile_part = 2;

	switch (id) {
		case LIBRESENSE_PROFILE_CIRCLE: output[0].report_id = output[1].report_id = output[2].report_id = DUALSENSE_REPORT_EDGE_UPDATE_PROFILE_CIRCLE; break;
		case LIBRESENSE_PROFILE_SQUARE: output[0].report_id = output[1].report_id = output[2].report_id = DUALSENSE_REPORT_EDGE_UPDATE_PROFILE_SQUARE; break;
		case LIBRESENSE_PROFILE_CROSS: output[0].report_id = output[1].report_id = output[2].report_id = DUALSENSE_REPORT_EDGE_UPDATE_PROFILE_CROSS; break;
		default: return LIBRESENSE_INVALID_PROFILE;
	}

	output[0].checksum = libresense_calc_checksum(crc_seed_feature_edge, (uint8_t*) &output[0], sizeof(output[0]) - 4);
	output[1].checksum = libresense_calc_checksum(crc_seed_feature_edge, (uint8_t*) &output[1], sizeof(output[1]) - 4);
	output[2].checksum = libresense_calc_checksum(crc_seed_feature_edge, (uint8_t*) &output[2], sizeof(output[2]) - 4);

	for (int i = 0; i < 3; ++i) {
		if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &output[i], sizeof(dualsense_edge_profile_blob)))) {
			return LIBRESENSE_HIDAPI_FAIL; // really only happens with bluetooth due to failed checksum
		}
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_delete_edge_profile(const libresense_handle handle, const libresense_profile_id id) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	if (id == LIBRESENSE_PROFILE_NONE) {
		return LIBRESENSE_OK;
	}

	if ((id <= LIBRESENSE_PROFILE_TRIANGLE || id > LIBRESENSE_PROFILE_CIRCLE) && id != LIBRESENSE_PROFILE_ALL) {
		return LIBRESENSE_INVALID_PROFILE;
	}

	dualsense_edge_profile_delete del = { 0 };
	del.report_id = DUALSENSE_REPORT_EDGE_DELETE_PROFILE;
	if (id == LIBRESENSE_PROFILE_ALL) {
		del.profile_id = 0xFF;
	} else {
		del.profile_id = id;
	}
	del.checksum = libresense_calc_checksum(crc_seed_feature_edge, (uint8_t*) &del, sizeof(del) - 4);
	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &del, sizeof(del)))) {
		return LIBRESENSE_HIDAPI_FAIL; // really only happens with bluetooth due to failed checksum
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_delete_access_profile(const libresense_handle handle, const libresense_profile_id id) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	if (id == LIBRESENSE_PROFILE_NONE) {
		return LIBRESENSE_OK;
	}

	if ((id <= LIBRESENSE_PROFILE_DEFAULT || id > LIBRESENSE_PROFILE_3) && id != LIBRESENSE_PROFILE_ALL) {
		return LIBRESENSE_INVALID_PROFILE;
	}

	dualsense_access_profile_blob del = { 0 };
	del.report_id = DUALSENSE_REPORT_ACCESS_SET_PROFILE;
	del.page_id = DUALSENSE_ACCESS_DELETE_PROFILE;
	if (id == LIBRESENSE_PROFILE_ALL) {
		del.profile_id = 0xFF;
	} else {
		del.profile_id = id;
	}
	del.checksum = libresense_calc_checksum(crc_seed_feature_edge, (uint8_t*) &del, sizeof(del) - 4);
	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &del, sizeof(del)))) {
		return LIBRESENSE_HIDAPI_FAIL; // really only happens with bluetooth due to failed checksum
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_close(const libresense_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE(handle);

	hid_close(state[handle].hid);
	memset(&state[handle], 0, sizeof(dualsense_state));

	return LIBRESENSE_OK;
}

void libresense_exit(void) {
	if (!is_initialized) {
		return;
	}

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		libresense_close(i);
	}

	hid_exit();

	is_initialized = false;
}

libresense_result libresense_debug_get_hid(const libresense_handle handle, intptr_t* hid) {
	CHECK_INIT();
	CHECK_HANDLE(handle);

	*hid = (intptr_t) state[handle].hid;

	return LIBRESENSE_OK;
}

libresense_result libresense_debug_get_hid_report_ids(const libresense_handle handle, libresense_report_id report_ids[0xFF]) {
	CHECK_INIT();
	CHECK_HANDLE(handle);

	uint8_t report[HID_API_MAX_REPORT_DESCRIPTOR_SIZE];
	const int report_size = hid_get_report_descriptor(state[handle].hid, report, HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

	memset(report_ids, 0, sizeof(libresense_report_id) * 0xFF);

	if (report_size > 7 && report_size < HID_API_MAX_REPORT_DESCRIPTOR_SIZE && report[0] == 0x05 && report[1] == 0x01 && // USAGE PAGE Generic Desktop
		report[2] == 0x09 && report[3] == 0x05 && // USAGE Game Pad
		report[4] == 0xA1 && report[5] == 0x01) {
		int report_id = 0;
		int last_size = 0;
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
					value = *(uint16_t*) (report + j);
					j += 2;
					break;
				case 3:
					if (j + 4 > report_size) {
						j += 4;
						break;
					}
					value = *(uint32_t*) (report + j);
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

				report_ids[report_id++].id = (uint8_t) value;
				report_ids[report_id - 1].size = last_size;
			} else if (report_id > 0) {
				if (op_value == 37) { // REPORT COUNT
					last_size = value;
					report_ids[report_id - 1].size = value;
				} else if (op_value == 32) { // INPUT
					report_ids[report_id - 1].type = 0;
				} else if (op_value == 36) { // OUTPUT
					report_ids[report_id - 1].type = 1;
				} else if (op_value == 44) { // FEATURE
					report_ids[report_id - 1].type = 2;
				}
			}
		}
	}

	return LIBRESENSE_OK;
}
