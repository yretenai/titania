#pragma once

#ifndef LIBRESENSE_STRUCTURES_H
#define LIBRESENSE_STRUCTURES_H

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

static_assert(sizeof(dualsense_report_id) == 1, "dualsense report id is not 1 byte");

typedef struct PACKED {
	uint8_t x;
	uint8_t y;
} dualsense_stick;

static_assert(sizeof(dualsense_stick) == 2, "dualsense_stick is not 2 bytes");

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
	bool unknown : 1;
	bool edge_f1 : 1;
	bool edge_f2 : 1;
	bool edge_lb : 1;
	bool edge_rb : 1;
	uint8_t padding : 8;
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

static_assert(sizeof(dualsense_battery_state) == 1, "dualsense_battery_state is not 1 byte");

static_assert(sizeof(libresense_device_state) == 1, "libresense_device_state is not 1 byte");

typedef struct PACKED {
	uint64_t value : 40;
} dualsense_uint40;

static_assert(sizeof(dualsense_uint40) == 0x5, "uint40 is not 5 bytes");

typedef struct PACKED {
	uint16_t unknown;
	libresense_edge_state edge_device;
	uint16_t unknown2;
	dualsense_battery_state battery;
	libresense_device_state device;
	uint8_t reserved;
} dualsense_device_state;

static_assert(sizeof(dualsense_device_state) == 8, "dualsense_deviec_state is not 8 bytes");

typedef struct PACKED {
	dualsense_vector3 accelerometer;
	dualsense_vector3 gyro;
	dualsense_uint40 time;
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
	uint32_t reserved;
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

	uint8_t reserved[9];
	uint32_t bt_checksum;
} dualsense_input_msg_ex;

static_assert(sizeof(dualsense_input_msg_ex) == 0x4e, "dualsense_input_msg_ex is not 78 bytes");

typedef union PACKED {
	struct {
		// byte 0
		bool gracefully_rumble : 1;
		bool allow_rumble_timeout : 1;
		bool right_trigger_motor : 1;
		bool left_trigger_motor : 1;
		bool jack : 1;
		bool speaker : 1;
		bool mic : 1;
		bool audio_output : 1;
		// byte 1
		bool mic_led : 1;
		bool mute : 1;
		bool led : 1;
		bool disable_led : 1;
		bool player_indicator_led : 1;
		bool advanced_rumble : 1; // related to adaptive triggers.
		bool trigger_rumble : 1;
		bool reserved : 1;		 // unused
	} bits;

	uint16_t value;
} dualsense_mutator_flags;

static_assert(sizeof(dualsense_mutator_flags) == 2, "dualsense_mutator_flags is not 2 bytes");

typedef struct PACKED {
	uint8_t mode;
	uint16_t unknown;
	uint8_t effect;
	uint8_t brightness;
	uint8_t led_id;
	dualsense_vector3b color;
} dualsense_led_output;

static_assert(sizeof(dualsense_led_output) == 9, "dualsense_led_output is not 9 bytes");

typedef struct PACKED {
	uint8_t mode;
	uint8_t values[10];
} dualsense_effect_output;

static_assert(sizeof(dualsense_effect_output) == 11, "dualsense_effect_output is not 11 bytes");

typedef struct PACKED {
	bool force_internal_mic : 1;
	bool force_external_mic : 1;
	bool balance_external_mic : 1;
	bool balance_internal_mic : 1;
	bool disable_jack : 1;
	bool enable_speaker : 1;
} dualsense_audio_flags;

static_assert(sizeof(dualsense_audio_flags) == 1, "dualsense_audio_flags is not 1 byte");

typedef enum ENUM_FORCE_8 {
	DUALSENSE_MIC_OFF = 0,
	DUALSENSE_MIC_ON = 1,
	DUALSENSE_MIC_FLASH = 2,
} dualsense_audio_mic_flags;

typedef struct PACKED {
	uint8_t jack;
	uint8_t speaker;
	uint8_t mic;
	dualsense_audio_flags flags;
	bool microphone_led;
	dualsense_audio_mic_flags mic_flags;
} dualsense_audio_output;

static_assert(sizeof(dualsense_audio_output) == 6, "dualsense_audio_output is not 6 bytes");

typedef struct PACKED {
	uint8_t rumble_power_reduction : 4;
	uint8_t trigger_power_reduction : 4;
} dualsense_motor_flags;
static_assert(sizeof(dualsense_motor_flags) == 1, "dualsense_motor_flags is not 1 byte");

typedef struct PACKED {
	dualsense_report_id id;
	dualsense_mutator_flags flags;
	uint8_t rumble[2];
	dualsense_audio_output audio;
	dualsense_effect_output effects[2];
	uint32_t state_id;
	dualsense_motor_flags motor_flags;
	uint8_t volume_state;
	dualsense_led_output led;
} dualsense_output_msg;

static_assert(sizeof(dualsense_output_msg) == 0x30, "dualsense_output_msg is not 48 bytes");

typedef struct PACKED {
	dualsense_report_id id;

	union {
		dualsense_output_msg data;
		uint8_t buffer[sizeof(dualsense_output_msg)];
	} msg;

	uint8_t reserved[25];
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

typedef struct PACKED {
	uint8_t op;
	libresense_vector3s gyro_bias;
	libresense_minmax gyro[3];
	libresense_minmax gyro_speed;
	libresense_minmax accelerometer[3];
	uint32_t crc;
	uint8_t padding[2];
} dualsense_calibration_info;

static_assert(sizeof(dualsense_calibration_info) == 41, "dualsense_calibration_info is not 41 bytes");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

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
	bool bt_initialized;
	uint16_t in_sequence;
	uint16_t out_sequence;

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

// Check if the library is initialized
#define CHECK_INIT       \
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

/**
 * @brief convert dualsense input report to libresense's representation
 * @param input: the input to convert
 * @param data: the data to convert into
 * @param calibration: calibration data
 */
void
libresense_convert_input(const dualsense_input_msg input, libresense_data *data, libresense_calibration_bit calibration[6]);

/**
 * @brief todo
 * convert a dualsense edge profile to libresense's representation
 * @param input: the input to convert
 * @param profile: the profile to convert into
 */
libresense_result
libresense_convert_edge_profile_input(const void *input, libresense_edge_profile *profile); // todo

/**
 * @brief todo
 * convert a libresense profile to dualsense edge's representation
 * @param input: the input to convert
 * @param profile: the profile to convert into
 */
libresense_result
libresense_convert_edge_profile_output(const libresense_edge_profile input, void *profile); // todo

/**
 * @brief get a HID feature report
 * @param handle: the device to query
 * @param report_id: the report to fetch
 * @param buffer: where to store the buffer
 * @param size: the size of the buffer
 * @param preserve: preserve byte 0
 */
size_t
libresense_get_feature_report(hid_device *handle, const int report_id, uint8_t *buffer, const size_t size, const bool preserve);

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

#endif
