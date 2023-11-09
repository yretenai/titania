#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include <hidapi/hidapi.h>

#include "../include/libresense.h"
#include "config.h"

#ifdef _MSC_VER
#define PACKED
#define ENUM_FORCE_8
#define ENUM_FORCE_16
#else
#define PACKED __attribute__((__packed__))
#define ENUM_FORCE_8 PACKED
#define ENUM_FORCE_16 PACKED
#define static_assert _Static_assert
#endif

const char *libresense_error_msg[ELIBRESENSE_ERROR_MAX + 1] = { "ok", "not initialized", "invalid handle", "invalid data", "out of range", "not implemented", "no available slots", NULL };
const char *libresense_battery_state_msg[ELIBRESENSE_BATTERY_MAX + 1] = { "unknown", "discharging", "charging", "full", NULL };
const char *libresense_edge_profile_id_msg[ELIBRESENSE_PROFILE_MAX + 1] = { "triangle", "cross", "square", "circle", NULL };
const int libresense_max_controllers = LIBRESENSE_MAX_CONTROLLERS;

#define MAKE_EDGE_PROFILE_REPORT(id, name) DUALSENSE_EDGE_REPORT_##name##_P1 = id, DUALSENSE_EDGE_REPORT_##name##_P2 = id + 1, DUALSENSE_EDGE_REPORT_##name##_P3 = id + 2

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef enum ENUM_FORCE_8 {
	DUALSENSE_REPORT_INPUT = 0x1,
	DUALSENSE_REPORT_OUTPUT = 0x2,
	DUALSENSE_REPORT_CALIBRATION = 0x5,
	DUALSENSE_REPORT_SERIAL = 0x9,
	DUALSENSE_REPORT_FIRMWARE = 0x20,
	DUALSENSE_REPORT_BLUETOOTH = 0x31,
	DUALSENSE_REPORT_129 = 0x81,				// zeroes?
	DUALSENSE_REPORT_131 = 0X83,				// -1 and then zeroes
	DUALSENSE_REPORT_133 = 0x85,				// 3 values
	DUALSENSE_REPORT_224 = 0xe0,				// zeroes
	DUALSENSE_REPORT_241 = 0xf1,				// zeroes
	DUALSENSE_REPORT_242 = 0xf2,				// 5 values then zeroes?
	DUALSENSE_REPORT_245 = 0xf5,				// 3 values
	MAKE_EDGE_PROFILE_REPORT(0x63, PROFILE_99), // base profile, maybe? empty
	MAKE_EDGE_PROFILE_REPORT(0x70, PROFILE_TRIANGLE),
	MAKE_EDGE_PROFILE_REPORT(0x73, PROFILE_SQUARE),
	MAKE_EDGE_PROFILE_REPORT(0x76, PROFILE_CROSS),
	MAKE_EDGE_PROFILE_REPORT(0x79, PROFILE_CIRCLE)
} dualsense_report_id;

static_assert(sizeof(dualsense_report_id) == 1, "Dualsense Report ID is not 1 byte");

typedef struct PACKED {
	uint8_t x;
	uint8_t y;
} dualsense_stick;

static_assert(sizeof(dualsense_stick) == 2, "size mismatch");

typedef enum PACKED {
	DUALSENSE_DPAD_U = 0,
	DUALSENSE_DPAD_UR = 1,
	DUALSENSE_DPAD_R = 2,
	DUALSENSE_DPAD_DR = 3,
	DUALSENSE_DPAD_D = 4,
	DUALSENSE_DPAD_DL = 5,
	DUALSENSE_DPAD_L = 6,
	DUALSENSE_DPAD_UL = 7,
	DUALSENSE_DPAD_RESET = 8
} dualsense_dpad;

typedef struct PACKED {
	dualsense_dpad dpad : 7;
	bool square : 1;
	bool cross : 1;
	bool circle : 1;
	bool triangle : 1;
	bool l1 : 1;
	bool r1 : 1;
	bool l2 : 1;
	bool r2 : 1;
	bool share : 1;
	bool option : 1;
	bool l3 : 1;
	bool r3 : 1;
	bool ps : 1;
	bool touch : 1;
	bool mute : 1;
	bool unknown : 1;
	bool edge_f1 : 1;
	bool edge_f2 : 1;
	bool edge_lb : 1;
	bool edge_rb : 1;
} dualsense_button;

static_assert(sizeof(dualsense_button) == 4, "size mismatch");

typedef struct PACKED {
	int16_t x;
	int16_t y;
	int16_t z;
} dualsense_vector3;

static_assert(sizeof(dualsense_vector3) == 6, "size mismatch");

typedef struct PACKED {
	uint16_t x : 12;
	uint16_t y : 12;
} dualsense_vector2;

static_assert(sizeof(dualsense_vector2) == 3, "size mismatch");

typedef struct PACKED {
	struct {
		uint8_t value : 7;
		bool idle : 1;
	} id;

	dualsense_vector2 coord;
} dualsense_touch;

static_assert(sizeof(dualsense_touch) == 4, "size mismatch");

typedef enum ENUM_FORCE_8 {
	DUALSENSE_BATTERY_STATE_DISCHARGING = 0x0,
	DUALSENSE_BATTERY_STATE_CHARGING = 0x1,
	DUALSENSE_BATTERY_STATE_FULL = 0x2,
	DUALSENSE_BATTERY_STATE_SAFETY = 0xa,
	DUALSENSE_BATTERY_STATE_SAFETY_TEMPERATURE = 0xb,
	DUALSENSE_BATTERY_STATE_SAFETY_VOLTAGE = 0xc,
	DUALSENSE_BATTERY_STATE_SAFETY_UNKNOWN_D = 0xd,
	DUALSENSE_BATTERY_STATE_SAFETY_UNKNOWN_E = 0xe,
	DUALSENSE_BATTERY_STATE_SAFETY_ERROR = 0xf
} dualsense_battery_state_v;

typedef struct PACKED {
	uint8_t level : 4;
	dualsense_battery_state_v state : 4;
} dualsense_battery_state;

static_assert(sizeof(dualsense_battery_state) == 1, "size mismatch");

typedef enum ENUM_FORCE_8 {
	DUALSENSE_STATE_AUDIO_HEADPHONES = 0x1,
	DUALSENSE_STATE_AUDIO_HEADSET = 0x2,
	DUALSENSE_STATE_AUDIO_MUTED = 0x4,
	DUALSENSE_STATE_CABLE_CONNECTED = 0x8
} dualsense_device_state;

typedef struct PACKED {
	dualsense_report_id id;
	dualsense_stick sticks[2];
	uint8_t triggers[2];
	uint8_t sequence;
	dualsense_button buttons;
	uint32_t firmware_time;
	dualsense_vector3 accelerometer;
	dualsense_vector3 gyro;
	uint32_t sensor_time;
	uint8_t sensor_reserved; // likely padding
	dualsense_touch touch[2];
	uint8_t adaptive_trigger_state; // random values? seems to go up every time the trigger info chages (including rumble)
	uint8_t adaptive_triggers[2];
	uint8_t adaptive_trigger_reserved; // likely padding
	uint8_t speaker_volume;			   // assumption
	uint8_t jack_volume;			   // assumption
	uint8_t mic_volume;				   // assumption
	uint8_t audio_reserved;			   // likely padding
	uint32_t audio_time;			   // assumption
	dualsense_battery_state battery;
	dualsense_device_state state;
	uint8_t state_reserved; // likely padding
	uint64_t checksum;
} dualsense_input_msg;

static_assert(sizeof(dualsense_input_msg) == 0x40, "size mismatch");

typedef struct PACKED {
	dualsense_report_id id;

	union {
		dualsense_input_msg data;
		uint8_t buffer[sizeof(dualsense_input_msg)];
	} msg;

	uint8_t reserved[9];
	uint32_t bt_checksum;
} dualsense_input_msg_ex;

static_assert(sizeof(dualsense_input_msg_ex) == 0x4e, "size mismatch");

typedef struct PACKED {
	bool rumble : 1;
	bool vibration_mode_compatible : 1;
	bool rt_rumble : 1;
	bool lt_rumble : 1;
	bool jack : 1;
	bool jack2 : 1; // set with jack
	bool mic : 1;
	bool speaker : 1;
	bool mic_led : 1;
	bool mute : 1;
	bool touch_led : 1;
	bool led : 1;
	bool player_led : 1;
	bool vibration_mode_advanced : 1;
	bool vibration_mode : 1; // modifying rumble at all sets this
	bool reserved : 1;		 // unused
} dualsense_mutator_flags;

static_assert(sizeof(dualsense_mutator_flags) == 2, "size mismatch");

typedef struct PACKED {
	uint8_t vol;
} dualsense_audio_output;

typedef struct PACKED {
	dualsense_mutator_flags flags;
} dualsense_output_msg;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct {
	hid_device *hid;
	libresense_hid hid_info;
	bool bt_initialized;

	union {
		dualsense_input_msg_ex data;
		uint8_t buffer[sizeof(dualsense_input_msg_ex)];
	} input;

	union {
		dualsense_output_msg data;
		uint8_t buffer[sizeof(dualsense_output_msg)];
	} output;
} dualsense_state;

typedef struct {
	uint16_t vendor_id;
	uint16_t product_id;
} libresense_device_info;

static libresense_device_info device_infos[] = {
	{0x054C,  0x0CE6}, // DualSense
	{ 0x054C, 0x0DF2}, // DualSense Edge
};

static dualsense_state state[LIBRESENSE_MAX_CONTROLLERS];
bool is_initialized = false;

#define CHECK_INIT       \
	if (!is_initialized) \
	return ELIBRESENSE_NOT_INITIALIZED
#define CHECK_HANDLE(h)                                                            \
	if (h == LIBRESENSE_INVALID_HANDLE || h < 0 || h > LIBRESENSE_MAX_CONTROLLERS) \
	return ELIBRESENSE_INVALID_HANDLE
#define CHECK_HANDLE_VALID(h) \
	CHECK_HANDLE(h);          \
	if (state[h].hid == NULL) \
	return ELIBRESENSE_INVALID_HANDLE

void
ascii_serial(const wchar_t *wserial, libresense_serial *serial) {
	mbstate_t state = { 0 };
	wcsrtombs((char *) serial, &wserial, sizeof(libresense_serial), &state);
}

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

			if (dev->bus_type != HID_API_BUS_USB && dev->bus_type != HID_API_BUS_BLUETOOTH) {
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
libresense_get_feature_report(hid_device *handle, int report_id, uint8_t *buffer, size_t size, bool preserve) {
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
libresense_send_feature_report(hid_device *handle, int report_id, uint8_t *buffer, size_t size, bool preserve) {
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

libresense_result
libresense_open(libresense_hid *handle) {
	CHECK_INIT;

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		if (state[i].hid == NULL) {
			handle->handle = i;
			state[i].hid = hid_open(handle->vendor_id, handle->product_id, handle->serial);

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
			return ELIBRESENSE_OK;
		}
	}

	return ELIBRESENSE_NO_SLOTS;
}

void
convert_input(const dualsense_input_msg input, libresense_data *data) { }

libresense_result
libresense_poll(libresense_handle *handle, int handle_count, libresense_data *data) {
	CHECK_INIT;
	if (handle == NULL || data == NULL) {
		return ELIBRESENSE_INVALID_DATA;
	}

	if (handle_count <= 0) {
		return ELIBRESENSE_OK;
	}

	libresense_data invalid = { 0 };
	invalid.hid.handle = LIBRESENSE_INVALID_HANDLE;

	for (int i = 0; i < handle_count; i++) {
		CHECK_HANDLE_VALID(handle[i]);
		dualsense_state *hid_state = &state[handle[i]];
		data[i].hid = hid_state->hid_info;
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

		convert_input(hid_state->input.data.msg.data, &data[i]);
	}

	return ELIBRESENSE_OK;
}

libresense_result
libresense_close(const libresense_handle handle) {
	CHECK_INIT;
	CHECK_HANDLE(handle);

	state[handle].hid = (hid_close(state[handle].hid), NULL);
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
