//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIA_H
#define TITANIA_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WCHAR_T_DEFINED
#include <wchar.h>
#endif

#ifndef __cplusplus
#if __STDC_VERSION__ < 202000L
#include <stdbool.h>
#endif
#endif

#include <stdint.h>

#ifdef _WIN32
#ifdef TITANIA_EXPORTING
#define TITANIA_EXPORT __declspec(dllexport)
#else
#define TITANIA_EXPORT __declspec(dllimport)
#endif
#else
#define TITANIA_EXPORT __attribute__((visibility("default")))
#endif

#define TITANIA_INVALID_HANDLE_ID (-1)

#define TITANIA_LEFT (0)
#define TITANIA_RIGHT (1)
#define TITANIA_PRIMARY (0)
#define TITANIA_SECONDARY (1)
#define TITANIA_EXTENSION1 (0)
#define TITANIA_EXTENSION2 (1)
#define TITANIA_EXTENSION3 (2)
#define TITANIA_EXTENSION4 (3)
#define TITANIA_MIN_DELAY (125)
#define TITANIA_POLLING_RATE_BT (1)
#define TITANIA_POLLING_RATE_USB (4)
#define TITANIA_TRIGGER_GRANULARITY (10)
#define TITANIA_FIRMWARE_DATE_LEN (0x20)
#define TITANIA_MERGED_REPORT_EDGE_SIZE (174)
#define TITANIA_MERGED_REPORT_ACCESS_SIZE (960)
#define TITANIA_ACCESS_BUTTON_CENTER (0)
#define TITANIA_ACCESS_BUTTON_B1 (1)
#define TITANIA_ACCESS_BUTTON_B2 (2)
#define TITANIA_ACCESS_BUTTON_B3 (3)
#define TITANIA_ACCESS_BUTTON_B4 (4)
#define TITANIA_ACCESS_BUTTON_B5 (5)
#define TITANIA_ACCESS_BUTTON_B6 (6)
#define TITANIA_ACCESS_BUTTON_B7 (7)
#define TITANIA_ACCESS_BUTTON_B8 (8)
#define TITANIA_ACCESS_BUTTON_STICK (9)

typedef enum titania_result {
	TITANIA_OK = 0,
	TITANIA_NOT_INITIALIZED,
	TITANIA_INVALID_LIBRARY,
	TITANIA_INVALID_HANDLE_DATA,
	TITANIA_INVALID_HANDLE,
	TITANIA_INVALID_DATA,
	TITANIA_INVALID_PROFILE,
	TITANIA_INVALID_ARGUMENT,
	TITANIA_HIDAPI_FAIL,
	TITANIA_UNICODE_ERROR,
	TITANIA_OUT_OF_RANGE,
	TITANIA_NOT_IMPLEMENTED,
	TITANIA_NO_SLOTS,
	TITANIA_NOT_EDGE,
	TITANIA_NOT_ACCESS,
	TITANIA_NOT_SUPPORTED,
	TITANIA_ERROR_MAX
} titania_result;

typedef enum titania_battery_state {
	TITANIA_BATTERY_UNKNOWN = 0,
	TITANIA_BATTERY_DISCHARGING,
	TITANIA_BATTERY_CHARGING,
	TITANIA_BATTERY_FULL,
	TITANIA_BATTERY_MAX
} titania_battery_state;

typedef enum titania_profile_id {
	TITANIA_PROFILE_NONE,
	TITANIA_PROFILE_TRIANGLE,
	TITANIA_PROFILE_SQUARE,
	TITANIA_PROFILE_CROSS,
	TITANIA_PROFILE_CIRCLE,
	TITANIA_PROFILE_MAX_VALID,

	TITANIA_PROFILE_ALL,
	TITANIA_PROFILE_MAX_META,

	TITANIA_PROFILE_DEFAULT = TITANIA_PROFILE_TRIANGLE,
	TITANIA_PROFILE_1 = TITANIA_PROFILE_SQUARE,
	TITANIA_PROFILE_2 = TITANIA_PROFILE_CROSS,
	TITANIA_PROFILE_3 = TITANIA_PROFILE_CIRCLE,
	TITANIA_PROFILE_COUNT = 4,
} titania_profile_id;

typedef enum titania_level {
	TITANIA_LEVEL_HIGH = 0,
	TITANIA_LEVEL_MEDIUM = 1,
	TITANIA_LEVEL_LOW = 2,
	TITANIA_LEVEL_OFF = 3
} titania_level;

typedef enum titania_access_extension_id {
	TITANIA_ACCESS_EXTENSION_DISCONNECTED = 0,
	TITANIA_ACCESS_EXTENSION_BUTTON = 1,
	TITANIA_ACCESS_EXTENSION_TRIGGER = 2,
	TITANIA_ACCESS_EXTENSION_STICK = 3,
	TITANIA_ACCESS_EXTENSION_INVALID = 4,
	TITANIA_ACCESS_EXTENSION_MAX
} titania_access_extension_id;

typedef enum titania_trigger_effect_state {
	TITANIA_TRIGGER_EFFECT_OFF = 0,
	TITANIA_TRIGGER_EFFECT_UNIFORM = 1,
	TITANIA_TRIGGER_EFFECT_TRIGGER = 2,
	TITANIA_TRIGGER_EFFECT_VIBRATION = 3,
	TITANIA_TRIGGER_EFFECT_MIXED = 4,
	TITANIA_TRIGGER_EFFECT_UNKNOWN5 = 5,
	TITANIA_TRIGGER_EFFECT_UNKNOWN6 = 6,
	TITANIA_TRIGGER_EFFECT_UNKNOWN7 = 7,
	TITANIA_TRIGGER_EFFECT_UNKNOWN8 = 8,
	TITANIA_TRIGGER_EFFECT_UNKNOWN9 = 9,
	TITANIA_TRIGGER_EFFECT_UNKNOWN10 = 10,
	TITANIA_TRIGGER_EFFECT_UNKNOWN11 = 11,
	TITANIA_TRIGGER_EFFECT_UNKNOWN12 = 12,
	TITANIA_TRIGGER_EFFECT_UNKNOWN13 = 13,
	TITANIA_TRIGGER_EFFECT_UNKNOWN14 = 14,
	TITANIA_TRIGGER_EFFECT_UNKNOWN15 = 15,
	TITANIA_TRIGGER_EFFECT_MAX
} titania_trigger_effect_state;

typedef enum titania_edge_button_id {
	TITANIA_BUTTON_ID_UP,
	TITANIA_BUTTON_ID_LEFT,
	TITANIA_BUTTON_ID_DOWN,
	TITANIA_BUTTON_ID_RIGHT,
	TITANIA_BUTTON_ID_CIRCLE,
	TITANIA_BUTTON_ID_CROSS,
	TITANIA_BUTTON_ID_SQUARE,
	TITANIA_BUTTON_ID_TRIANGLE,
	TITANIA_BUTTON_ID_R1,
	TITANIA_BUTTON_ID_R2,
	TITANIA_BUTTON_ID_R3,
	TITANIA_BUTTON_ID_L1,
	TITANIA_BUTTON_ID_L2,
	TITANIA_BUTTON_ID_L3,
	TITANIA_BUTTON_ID_LEFT_PADDLE,
	TITANIA_BUTTON_ID_RIGHT_PADDLE,
	TITANIA_BUTTON_ID_OPTION,
	TITANIA_BUTTON_ID_TOUCH,
	TITANIA_BUTTON_ID_MAX
} titania_edge_button_id;

typedef enum titania_edge_stick_template {
	TITANIA_EDGE_STICK_TEMPLATE_DEFAULT,
	TITANIA_EDGE_STICK_TEMPLATE_QUICK,
	TITANIA_EDGE_STICK_TEMPLATE_PRECISE,
	TITANIA_EDGE_STICK_TEMPLATE_STEADY,
	TITANIA_EDGE_STICK_TEMPLATE_DIGITAL,
	TITANIA_EDGE_STICK_TEMPLATE_DYNAMIC,
	TITANIA_EDGE_STICK_TEMPLATE_MAX
} titania_edge_stick_template;

// NOTE: 1 and 2 may be valid, but it is untested!
// Also I don't know if this is actually interpolation!
// Only two templates that have Smooth is Precise and Steady
typedef enum titania_edge_interpolation_type {
	TITANIA_EDGE_INTERPOLATION_TYPE_NONE = 0,
	TITANIA_EDGE_INTERPOLATION_TYPE_UNKNOWN1 = 1,
	TITANIA_EDGE_INTERPOLATION_TYPE_UNKNOWN2 = 2,
	TITANIA_EDGE_INTERPOLATION_TYPE_LINEAR = 3,
	TITANIA_EDGE_INTERPOLATION_TYPE_SMOOTH = 4
} titania_edge_interpolation_type;

typedef enum titania_effect_mode {
	TITANIA_EFFECT_NONE = -1,
	TITANIA_EFFECT_OFF = 0,
	TITANIA_EFFECT_STOP_VIBRATING,
	TITANIA_EFFECT_UNIFORM,
	TITANIA_EFFECT_SLOPE,
	TITANIA_EFFECT_TRIGGER,
	TITANIA_EFFECT_SECTION,
	TITANIA_EFFECT_VIBRATE,
	TITANIA_EFFECT_VIBRATE_SLOPE,
	TITANIA_EFFECT_MUTIPLE_SECTIONS,
	TITANIA_EFFECT_MUTIPLE_VIBRATE,
	TITANIA_EFFECT_MUTIPLE_VIBRATE_SECTIONS
} titania_effect_mode;

typedef enum titania_led_effect {
	TITANIA_LED_EFFECT_OFF = 0,
	TITANIA_LED_EFFECT_RESET = 1,
	TITANIA_LED_EFFECT_FADE_OUT = 2
} titania_led_effect;

typedef enum titania_led_index {
	TITANIA_LED_NONE = 0,
	TITANIA_LED_PLAYER_1 = 4,
	TITANIA_LED_PLAYER_2 = 10,
	TITANIA_LED_PLAYER_3 = 21,
	TITANIA_LED_PLAYER_4 = 27,
	TITANIA_LED_1 = 1,
	TITANIA_LED_2 = 2,
	TITANIA_LED_3 = 4,
	TITANIA_LED_4 = 8,
	TITANIA_LED_5 = 16,
	TITANIA_LED_ACCESS_1 = 1,
	TITANIA_LED_ACCESS_2 = 2,
	TITANIA_LED_ACCESS_3 = 3,
	TITANIA_LED_ACCESS_4 = 4,
	TITANIA_LED_ALL = 31,
	TITANIA_LED_INSTANT = 32,
	TITANIA_LED_NO_UPDATE = 128
} titania_led_index;

typedef enum titania_audio_mic {
	TITANIA_MIC_AUTO = 0,
	TITANIA_MIC_INTERNAL = 1,
	TITANIA_MIC_EXTERNAL = 2,
	TITANIA_MIC_BOTH = 3
} titania_audio_mic;

typedef enum titania_mic_led {
	TITANIA_MIC_LED_OFF = 0,
	TITANIA_MIC_LED_ON = 1,
	TITANIA_MIC_LED_FLASH = 2,
	TITANIA_MIC_LED_FAST_FLASH = 3
} titania_mic_led;

typedef enum titania_access_orientation {
	TITANIA_ACCESS_ORIENTATION_DOWN = 0,
	TITANIA_ACCESS_ORIENTATION_LEFT = 1,
	TITANIA_ACCESS_ORIENTATION_UP = 2,
	TITANIA_ACCESS_ORIENTATION_RIGHT = 3
} titania_access_orientation;

typedef enum titania_access_button_id {
	TITANIA_ACCESS_BUTTON_ID_NONE = 0,
	TITANIA_ACCESS_BUTTON_ID_CIRCLE = 1,
	TITANIA_ACCESS_BUTTON_ID_CROSS = 2,
	TITANIA_ACCESS_BUTTON_ID_TRIANGLE = 3,
	TITANIA_ACCESS_BUTTON_ID_SQUARE = 4,
	TITANIA_ACCESS_BUTTON_ID_UP = 5,
	TITANIA_ACCESS_BUTTON_ID_DOWN = 6,
	TITANIA_ACCESS_BUTTON_ID_LEFT = 7,
	TITANIA_ACCESS_BUTTON_ID_RIGHT = 8,
	TITANIA_ACCESS_BUTTON_ID_L1 = 9,
	TITANIA_ACCESS_BUTTON_ID_R1 = 10,
	TITANIA_ACCESS_BUTTON_ID_L2 = 11,
	TITANIA_ACCESS_BUTTON_ID_R2 = 12,
	TITANIA_ACCESS_BUTTON_ID_L3 = 13,
	TITANIA_ACCESS_BUTTON_ID_R3 = 14,
	TITANIA_ACCESS_BUTTON_ID_OPTIONS = 15,
	TITANIA_ACCESS_BUTTON_ID_CREATE = 16,
	TITANIA_ACCESS_BUTTON_ID_PLAYSTATION = 17,
	TITANIA_ACCESS_BUTTON_ID_TOUCH = 18,
	TITANIA_ACCESS_BUTTON_ID_MAX
} titania_access_button_id;

typedef enum titania_access_stick_id {
	TITANIA_ACCESS_STICK_ID_NONE = 0,
	TITANIA_ACCESS_STICK_ID_LEFT = 1,
	TITANIA_ACCESS_STICK_ID_RIGHT = 2,
	TITANIA_ACCESS_STICK_ID_MAX
} titania_access_stick_id;

// why
typedef enum titania_access_extension_type_id {
	TITANIA_ACCESS_EXTENSION_TYPE_DISCONNECTED = 0,
	TITANIA_ACCESS_EXTENSION_TYPE_STICK = 1,
	TITANIA_ACCESS_EXTENSION_TYPE_BUTTON = 2,
	TITANIA_ACCESS_EXTENSION_TYPE_MAX
} titania_access_extension_type_id;

TITANIA_EXPORT extern const char* const titania_error_msg[TITANIA_ERROR_MAX + 1];
TITANIA_EXPORT extern const char* const titania_battery_state_msg[TITANIA_BATTERY_MAX + 1];
TITANIA_EXPORT extern const char* const titania_profile_id_msg[TITANIA_PROFILE_MAX_META + 1];
TITANIA_EXPORT extern const char* const titania_profile_id_alt_msg[TITANIA_PROFILE_MAX_META + 1];
TITANIA_EXPORT extern const char* const titania_level_msg[TITANIA_LEVEL_LOW + 2];
TITANIA_EXPORT extern const char* const titania_trigger_effect_msg[TITANIA_TRIGGER_EFFECT_MAX + 1];
TITANIA_EXPORT extern const char* const titania_edge_button_id_msg[TITANIA_BUTTON_ID_MAX + 1];
TITANIA_EXPORT extern const char* const titania_edge_button_id_alt_msg[TITANIA_BUTTON_ID_MAX + 1];
TITANIA_EXPORT extern const char* const titania_access_extension_id_msg[TITANIA_ACCESS_EXTENSION_MAX + 1];
TITANIA_EXPORT extern const char* const titania_edge_stick_template_msg[TITANIA_EDGE_STICK_TEMPLATE_MAX + 1];
TITANIA_EXPORT extern const int titania_max_controllers;

#define IS_TITANIA_OKAY(result) (result == TITANIA_OK)
#define IS_TITANIA_BAD(result) (result != TITANIA_OK)
typedef uint16_t titania_wchar;

typedef signed int titania_handle;
typedef wchar_t titania_serial[0x100]; // Max HID Parameter length is 256 on USB, 512 on BT. HID serials are wide-chars, which are 2 bytes.
typedef char titania_hid_path[0x200];

typedef char titania_mac[0x12];
typedef char titania_link_key[0x10];

typedef struct titania_vector2 {
	union {
		float x;
		float r;
		float min;
	};

	union {
		float y;
		float g;
		float max;
	};
} titania_vector2;

typedef struct titania_vector2i {
	int x;
	int y;
} titania_vector2i;

typedef struct titania_vector3 {
	union {
		float x;
		float r;
	};

	union {
		float y;
		float g;
	};

	union {
		float z;
		float b;
	};
} titania_vector3;

typedef struct titania_buttons {
	bool dpad_up;
	bool dpad_right;
	bool dpad_down;
	bool dpad_left;
	bool square;
	bool cross;
	bool circle;
	bool triangle;
	bool l1;
	bool r1;
	bool l2;
	bool r2;
	bool create;
	bool option;
	bool l3;
	bool r3;
	bool playstation;
	bool touch;
	bool mute;
	bool reserved;
	bool edge_f1;
	bool edge_f2;
	bool edge_left_paddle;
	bool edge_right_paddle;
	uint8_t edge_reserved;
	bool touchpad;
} titania_buttons;

typedef struct titania_trigger {
	float level;
	uint8_t id;
	uint8_t section;
	titania_trigger_effect_state effect;
} titania_trigger;

typedef struct titania_touchpad {
	uint32_t id;
	bool active;
	titania_vector2i pos;
} titania_touchpad;

typedef struct titania_time {
	uint8_t sequence;
	uint8_t touch_sequence;
	uint32_t driver_sequence;
	uint32_t system;
	uint32_t battery; // does not exist in edge because the field is re-used
	uint32_t sensor;
	uint64_t checksum;
} titania_time;

typedef struct titania_sensors {
	titania_vector3 accelerometer;
	titania_vector3 gyro;
	int32_t temperature;
} titania_sensors;

typedef struct titania_device_state {
	bool headphones;
	bool headset;
	bool muted;
	bool usb_data;
	bool usb_power;
	bool external_mic;
	bool haptic_filter;
	uint16_t reserved;
} titania_device_state;

typedef struct titania_battery {
	float level;
	titania_battery_state state;
} titania_battery;

typedef struct titania_firmware_version {
	uint16_t major;
	uint16_t minor;
	uint16_t revision;
} titania_firmware_version;

typedef struct titania_firmware_hardware {
	uint16_t reserved;
	uint16_t variation;
	uint16_t generation;
	uint16_t revision;
} titania_firmware_hardware;

typedef struct titania_firmware_info {
	char datetime[TITANIA_FIRMWARE_DATE_LEN];
	uint16_t type;
	// 0x0004 = DualSense (0xCE6, Bond)
	// 0x???? = DualSense Prototype (0xCE7, Aston)
	// 0x0044 = DualSense Edge (0xDF2, ?? pick a bond car lol i'm guessing vanquish or valhalla)
	// 0x0060 = Playstation Access (0xE5F, Leonardo)
	uint16_t series;
	titania_firmware_hardware hardware;
	titania_firmware_version update;
	titania_firmware_version firmware;
	titania_firmware_version firmware2;
	titania_firmware_version firmware3;
	titania_firmware_version device;
	titania_firmware_version device2;
	titania_firmware_version device3;
	titania_firmware_version mcu_firmware;
} titania_firmware_info;

typedef struct titania_serial_info {
	titania_mac mac;
	titania_mac paired_mac;
	uint32_t unknown;
} titania_serial_info;

typedef struct titania_edge_stick {
	titania_edge_stick_template template_id;
	titania_vector2 deadzone;
	titania_vector2 curve_points[3];
	titania_edge_interpolation_type interpolation_type;
	uint8_t unknown;
	bool disabled;
} titania_edge_stick;

typedef struct titania_edge_trigger {
	titania_vector2 deadzone;
} titania_edge_trigger;

typedef union titania_edge_button_remap {
	struct {
		titania_edge_button_id up;
		titania_edge_button_id down;
		titania_edge_button_id left;
		titania_edge_button_id right;
		titania_edge_button_id circle;
		titania_edge_button_id cross;
		titania_edge_button_id square;
		titania_edge_button_id triangle;
		titania_edge_button_id r1;
		titania_edge_button_id r2;
		titania_edge_button_id r3;
		titania_edge_button_id l1;
		titania_edge_button_id l2;
		titania_edge_button_id l3;
		titania_edge_button_id left_paddle;
		titania_edge_button_id right_paddle;
	};

	titania_edge_button_id values[0x10];
} titania_edge_button_remap;

typedef struct titania_edge_profile {
	bool valid;
	uint32_t version;
	char name[0xA1];
	uint8_t id[0x10];
	titania_edge_stick sticks[2];
	titania_edge_trigger triggers[2];
	titania_edge_button_remap buttons;
	titania_level vibration;
	titania_level trigger_effect;
	bool sticks_swapped;
	bool trigger_deadzone_mirrored;
	titania_buttons disabled_buttons;
	uint32_t unknown;
	uint64_t timestamp;
} titania_edge_profile;

typedef struct titania_access_profile {
	bool valid;
	uint32_t version;
	char name[0xA1];
	uint8_t id[0x10];
} titania_access_profile;

typedef struct titania_query {
	uint16_t product_id;
	uint16_t vendor_id;
	bool is_bluetooth;
	bool is_edge;
	bool is_access;
	titania_serial hid_serial;
	titania_hid_path hid_path;
} titania_query;

typedef struct titania_hid {
	titania_handle handle;
	uint16_t product_id;
	uint16_t vendor_id;
	bool is_bluetooth;
	bool is_edge;
	bool is_access;
	titania_serial_info serial;
	titania_firmware_info firmware;
} titania_hid;

typedef struct titania_report_id {
	uint8_t id;
	uint8_t type;
	uint32_t size;
} titania_report_id;

typedef struct titania_bt {
	bool has_hid;
	bool unknown;
	bool unknown2;
	bool unknown3;
	uint8_t seq;
} titania_bt;

typedef struct titania_edge_state {
	titania_buttons raw_buttons;

	struct titania_edge_state_stick {
		bool disconnected;
		bool errored;
		bool calibrating;
		bool unknown;
	} stick;

	titania_level trigger_levels[2];
	titania_profile_id current_profile_id;

	struct titania_edge_state_indicator {
		bool led;
		bool vibration;
		bool switching_disabled;
		bool unknown1;
		bool unknown2;
	} profile_indicator;

	titania_level brightness;
	bool emulating_rumble;
	uint8_t unknown;
} titania_edge_state;

typedef struct titania_access_button {
	bool button1;
	bool button2;
	bool button3;
	bool button4;
	bool button5;
	bool button6;
	bool button7;
	bool button8;
	bool center_button;
	bool stick_button;
	bool playstation;
	bool profile;
	bool e1;
	bool e2;
	bool e3;
	bool e4;
	uint8_t reserved;
} titania_access_button;

typedef struct titania_access_extension {
	titania_vector2 pos;
	titania_access_extension_id type;
} titania_access_extension;

typedef struct titania_access_state {
	titania_access_button buttons;
	titania_vector2 raw_stick;
	titania_vector2 sticks[2];
	titania_profile_id current_profile_id;
	bool profile_switching_disabled;
	titania_access_extension extensions[4];
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t unknown4;
	uint32_t unknown5;
	uint32_t unknown6;
	uint32_t unknown7;
	uint32_t unknown8;
	uint32_t unknown9;
} titania_access_state;

typedef struct titania_data {
	titania_hid hid;
	titania_bt bt;
	titania_time time;
	titania_buttons buttons;
	titania_trigger triggers[2];
	titania_vector2 sticks[2];
	titania_touchpad touch[2];
	titania_sensors sensors;
	titania_battery battery;
	titania_device_state device;
	titania_edge_state edge_device;
	titania_access_state access_device;
	uint64_t state_id;
} titania_data;

typedef struct titania_access_led_update {
	bool enable_profile_led;
	bool enable_center_led;
	bool enable_second_center_led;
	bool update_profile;
	uint8_t profile_led;
} titania_access_led_update;

typedef struct titania_led_update {
	titania_led_index led;
	titania_vector3 color;
	titania_access_led_update access;
} titania_led_update;

typedef struct titania_effect_update_off {
	uint8_t reserved[0x60];
} titania_effect_update_off;

typedef titania_effect_update_off titania_effect_update_stop;
typedef titania_effect_update_off titania_effect_update_none;

typedef struct titania_effect_update_uniform {
	float position;
	float resistance;
} titania_effect_update_uniform;

typedef struct titania_effect_update_slope {
	titania_vector2 position;
	titania_vector2 resistance;
} titania_effect_update_slope;

typedef struct titania_effect_update_trigger {
	titania_vector2 position;
	float resistance;
} titania_effect_update_trigger;

typedef struct titania_effect_update_section {
	titania_vector2 position;
	float resistance;
} titania_effect_update_section;

// todo: document frequency and period.
// frequency is the raw Hz frequency of the vibration, 201 is a good value.
// period is the period in 0.1s steps, 1 is 100ms
// "stable" frequency table = 1..38, 39, 41, 42, 44, 46, 48, 51, 53, 56, 59, 63, 67, 72, 77, 84, 91, 101, 112, 126, 143, 167, 201, 251, 255
typedef struct titania_effect_update_vibrate {
	float position;
	float amplitude;
	int32_t frequency;
} titania_effect_update_vibrate;

typedef struct titania_effect_update_vibrate_slope {
	titania_vector2 position;
	titania_vector2 amplitude;
	int32_t frequency;
	int32_t period;
} titania_effect_update_vibrate_slope;

typedef struct titania_effect_update_multiple_sections {
	float resistance[TITANIA_TRIGGER_GRANULARITY];
} titania_effect_update_multiple_sections;

typedef struct titania_effect_update_multiple_vibrate {
	float amplitude[TITANIA_TRIGGER_GRANULARITY];
	int32_t frequency;
	int32_t period;
} titania_effect_update_multiple_vibrate;

typedef struct titania_effect_update_multiple_vibrate_sections {
	float resistance[TITANIA_TRIGGER_GRANULARITY];
	float amplitude[TITANIA_TRIGGER_GRANULARITY];
} titania_effect_update_multiple_vibrate_sections;

typedef struct titania_effect_update {
	titania_effect_mode mode;

	union titania_effect_update_effect {
		titania_effect_update_none none;
		titania_effect_update_off off;
		titania_effect_update_stop stop;
		titania_effect_update_uniform uniform;
		titania_effect_update_slope slope;
		titania_effect_update_trigger trigger;
		titania_effect_update_section section;
		titania_effect_update_vibrate vibrate;
		titania_effect_update_vibrate_slope vibrate_slope;
		titania_effect_update_multiple_sections multiple_sections;
		titania_effect_update_multiple_vibrate multiple_vibrate;
		titania_effect_update_multiple_vibrate_sections multiple_vibrate_sections;
	} effect;
} titania_effect_update;

typedef struct titania_audio_update {
	float jack_volume;
	float speaker_volume;
	float microphone_volume;
	titania_audio_mic mic_selection;
	titania_audio_mic mic_balance;
	titania_mic_led mic_led;
	bool disable_audio_jack;
	bool force_enable_speaker;
} titania_audio_update;

typedef struct titania_control_update {
	bool touch_powersave;
	bool sensor_powersave;
	bool rumble_powersave;
	bool speaker_powersave;
	bool mute_mic;
	bool mute_speaker;
	bool mute_jack;
	bool disable_rumble;
	bool disable_beamforming;
	bool enable_lowpass_filter;
	bool disable_led_brightness_control;
	bool disable_led_effect_control;
	bool disable_rumble_emulation;
	uint8_t gain;
	titania_level led_brightness;
	titania_led_effect led_effect;
	uint8_t reserved1;
	uint8_t reserved2;
	uint16_t reserved3;

	bool edge_disable_switching_profiles;
	bool edge_disable_led_indicators;
	bool edge_disable_vibration_indicators;
} titania_control_update;

#define titania_init() titania_init_checked(sizeof(titania_hid))

/**
 * @brief initialize the library, this is mandatory.
 * @param size: sizeof(titania_hid)
 */
TITANIA_EXPORT titania_result titania_init_checked(const size_t size);

/**
 * @brief scan and return all HIDs that this library supports.
 * @param hids: pointer to where HID data should be stored
 * @param hids_length: array size of hids
 */
TITANIA_EXPORT titania_result titania_get_hids(titania_query* hids, const size_t hids_length);

/**
 * @brief open a HID handle for processing
 * @param path: the path of the device to open
 * @param is_bluetooth: whether or not to consider this device a bluetooth device.
 * @param handle: pointer to the titania HID handle, this value will hold the titania_handle value when the function returns
 * @param use_calibration: whether or not to use calibration data for the gyroscope and accelerometer
 * @param blocking: whether or not to wait for data before reading, this is sometimes slower or faster.
 */
TITANIA_EXPORT titania_result titania_open(const titania_hid_path path, const bool is_bluetooth, titania_hid* handle, const bool use_calibration, const bool blocking);

/**
 * @brief poll controllers for input data
 * @param handle: pointer to an array of handles, values will be set to TITANIA_INVALID_HANDLE if they are invalid.
 * @param handle_count: number of handles to process
 * @param data: pointer to an array of data storage
 */
TITANIA_EXPORT titania_result titania_pull(titania_handle* handle, const size_t handle_count, titania_data* data);

/**
 * @brief push output data to controllers
 * @param handle: pointer to an array of handles, values will be set to TITANIA_INVALID_HANDLE if they are invalid.
 * @param handle_count: number of handles to process
 */
TITANIA_EXPORT titania_result titania_push(titania_handle* handle, const size_t handle_count);

/**
 * @brief update LED state of a controller
 * @param handle: the controller to update
 * @param data: led update data
 */
TITANIA_EXPORT titania_result titania_update_led(const titania_handle handle, const titania_led_update data);

/**
 * @brief update audio state of a controller
 * @param handle: the controller to update
 * @param data: audio update data
 */
TITANIA_EXPORT titania_result titania_update_audio(const titania_handle handle, const titania_audio_update data);

/**
 * @brief update control state flags of a controller
 * @note these are controller settings, some of these options persist through device restarts
 * @param handle: the controller to update
 * @param data: control update data
 */
TITANIA_EXPORT titania_result titania_update_control(const titania_handle handle, const titania_control_update data);

/**
 * @brief get control state flags of a controller (if we've sent them this session.)
 * @param handle: the controller to update
 * @param control: control update data
 */
TITANIA_EXPORT titania_result titania_get_control(const titania_handle handle, titania_control_update* control);

/**
 * @brief update effect state of a controller
 * @param handle: the controller to update
 * @param left_trigger: effect data for LT
 * @param right_trigger: effect data for RT
 * @param power_reduction: power reduction amount for trigger motors
 */
TITANIA_EXPORT titania_result titania_update_effect(const titania_handle handle, const titania_effect_update left_trigger, const titania_effect_update right_trigger, const float power_reduction);

/**
 * @brief update rumble state of a controller
 * @param handle: the controller to update
 * @param large_motor: amplitude for the large motor
 * @param small_motor: amplitude for the small motor
 * @param power_reduction: power reduction amount for haptic motors
 * @param emulate_legacy_behavior: instructs the dualsense to emulate how rumble motors used to work
 */
TITANIA_EXPORT titania_result titania_update_rumble(const titania_handle handle, const float large_motor, const float small_motor, const float power_reduction, const bool emulate_legacy_behavior);

/**
 * @brief pair a controller with a bluetooth adapter
 * @param handle: the controller to update
 * @param mac: mac address of the host bluetooth adapter
 * @param link_key: bluetooth link key
 */
TITANIA_EXPORT titania_result titania_bt_pair(const titania_handle handle, const titania_mac mac, const titania_link_key link_key);

/**
 * @brief tell a controller to connect with bluetooth
 * @param handle: the controller to update
 */
TITANIA_EXPORT titania_result titania_bt_connect(const titania_handle handle);

/**
 * @brief tell a controller to connect with usb
 * @param handle: the controller to update
 */
TITANIA_EXPORT titania_result titania_bt_disconnect(const titania_handle handle);

/**
 * @brief update a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to store the profile into
 * @param profile: the profile data to store
 */
TITANIA_EXPORT titania_result titania_update_edge_profile(const titania_handle handle, const titania_profile_id id, const titania_edge_profile profile);

/**
 * @brief update a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to store the profile into
 * @param profile: the profile data to store
 */
TITANIA_EXPORT titania_result titania_update_access_profile(const titania_handle handle, const titania_profile_id id, const titania_access_profile profile);

/**
 * @brief fetches all dualsense edge profiles
 * @param handle: the controller to query
 * @param profile_id: profile id to query
 * @param profile: the profile data
 */
TITANIA_EXPORT titania_result titania_query_edge_profile(const titania_handle handle, const titania_profile_id profile_id, titania_edge_profile* profile);

/**
 * @brief fetches all access profiles
 * @param handle: the controller to query
 * @param profile_id: profile id to query
 * @param profile: the profile data
 */
TITANIA_EXPORT titania_result titania_query_access_profile(const titania_handle handle, const titania_profile_id profile_id, titania_access_profile* profile);

/**
 * @brief reset a stick template to a specific template
 * @param stick: the stick to update
 * @param template_id: the stick template to apply
 * @param offset: range between -5 and 5 to offset the sticks
 */
TITANIA_EXPORT titania_result titania_helper_edge_stick_template(titania_edge_stick* stick, const titania_edge_stick_template template_id, int32_t offset);

/**
 * @brief delete a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to delete
 */
TITANIA_EXPORT titania_result titania_delete_edge_profile(const titania_handle handle, const titania_profile_id id);

/**
 * @brief delete a playstation access profile
 * @param handle: the controller to update
 * @param id: the profile id to delete
 */
TITANIA_EXPORT titania_result titania_delete_access_profile(const titania_handle handle, const titania_profile_id id);

/**
 * @brief close a controller device handle
 * @param handle: the controller to close
 */
TITANIA_EXPORT titania_result titania_close(const titania_handle handle);

/**
 * @brief cleans up library internals for exit
 */
TITANIA_EXPORT void titania_exit(void);

/**
 * @brief (debug) get the underlying hid device
 * @param handle: the device to query
 * @param hid: where to store the hid device pointer
 */
TITANIA_EXPORT titania_result titania_debug_get_hid(const titania_handle handle, intptr_t* hid);

/**
 * @brief (debug) get hid report ids
 * @param handle: the device to query
 * @param report_ids: where to store the hid report info
 */
TITANIA_EXPORT titania_result titania_debug_get_hid_report_ids(const titania_handle handle, titania_report_id report_ids[0xFF]);

/**
 * @brief (debug) get a merged edge profile
 * @param handle: the device to query
 * @param profile_id: profile to get
 * @param profile_data: profile data buffer
 */
TITANIA_EXPORT titania_result titania_debug_get_edge_profile(const titania_handle handle, const titania_profile_id profile_id, uint8_t profile_data[TITANIA_MERGED_REPORT_EDGE_SIZE]);

/**
 * @brief (debug) get a merged access profile
 * @param handle: the device to query
 * @param profile_id: profile to get
 * @param profile_data: profile data buffer
 */
TITANIA_EXPORT titania_result titania_debug_get_access_profile(const titania_handle handle, const titania_profile_id profile_id, uint8_t profile_data[TITANIA_MERGED_REPORT_ACCESS_SIZE]);

/**
 * @brief convert a dualsense edge profile to titania's representation
 * @param input: the input to convert
 * @param output: the profile to convert into
 */
TITANIA_EXPORT titania_result titania_convert_edge_profile_input(uint8_t input[TITANIA_MERGED_REPORT_EDGE_SIZE], titania_edge_profile* output);

/**
 * @brief convert an access profile to titania's representation
 * @param input: the input to convert
 * @param output: the profile to convert into
 */
TITANIA_EXPORT titania_result titania_convert_access_profile_input(uint8_t input[TITANIA_MERGED_REPORT_ACCESS_SIZE], titania_access_profile* output);

#ifdef __cplusplus
}
#endif
#endif
