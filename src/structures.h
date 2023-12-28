//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_STRUCTURES_H
#define LIBRESENSE_STRUCTURES_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <hidapi/hidapi.h>

#include "../include/libresense.h"
#include "config.h"

#ifdef _MSC_VER
#define PACKED
#else
#define PACKED __attribute__((__packed__))
#endif

static_assert(__STDC_VERSION__ >= 202000L, "a c2x compiler is required");

#define MAKE_EDGE_PROFILE_REPORT(id, name) DUALSENSE_EDGE_REPORT_##name##_P1 = id, DUALSENSE_EDGE_REPORT_##name##_P2 = id + 1, DUALSENSE_EDGE_REPORT_##name##_P3 = id + 2

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

#define DUALSENSE_LEFT (0)
#define DUALSENSE_RIGHT (1)
#define DUALSENSE_SMALL_MOTOR (0)
#define DUALSENSE_LARGE_MOTOR (1)
#define ADAPTIVE_TRIGGER_LEFT (1)
#define ADAPTIVE_TRIGGER_RIGHT (0)
#define DUALSENSE_CRC_INPUT (0xA1)
#define DUALSENSE_CRC_OUTPUT (0xA2)
#define DUALSENSE_CRC_FEATURE (0xA3)
#define DUALSENSE_TRIGGER_MAX (0xEE)
#define DUALSENSE_TRIGGER_AMPLITUDE_MAX (0x3F)
#define DUALSENSE_TRIGGER_VIBRATION_MAX (0xA8)
#define DUALSENSE_TRIGGER_STEP (7)
#define DUALSENSE_TRIGGER_SHIFT (3)
#define DUALSENSE_TRIGGER_FREQ_BITS (48)
#define DUALSENSE_TRIGGER_PERD_BITS (56)

#define NORM_CLAMP(value, max) (value >= 1.0f ? max : value <= 0.0f ? 0 : (uint8_t) (value * max))
#define NORM_CLAMP_UINT8(value) NORM_CLAMP(value, UINT8_MAX)

typedef enum _dualsense_report_id : uint8_t {
	// usb:
	DUALSENSE_REPORT_INPUT = 0x1,
	DUALSENSE_REPORT_OUTPUT = 0x2,
	DUALSENSE_REPORT_CALIBRATION = 0x5,
	DUALSENSE_REPORT_CONNECT = 0x8,
	DUALSENSE_REPORT_SERIAL = 0x9,
	DUALSENSE_REPORT_PAIR = 0x0A,
	DUALSENSE_REPORT_FIRMWARE = 0x20,
	DUALSENSE_REPORT_AUDIO = 0x21,
	DUALSENSE_REPORT_HARDWARE = 0x22,
	DUALSENSE_REPORT_SET_TEST = 0x80,
	DUALSENSE_REPORT_GET_TEST = 0x81,
	DUALSENSE_REPORT_RECALIBRATE = 0x83,
	DUALSENSE_REPORT_CALIBRATION_STATUS = 0x83,
	DUALSENSE_REPORT_SET_DATA = 0x84,
	DUALSENSE_REPORT_GET_DATA = 0x85,
	DUALSENSE_REPORT_SYS = 0xe0,
	DUALSENSE_REPORT_COMMAND_REQ = 0xf0,
	DUALSENSE_REPORT_STATUS = 0xf1,
	DUALSENSE_REPORT_COMMAND_RES = 0xf2,
	DUALSENSE_REPORT_SET_USER = 0xf4,
	DUALSENSE_REPORT_GET_USER = 0xf5,
	// edge:
	DUALSENSE_UPDATE_PROFILE_SQUARE = 0x60,
	DUALSENSE_UPDATE_PROFILE_CROSS = 0x61,
	DUALSENSE_UPDATE_PROFILE_CIRCLE = 0x62,
	MAKE_EDGE_PROFILE_REPORT(0x63, PROFILE_99), // base profile, maybe? empty
	DUALSENSE_DELETE_PROFILE = 0x68,
	MAKE_EDGE_PROFILE_REPORT(0x70, PROFILE_TRIANGLE),
	MAKE_EDGE_PROFILE_REPORT(0x73, PROFILE_SQUARE),
	MAKE_EDGE_PROFILE_REPORT(0x76, PROFILE_CROSS),
	MAKE_EDGE_PROFILE_REPORT(0x79, PROFILE_CIRCLE),
	// bluetooth:
	DUALSENSE_REPORT_BLUETOOTH = 0x31,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT1 = 0x32,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT2 = 0x33,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT3 = 0x34,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT4 = 0x35,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT5 = 0x36,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT6 = 0x37,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT7 = 0x38,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT8 = 0x39,
	// bluetooth + edge:
	DUALSENSE_REPORT_BLUETOOTH_SET_DATA = 0xf6,
	DUALSENSE_REPORT_BLUETOOTH_GET_DATA = 0xf7,
} dualsense_report_id;

static_assert(sizeof(dualsense_report_id) == 1, "dualsense report id is not 1 byte");

typedef struct PACKED {
	uint8_t x;
	uint8_t y;
} dualsense_stick;

static_assert(sizeof(dualsense_stick) == 2, "dualsense_stick is not 2 bytes");

typedef enum _dualsense_dpad : uint8_t {
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
	dualsense_dpad dpad : 4;
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
	bool reserved : 1;
	bool edge_f1 : 1;
	bool edge_f2 : 1;
	bool edge_lb : 1;
	bool edge_rb : 1;
	uint8_t edge_reserved : 8;
} dualsense_button;

static_assert(sizeof(dualsense_button) == 4, "dualsense_button is not 4 bytes");

typedef struct PACKED {
	int16_t x;
	int16_t y;
	int16_t z;
} dualsense_vector3;

static_assert(sizeof(dualsense_vector3) == 6, "dualsense_vector3 is not 6 bytes");

typedef struct PACKED {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} dualsense_vector3b;

static_assert(sizeof(dualsense_vector3b) == 3, "dualsense_vector3b is not 3 bytes");

typedef struct PACKED {
	uint16_t x : 12;
	uint16_t y : 12;
} dualsense_vector2;

static_assert(sizeof(dualsense_vector2) == 3, "dualsense_vector2 is not 3 bytes");

typedef struct PACKED {
	struct {
		uint8_t value : 7;
		bool idle : 1;
	} id;

	dualsense_vector2 coord;
} dualsense_touch;

static_assert(sizeof(dualsense_touch) == 4, "dualsense_touch is not 4 bytes");

typedef enum _dualsense_battery_state_v : uint8_t {
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

static_assert(sizeof(dualsense_battery_state) == 1, "dualsense_battery_state is not 1 byte");

static_assert(sizeof(libresense_device_state) == 2, "libresense_device_state is not 2 byte");

typedef struct PACKED {
	uint8_t right : 4;
	uint8_t left : 4;
} dualsense_trigger_state;

typedef struct PACKED {
	dualsense_trigger_state trigger;
	union {
		struct PACKED dualsense_device_state_edge {
			struct PACKED {
				bool unknown1 : 1;
				bool unknown2 : 1;
				bool led_indicator : 1;
				bool vibrate_indicator: 1;
				libresense_edge_profile_id id : 3;
				bool disable_switching : 1;
			} profile;
			bool stick_disconnected : 1;
			bool stick_error : 1;
			bool stick_calibrating : 1;
			bool stick_unknown : 1;
			libresense_level left_trigger_level : 2;
			libresense_level right_trigger_level : 2;
			struct {
				dualsense_dpad dpad : 4;
				bool square : 1;
				bool cross : 1;
				bool circle : 1;
				bool triangle : 1;
			} unmapped_buttons;
			struct PACKED {
				bool powersave_state : 1; // this is updated with motor power state
				libresense_level brightness_override : 2; // this is updated *somewhere* -> setting the entire report to 0xFF sets this to 0b11
				bool unknown3 : 1; // ??
				bool mute : 1;
				bool ps : 1;
				bool share : 1;
				bool option : 1;
			} unmapped_peculiar;
		} edge;
		static_assert(sizeof(struct dualsense_device_state_edge) == 4, "dualsense_device_state.edge is not 4 bytes");
		uint32_t battery_time; // why tf is this not reserved, sony please
	};
	dualsense_battery_state battery;
	libresense_device_state device;
} dualsense_device_state;

static_assert(sizeof(dualsense_device_state) == 8, "dualsense_device_state is not 8 bytes");

typedef struct PACKED {
	dualsense_vector3 accelerometer;
	dualsense_vector3 gyro;
	uint32_t time;
	uint8_t temperature;
} dualsense_sensors;

static_assert(sizeof(dualsense_sensors) == 0x11, "dualsense_sensors is not 17 bytes");

typedef struct PACKED {
	uint8_t id : 4;
	uint8_t level : 4;
} dualsense_adaptive_trigger;

typedef struct PACKED {
	dualsense_report_id id;
	dualsense_stick sticks[2];
	uint8_t triggers[2];
	uint8_t sequence;
	dualsense_button buttons;
	uint32_t firmware_time;
	dualsense_sensors sensors;
	dualsense_touch touch[2];
	uint8_t touch_sequence;
	dualsense_adaptive_trigger adaptive_triggers[2];
	uint32_t state_id;
	dualsense_device_state state;
	uint64_t checksum;
} dualsense_input_msg;

static_assert(sizeof(dualsense_input_msg) == 0x40, "dualsense_input_msg is not 64 bytes");

typedef struct PACKED {
	dualsense_report_id id;

	union {
		dualsense_input_msg data;
		uint8_t buffer[sizeof(dualsense_input_msg)];
	} msg;
	uint8_t connection;
	uint8_t failed_counter;
	uint8_t reserved[7];
	uint32_t bt_checksum;
} dualsense_input_msg_ex;

static_assert(sizeof(dualsense_input_msg_ex) == 0x4e, "dualsense_input_msg_ex is not 78 bytes");

typedef union PACKED {
	struct {
		// byte 0
		bool haptics : 1;
		bool rumble : 1;
		bool right_trigger_motor : 1;
		bool left_trigger_motor : 1;
		bool jack : 1;
		bool speaker : 1;
		bool mic : 1;
		bool audio_output : 1;
		// byte 1
		bool mic_led : 1;
		bool control1 : 1;
		bool led : 1;
		bool reset_led : 1;
		bool player_indicator_led : 1;
		bool haptic_filter : 1;
		bool motor_power : 1;
		bool control2 : 1;
	};

	uint16_t value;
} dualsense_mutator_flags;

static_assert(sizeof(dualsense_mutator_flags) == 2, "dualsense_mutator_flags is not 2 bytes");

typedef struct PACKED {
	uint8_t effect;
	uint8_t brightness;
	uint8_t led_id;
	dualsense_vector3b color;
} dualsense_led_output;

static_assert(sizeof(dualsense_led_output) == 6, "dualsense_led_output is not 6 bytes");

typedef enum _dualsense_effect_mode : uint8_t {
	DUALSENSE_EFFECT_MODE_OFF = 0x5,
	DUALSENSE_EFFECT_MODE_STOP = 0x0,
	DUALSENSE_EFFECT_MODE_UNIFORM = 0x1,
	DUALSENSE_EFFECT_MODE_SLOPE = 0x22,
	DUALSENSE_EFFECT_MODE_TRIGGER = 0x25,
	DUALSENSE_EFFECT_MODE_SECTION = 0x2,
	DUALSENSE_EFFECT_MODE_VIBRATE = 0x6,
	DUALSENSE_EFFECT_MODE_VIBRATE_SLOPE = 0x27,
	DUALSENSE_EFFECT_MODE_MUTIPLE_SECTIONS = 0x21,
	DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE = 0x26,
	DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE_SECTIONS = 0x23
} dualsense_effect_mode;

typedef struct PACKED {
	dualsense_effect_mode mode;
	union {
		uint8_t value[LIBRESENSE_TRIGGER_GRANULARITY];
		struct PACKED {
			uint16_t id;
			uint64_t value;
		} multiple;
	} params;
} dualsense_effect_output;

static_assert(sizeof(dualsense_effect_output) == 11, "dualsense_effect_output is not 11 bytes");

typedef struct PACKED {
	bool force_internal_mic : 1;
	bool force_external_mic : 1;
	bool echo_cancellation : 1;
	bool noise_cancellation : 1;
	bool balance_external_mic : 1;
	bool balance_internal_mic : 1;
	bool disable_jack : 1;
	bool enable_speaker : 1;
} dualsense_audio_flags;

static_assert(sizeof(dualsense_audio_flags) == 1, "dualsense_audio_flags is not 1 byte");

typedef enum _dualsense_audio_mic_flags : uint8_t {
	DUALSENSE_MIC_OFF = 0,
	DUALSENSE_MIC_ON = 1,
	DUALSENSE_MIC_FLASH = 2,
	DUALSENSE_MIC_FAST_FLASH = 3,
} dualsense_audio_mic_flags;

typedef struct PACKED {
	bool touch_powersave : 1;
	bool sensor_powersave : 1;
	bool rumble_powersave : 1;
	bool speaker_powersave : 1;
	bool mute_mic : 1;
	bool mute_speaker : 1;
	bool mute_jack : 1;
	bool disable_rumble : 1;
} dualsense_control1;
static_assert(sizeof(dualsense_control1) == 1, "dualsense_control1 is not 1 byte");

typedef struct PACKED {
	// audio control
	uint8_t gain : 3;
	bool enable_beamforming : 1;
	uint8_t reserved1 : 4;

	// interactive flags
	bool led_brightness_control : 1;
	bool led_color_control : 1;
	bool advanced_rumble_control : 1;
	uint8_t reserved2 : 3;
	bool has_edge_byte : 1;
	bool has_edge_misc : 1;

	// misc flags
	bool enable_lowpass_filter : 1;
	uint8_t reserved3 : 7;

	// todo: edge flags, something controls brightness
	// but i can only get it to happen if i set the entire struct to 0xFF
	uint8_t edge_unknown2 : 7;
	bool edge_disable_switching_profiles : 1;
} dualsense_control2;
static_assert(sizeof(dualsense_control2) == 4, "dualsense_control2 is not 4 bytes");

typedef struct PACKED {
	uint8_t jack;
	uint8_t speaker;
	uint8_t mic;
	dualsense_audio_flags flags;
	dualsense_audio_mic_flags mic_led_flags;
} dualsense_audio_output;

static_assert(sizeof(dualsense_audio_output) == 5, "dualsense_audio_output is not 5 bytes");

typedef struct PACKED {
	uint8_t rumble_power_reduction : 4;
	uint8_t trigger_power_reduction : 4;
} dualsense_motor_flags;

static_assert(sizeof(dualsense_motor_flags) == 1, "dualsense_motor_flags is not 1 byte");

typedef struct PACKED {
	bool enable_led : 1;
	bool enable_vibration : 1;
} dualsense_edge_indicator_update;

typedef union {
	struct PACKED {
		bool indicator : 1;
		uint8_t reserved : 7;
	};
	uint8_t value;
} dualsense_edge_mutator;

typedef struct PACKED {
	dualsense_edge_mutator flags;
	dualsense_edge_indicator_update indicator;
	uint8_t reserved[0xE];
} dualsense_edge_update;

typedef struct PACKED {
	dualsense_report_id id;
	dualsense_mutator_flags flags;
	uint8_t rumble[2];
	dualsense_audio_output audio;
	dualsense_control1 control1;
	dualsense_effect_output effects[2];
	uint32_t state_id;
	dualsense_motor_flags motor_flags;
	dualsense_control2 control2;
	dualsense_led_output led;
	dualsense_edge_update edge;
} dualsense_output_msg;

static_assert(sizeof(dualsense_output_msg) == 0x40, "dualsense_output_msg is not 64 bytes");

typedef struct PACKED {
	dualsense_report_id id;

	union {
		dualsense_output_msg data;
		uint8_t buffer[sizeof(dualsense_output_msg)];
	} msg;

	uint8_t reserved[9];
	uint32_t bt_checksum;
} dualsense_output_msg_ex;

static_assert(sizeof(dualsense_output_msg_ex) == 0x4e, "dualsense_output_msg_ex is not 78 bytes");

#define CALIBRATION_RAW_X 0
#define CALIBRATION_RAW_Y 1
#define CALIBRATION_RAW_Z 2

#define CALIBRATION_GYRO_X 0
#define CALIBRATION_GYRO_Y 1
#define CALIBRATION_GYRO_Z 2
#define CALIBRATION_ACCELEROMETER_X 3
#define CALIBRATION_ACCELEROMETER_Y 4
#define CALIBRATION_ACCELEROMETER_Z 5

#define DUALSENSE_GYRO_RESOLUTION 8096
#define DUALSENSE_ACCELEROMETER_RESOLUTION 1024

#define DUALSENSE_FIRMWARE_VERSION_DATE_LEN 0xB
#define DUALSENSE_FIRMWARE_VERSION_TIME_LEN 0x8

typedef struct PACKED {
	int16_t max;
	int16_t min;
} dualsense_minmax;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} dualsense_vector3s;

typedef struct PACKED {
	uint8_t op;
	dualsense_vector3s gyro_bias;
	dualsense_minmax gyro[3];
	dualsense_minmax gyro_speed;
	dualsense_minmax accelerometer[3];
	uint16_t flags;
	uint32_t checksum;
} dualsense_calibration_info;

static_assert(sizeof(dualsense_calibration_info) == 41, "dualsense_calibration_info is not 41 bytes");

typedef struct PACKED {
	uint16_t major;
	uint16_t minor;
} dualsense_firmware_version;

static_assert(sizeof(dualsense_firmware_version) == 4, "dualsense_firmware_version is 4 bytes");

typedef struct PACKED {
	uint8_t report_id;
	char date[DUALSENSE_FIRMWARE_VERSION_DATE_LEN];
	char time[DUALSENSE_FIRMWARE_VERSION_TIME_LEN];
	dualsense_firmware_version versions[LIBRESENSE_VERSION_MAX];
	uint32_t checksum;
} dualsense_firmware_info;

static_assert(DUALSENSE_FIRMWARE_VERSION_DATE_LEN + 1 + DUALSENSE_FIRMWARE_VERSION_TIME_LEN + 1 < LIBRESENSE_FIRMWARE_DATE_LEN,
	"date + space + time + null is >= libresense_firmware_info.version");
static_assert(sizeof(dualsense_firmware_info) == 64, "dualsense_firmware_info is not 64 bytes");

typedef struct PACKED {
	uint8_t report_id;
	uint8_t device_mac[6];
	uint8_t unknown[3];
	uint8_t pair_mac[6];
	uint32_t checksum;
} dualsense_serial_info;
static_assert(sizeof(dualsense_serial_info) == 20, "dualsense_serial_info is not 20 bytes");

typedef struct {
	float max;
	float min;
	int bias;
	int speed;
} libresense_calibration_bit;

typedef struct {
	hid_device *hid;
	libresense_hid hid_info;
	libresense_calibration_bit calibration[6];
	uint32_t seq;

	union {
		dualsense_input_msg_ex data;
		uint8_t buffer[sizeof(dualsense_input_msg_ex)];
	} input;

	union {
		dualsense_output_msg_ex data;
		uint8_t buffer[sizeof(dualsense_output_msg_ex)];
	} output;
} dualsense_state;

typedef struct {
	uint16_t vendor_id;
	uint16_t product_id;
} libresense_device_info;

typedef uint8_t dualsense_profile_guid[0x10];
typedef uint8_t dualsense_profile_data[0x40];

static_assert(sizeof(libresense_wchar) == 2, "libresense_wchar size is not 2");

typedef struct PACKED {
	uint8_t id;
	uint8_t part;
	uint32_t version;
	libresense_wchar name[27];
	uint32_t checksum;
} dualsense_profile_p1;
static_assert(sizeof(dualsense_profile_p1) == 64, "dualsense_profile_p1 size is not 64");

typedef struct PACKED {
	uint8_t id;
	uint8_t part;
	libresense_wchar name[13];
	dualsense_profile_guid guid;
	uint8_t left_stick_unknown1;
	uint16_t left_stick_unknown2;
	uint8_t left_stick_curve[6];
	uint8_t right_stick_unknown1;
	uint16_t right_stick_unknown2;
	uint8_t right_stick_curve[4];
	uint32_t checksum;
} dualsense_profile_p2;
static_assert(sizeof(dualsense_profile_p2) == 64, "dualsense_profile_p2 size is not 64");

typedef struct PACKED {
	uint8_t id;
	uint8_t part;
	uint8_t right_stick_curve[2];
	uint8_t left_trigger[2];
	uint8_t right_trigger[2];
	uint16_t unknown1;
	uint8_t buttons[0x12];
	uint16_t unknown2;
	uint16_t unknown3;
	uint16_t unknown4;
	uint64_t timestamp;
	uint8_t reserved[14];
	uint32_t full_checksum;
	uint32_t checksum;
} dualsense_profile_p3;
static_assert(sizeof(dualsense_profile_p3) == 64, "dualsense_profile_p3 size is not 64");

// Check if the library is initialized
#define CHECK_INIT()     \
	if (!is_initialized) \
		return LIBRESENSE_NOT_INITIALIZED

// Check if a handle is a valid number.
#define CHECK_HANDLE(h)                                                             \
	if (h == LIBRESENSE_INVALID_HANDLE || h < 0 || h >= LIBRESENSE_MAX_CONTROLLERS) \
		return LIBRESENSE_INVALID_HANDLE

// Check if a handle is a valid number, and that it has been initialized.
#define CHECK_HANDLE_VALID(h) \
	CHECK_HANDLE(h);          \
	if (state[h].hid == NULL) \
		return LIBRESENSE_INVALID_HANDLE

#define IS_EDGE(h) \
	(h.vendor_id == 0x054C && h.product_id == 0x0DF2)

#define CHECK_EDGE(h) \
	if (!IS_EDGE(state[h].hid_info)) \
		return LIBRESENSE_NOT_EDGE

extern uint32_t crc_seed_input;
extern uint32_t crc_seed_output;
extern uint32_t crc_seed_feature;

/**
 * @brief convert dualsense input report to libresense's representation
 * @param hid_info: hid device info
 * @param input: the input to convert
 * @param data: the data to convert into
 * @param calibration: calibration data
 */
void
libresense_convert_input(const libresense_hid hid_info, const dualsense_input_msg input, libresense_data *data, libresense_calibration_bit calibration[6]);

/**
 * @brief todo
 * convert a dualsense edge profile to libresense's representation
 * @param input: the input to convert
 * @param profile: the profile to convert into
 */
libresense_result
libresense_convert_edge_profile_input(dualsense_profile_data input[3], libresense_edge_profile *profile); // todo

/**
 * @brief todo
 * convert a libresense profile to dualsense edge's representation
 * @param input: the input to convert
 * @param profile: the profile to convert into
 */
libresense_result
libresense_convert_edge_profile_output(libresense_edge_profile input, dualsense_profile_data profile[3]); // todo

/**
 * @brief get a HID feature report
 * @param handle: the device to query
 * @param report_id: the report to fetch
 * @param buffer: where to store the buffer
 * @param size: the size of the buffer
 */
size_t
libresense_get_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size);

/**
 * @brief send a HID feature report
 * @param handle: the device to update
 * @param report_id: the report to send
 * @param buffer: where the buffer is
 * @param size: the size of the buffer
 * @param preserve: preserve byte 0
 */
size_t
libresense_send_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size, const bool preserve);

/**
 * @brief initializes checksum tables
 */
void
libresense_init_checksum(void);

/**
 * @brief calculates a bluetooth checksum
 * @param state: existing state.
 * @param buffer: data to hash
 * @param size: sizeof(buffer)
 */
uint32_t
libresense_calc_checksum(const uint32_t state, const uint8_t *buffer, const size_t size);

#endif
