//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_H
#define LIBRESENSE_H

#include <wchar.h>
#if __STDC_VERSION__ < 202000L
#include <stdbool.h>
#endif
#include <stdint.h>

#ifdef __WIN32
#ifdef LIBRESENSE_EXPORTING
#define LIBRESENSE_EXPORT __declspec(dllexport)
#else
#define LIBRESENSE_EXPORT __declspec(dllimport)
#endif
#else
#define LIBRESENSE_EXPORT __attribute__((visibility("default")))
#endif

#define LIBRESENSE_INVALID_HANDLE_ID (-1)

#define LIBRESENSE_LEFT (0)
#define LIBRESENSE_RIGHT (1)
#define LIBRESENSE_PRIMARY (0)
#define LIBRESENSE_SECONDARY (1)
#define LIBRESENSE_MIN_DELAY (125)
#define LIBRESENSE_POLLING_RATE_BT (1)
#define LIBRESENSE_POLLING_RATE_USB (4)
#define LIBRESENSE_TRIGGER_GRANULARITY (10)
#define LIBRESENSE_FIRMWARE_DATE_LEN (0x20)

typedef enum {
	LIBRESENSE_OK = 0,
	LIBRESENSE_NOT_INITIALIZED,
	LIBRESENSE_INVALID_LIBRARY,
	LIBRESENSE_INVALID_HANDLE,
	LIBRESENSE_INVALID_DATA,
	LIBRESENSE_HIDAPI_FAIL,
	LIBRESENSE_OUT_OF_RANGE,
	LIBRESENSE_NOT_IMPLEMENTED,
	LIBRESENSE_NO_SLOTS,
	LIBRESENSE_NOT_EDGE,
	LIBRESENSE_ERROR_MAX
} libresense_result;

typedef enum {
	LIBRESENSE_BATTERY_UNKNOWN = 0,
	LIBRESENSE_BATTERY_DISCHARGING,
	LIBRESENSE_BATTERY_CHARGING,
	LIBRESENSE_BATTERY_FULL,
	LIBRESENSE_BATTERY_MAX
} libresense_battery_state;

typedef enum {
	LIBRESENSE_PROFILE_NONE,
	LIBRESENSE_PROFILE_TRIANGLE,
	LIBRESENSE_PROFILE_SQUARE,
	LIBRESENSE_PROFILE_CROSS,
	LIBRESENSE_PROFILE_CIRCLE,
	LIBRESENSE_PROFILE_MAX,
	LIBRESENSE_PROFILE_MAX_MINUS_ONE = LIBRESENSE_PROFILE_MAX - 1
} libresense_edge_profile_id;

typedef enum {
	LIBRESENSE_LEVEL_HIGH = 0,
	LIBRESENSE_LEVEL_MEDIUM = 1,
	LIBRESENSE_LEVEL_LOW = 2
} libresense_level;

typedef enum {
	LIBRESENSE_EDGE_LEVEL_HIGH = 0,
	LIBRESENSE_EDGE_VIBRATION_LEVEL_MEDIUM = 2,
	LIBRESENSE_EDGE_VIBRATION_LEVEL_LOW = 3,
	LIBRESENSE_EDGE_EFFECT_LEVEL_MEDIUM = 6,
	LIBRESENSE_EDGE_EFFECT_LEVEL_LOW = 9,
	LIBRESENSE_EDGE_LEVEL_OFF = 255
} libresense_edge_power_level;

typedef enum {
	LIBRESENSE_TRIGGER_EFFECT_OFF = 0,
	LIBRESENSE_TRIGGER_EFFECT_UNIFORM = 1,
	LIBRESENSE_TRIGGER_EFFECT_TRIGGER = 2,
	LIBRESENSE_TRIGGER_EFFECT_VIBRATION = 3,
	LIBRESENSE_TRIGGER_EFFECT_MIXED = 4,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN5 = 5,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN6 = 6,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN7 = 7,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN8 = 8,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN9 = 9,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN10 = 10,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN11 = 11,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN12 = 12,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN13 = 13,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN14 = 14,
	LIBRESENSE_TRIGGER_EFFECT_UNKNOWN15 = 15,
	LIBRESENSE_TRIGGER_EFFECT_MAX
} libresense_trigger_effect_state;

LIBRESENSE_EXPORT extern const char* libresense_error_msg[LIBRESENSE_ERROR_MAX + 1];
LIBRESENSE_EXPORT extern const char* libresense_battery_state_msg[LIBRESENSE_BATTERY_MAX + 1];
LIBRESENSE_EXPORT extern const char* libresense_edge_profile_id_msg[LIBRESENSE_PROFILE_MAX + 1];
LIBRESENSE_EXPORT extern const char* libresense_level_msg[LIBRESENSE_LEVEL_LOW + 2];
LIBRESENSE_EXPORT extern const char* libresense_trigger_effect_msg[LIBRESENSE_TRIGGER_EFFECT_MAX + 1];
LIBRESENSE_EXPORT extern const int libresense_max_controllers;

#define IS_LIBRESENSE_OKAY(result) (result == LIBRESENSE_OK)
#define IS_LIBRESENSE_BAD(result) (result != LIBRESENSE_OK)
typedef uint16_t libresense_wchar;

typedef signed int libresense_handle;
typedef wchar_t libresense_serial[0x100]; // Max HID Parameter length is 256 on USB, 512 on BT. HID serials are wide-chars, which are 2 bytes.

typedef struct {
	float x;
	float y;
} libresense_vector2;

typedef struct {
	int x;
	int y;
} libresense_vector2i;

typedef struct {
	float x;
	float y;
	float z;
} libresense_vector3;

typedef struct {
	float min;
	float max;
} libresense_minmax;

typedef struct {
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
	bool share;
	bool option;
	bool l3;
	bool r3;
	bool playstation;
	bool touchpad;
	bool touch;
	bool mute;
	bool edge_f1;
	bool edge_f2;
	bool edge_left_paddle;
	bool edge_right_paddle;
	bool reserved;
	uint8_t edge_reserved;
} libresense_buttons;

typedef struct {
	float level;
	uint8_t id;
	uint8_t section;
	libresense_trigger_effect_state effect;
} libresense_trigger;

typedef struct {
	uint32_t id;
	bool active;
	libresense_vector2i pos;
} libresense_touchpad;

typedef struct {
	uint8_t sequence;
	uint8_t touch_sequence;
	uint32_t driver_sequence;
	uint32_t system;
	uint32_t battery; // does not exist in edge because the field is re-used
	uint32_t sensor;
	uint64_t checksum;
} libresense_time;

typedef struct {
	libresense_vector3 accelerometer;
	libresense_vector3 gyro;
	int32_t temperature;
} libresense_sensors;

typedef struct {
	bool headphones;
	bool headset;
	bool muted;
	bool usb_data;
	bool usb_power;
	bool external_mic;
	bool haptic_filter;
	uint16_t reserved;
} libresense_device_state;

typedef struct {
	float level;
	libresense_battery_state state;
} libresense_battery;

typedef struct {
	uint16_t major;
	uint16_t minor;
	uint16_t revision;
} libresense_firmware_version;

typedef struct {
	uint16_t reserved;
	uint16_t variation;
	uint16_t generation;
	uint16_t revision;
} libresense_firmware_hardware;

typedef struct {
	char datetime[LIBRESENSE_FIRMWARE_DATE_LEN];
	uint16_t type;
	// 0x0004 = DualSense (0xCE6, Bond)
	// 0x???? = DualSense Prototype (0xCE7, Aston)
	// 0x0044 = DualSense Edge (0xDF2, ?? pick a bond car lol i'm guessing vanquish or valhalla)
	uint16_t series;
	libresense_firmware_hardware hardware;
	libresense_firmware_version update;
	libresense_firmware_version firmware;
	libresense_firmware_version firmware2;
	libresense_firmware_version firmware3;
	libresense_firmware_version device;
	libresense_firmware_version device2;
	libresense_firmware_version device3;
	libresense_firmware_version mcu_firmware;
} libresense_firmware_info;

typedef struct {
	char mac[0x12];
	char paired_mac[0x12];
	uint32_t unknown;
} libresense_serial_info;

typedef enum {
	LIBRESENSE_EDGE_STICK_TEMPLATE_DEFAULT,
	LIBRESENSE_EDGE_STICK_TEMPLATE_QUICK,
	LIBRESENSE_EDGE_STICK_TEMPLATE_PRECISE,
	LIBRESENSE_EDGE_STICK_TEMPLATE_STEADY,
	LIBRESENSE_EDGE_STICK_TEMPLATE_DIGITAL,
	LIBRESENSE_EDGE_STICK_TEMPLATE_DYNAMIC,
} libresense_edge_stick_template;

typedef enum {
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_LINEAR = 0,
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_CUBIC = 3,
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_SMOOTHSTEP = 4,
} libresense_edge_interpolation_type;

typedef struct {
	libresense_edge_stick_template id;
	libresense_vector2 deadzone;
	libresense_vector2 curve_points[3];
	libresense_edge_interpolation_type interpolation_type;
	uint8_t unknown;
	bool disabled;
} libresense_edge_stick;

typedef struct {
	libresense_vector2 deadzone;
} libresense_edge_trigger;

typedef enum {
	LIBRESENSE_BUTTON_UP,
	LIBRESENSE_BUTTON_LEFT,
	LIBRESENSE_BUTTON_DOWN,
	LIBRESENSE_BUTTON_RIGHT,
	LIBRESENSE_BUTTON_CIRCLE,
	LIBRESENSE_BUTTON_CROSS,
	LIBRESENSE_BUTTON_SQUARE,
	LIBRESENSE_BUTTON_TRIANGLE,
	LIBRESENSE_BUTTON_R1,
	LIBRESENSE_BUTTON_R2,
	LIBRESENSE_BUTTON_R3,
	LIBRESENSE_BUTTON_L1,
	LIBRESENSE_BUTTON_L2,
	LIBRESENSE_BUTTON_L3,
	LIBRESENSE_BUTTON_LP,
	LIBRESENSE_BUTTON_RP,
	LIBRESENSE_BUTTON_OPT,
	LIBRESENSE_BUTTON_TOUCH,
} libresense_edge_button_id;

typedef union {
	struct {
		libresense_edge_button_id up;
		libresense_edge_button_id down;
		libresense_edge_button_id left;
		libresense_edge_button_id right;
		libresense_edge_button_id circle;
		libresense_edge_button_id cross;
		libresense_edge_button_id square;
		libresense_edge_button_id triangle;
		libresense_edge_button_id r1;
		libresense_edge_button_id r2;
		libresense_edge_button_id r3;
		libresense_edge_button_id l1;
		libresense_edge_button_id l2;
		libresense_edge_button_id l3;
		libresense_edge_button_id left_paddle;
		libresense_edge_button_id right_paddle;
	};

	libresense_edge_button_id buttons[0x10];
} libresense_edge_button_remap;

typedef struct {
	bool valid;
	uint32_t version;
	char name[0x81];
	uint8_t id[0x10];
	libresense_edge_stick sticks[2];
	libresense_edge_trigger triggers[2];
	libresense_edge_button_remap buttons;
	libresense_edge_power_level vibration;
	libresense_edge_power_level trigger_effect;
	bool sticks_swapped;
	bool trigger_deadzone_mirrored;
	libresense_buttons disabled_buttons;
	uint32_t unknown;
	uint64_t timestamp;
} libresense_edge_profile;

typedef struct {
	libresense_handle handle;
	uint16_t product_id;
	uint16_t vendor_id;
	bool is_bluetooth;
	bool is_edge;
	libresense_serial hid_serial;
	libresense_serial_info serial;
	libresense_firmware_info firmware;
	libresense_edge_profile edge_profiles[LIBRESENSE_PROFILE_MAX_MINUS_ONE];
#ifdef LIBRESENSE_DEBUG
	struct libresense_report_id {
		uint8_t id;
		uint8_t type;
		uint32_t size;
	} report_ids[0xFF];
#endif
} libresense_hid;

typedef struct {
	bool has_hid;
	bool unknown;
	bool unknown2;
	bool unknown3;
	uint8_t seq;
} libresense_bt;

typedef struct {
	libresense_buttons raw_buttons;

	struct {
		bool disconnected;
		bool errored;
		bool calibrating;
		bool unknown;
	} stick;

	libresense_level trigger_levels[2];
	libresense_edge_profile_id current_profile_id;

	struct {
		bool led;
		bool vibration;
		bool switching_disabled;
		bool unknown1;
		bool unknown2;
	} profile_indicator;

	libresense_level brightness;
	bool emulating_rumble;
	uint8_t unknown;
} libresense_edge_state;

typedef struct {
	libresense_hid hid;
	libresense_bt bt;
	libresense_time time;
	libresense_buttons buttons;
	libresense_trigger triggers[2];
	libresense_vector2 sticks[2];
	libresense_touchpad touch[2];
	libresense_sensors sensors;
	libresense_battery battery;
	libresense_device_state device;
	libresense_edge_state edge_device;
	uint64_t state_id;
} libresense_data;

typedef enum {
	LIBRESENSE_LED_EFFECT_OFF = 0,
	LIBRESENSE_LED_EFFECT_RESET = 1,
	LIBRESENSE_LED_EFFECT_FADE_OUT = 2
} libresense_led_effect;

typedef enum {
	LIBRESENSE_LED_NONE = 0,
	LIBRESENSE_LED_PLAYER_1 = 4,
	LIBRESENSE_LED_PLAYER_2 = 10,
	LIBRESENSE_LED_PLAYER_3 = 21,
	LIBRESENSE_LED_PLAYER_4 = 27,
	LIBRESENSE_LED_PLAYER_5 = 17,
	LIBRESENSE_LED_PLAYER_6 = 18,
	LIBRESENSE_LED_PLAYER_7 = 20,
	LIBRESENSE_LED_PLAYER_8 = 24,
	LIBRESENSE_LED_1 = 1,
	LIBRESENSE_LED_2 = 2,
	LIBRESENSE_LED_3 = 4,
	LIBRESENSE_LED_4 = 8,
	LIBRESENSE_LED_5 = 16,
	LIBRESENSE_LED_ALL = 31,
	LIBRESENSE_LED_FADE = 64
} libresense_led_index;

typedef struct {
	libresense_led_effect effect;
	libresense_level brightness;
	libresense_led_index led;
	libresense_vector3 color;
} libresense_led_update;

typedef enum {
	LIBRESENSE_EFFECT_NONE = -1,
	LIBRESENSE_EFFECT_OFF = 0,
	LIBRESENSE_EFFECT_STOP_VIBRATING,
	LIBRESENSE_EFFECT_UNIFORM,
	LIBRESENSE_EFFECT_SLOPE,
	LIBRESENSE_EFFECT_TRIGGER,
	LIBRESENSE_EFFECT_SECTION,
	LIBRESENSE_EFFECT_VIBRATE,
	LIBRESENSE_EFFECT_VIBRATE_SLOPE,
	LIBRESENSE_EFFECT_MUTIPLE_SECTIONS,
	LIBRESENSE_EFFECT_MUTIPLE_VIBRATE,
	LIBRESENSE_EFFECT_MUTIPLE_VIBRATE_SECTIONS,
} libresense_effect_mode;

typedef struct {
	uint8_t reserved[0x60];
} libresense_effect_update_off;

typedef libresense_effect_update_off libresense_effect_update_stop;
typedef libresense_effect_update_off libresense_effect_update_none;

typedef struct {
	float position;
	float resistance;
} libresense_effect_update_uniform;

typedef struct {
	libresense_vector2 position;
	libresense_vector2 resistance;
} libresense_effect_update_slope;

typedef struct {
	libresense_vector2 position;
	float resistance;
} libresense_effect_update_trigger;

typedef struct {
	libresense_vector2 position;
	float resistance;
} libresense_effect_update_section;

// todo: document frequency and period.
// frequency is the raw Hz frequency of the vibration, 201 is a good value.
// period is the period in 0.1s steps, 1 is 100ms
// "stable" frequency table = 1..38, 39, 41, 42, 44, 46, 48, 51, 53, 56, 59, 63, 67, 72, 77, 84, 91, 101, 112, 126, 143, 167, 201, 251, 255
typedef struct {
	float position;
	float amplitude;
	int32_t frequency;
} libresense_effect_update_vibrate;

typedef struct {
	libresense_vector2 position;
	libresense_vector2 amplitude;
	int32_t frequency;
	int32_t period;
} libresense_effect_update_vibrate_slope;

typedef struct {
	float resistance[LIBRESENSE_TRIGGER_GRANULARITY];
} libresense_effect_update_multiple_sections;

typedef struct {
	float amplitude[LIBRESENSE_TRIGGER_GRANULARITY];
	int32_t frequency;
	int32_t period;
} libresense_effect_update_multiple_vibrate;

typedef struct {
	float resistance[LIBRESENSE_TRIGGER_GRANULARITY];
	float amplitude[LIBRESENSE_TRIGGER_GRANULARITY];
} libresense_effect_update_multiple_vibrate_sections;

typedef struct {
	libresense_effect_mode mode;

	union {
		libresense_effect_update_none none;
		libresense_effect_update_off off;
		libresense_effect_update_stop stop;
		libresense_effect_update_uniform uniform;
		libresense_effect_update_slope slope;
		libresense_effect_update_trigger trigger;
		libresense_effect_update_section section;
		libresense_effect_update_vibrate vibrate;
		libresense_effect_update_vibrate_slope vibrate_slope;
		libresense_effect_update_multiple_sections multiple_sections;
		libresense_effect_update_multiple_vibrate multiple_vibrate;
		libresense_effect_update_multiple_vibrate_sections multiple_vibrate_sections;
	} effect;
} libresense_effect_update;

typedef enum {
	LIBRESENSE_MIC_AUTO = 0,
	LIBRESENSE_MIC_INTERNAL = 1,
	LIBRESENSE_MIC_EXTERNAL = 2,
	LIBRESENSE_MIC_BOTH = 3,
} libresense_audio_mic;

typedef enum {
	LIBRESENSE_MIC_LED_OFF = 0,
	LIBRESENSE_MIC_LED_ON = 1,
	LIBRESENSE_MIC_LED_FLASH = 2,
	LIBRESENSE_MIC_LED_FAST_FLASH = 3,
} libresense_mic_led;

typedef struct {
	float jack_volume;
	float speaker_volume;
	float microphone_volume;
	libresense_audio_mic mic_selection;
	libresense_audio_mic mic_balance;
	libresense_mic_led mic_led;
	bool disable_audio_jack;
	bool force_enable_speaker;
} libresense_audio_update;

typedef struct {
	bool touch_powersave;
	bool sensor_powersave;
	bool rumble_powersave;
	bool speaker_powersave;
	bool mute_mic;
	bool mute_speaker;
	bool mute_jack;
	bool disable_rumble;
	bool enable_beamforming;
	bool enable_lowpass_filter;
	bool led_brightness_control;
	bool led_color_control;
	uint8_t gain;
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t reserved3;

	bool has_edge_misc;
	bool has_edge_byte;
	uint8_t edge_unknown2;
	bool edge_disable_switching_profiles;
	bool edge_enable_led_indicators;
	bool edge_enable_vibration_indicators;
} libresense_control_update;

#define libresense_init() libresense_init_checked(sizeof(libresense_hid))

/**
 * @brief initialize the library, this is mandatory.
 * @param size: sizeof(libresense_hid)
 */
LIBRESENSE_EXPORT libresense_result libresense_init_checked(const int size);

/**
 * @brief scan and return all HIDs that this library supports.
 * @param hids: pointer to where HID data should be stored
 * @param hids_length: array size of hids
 */
LIBRESENSE_EXPORT libresense_result libresense_get_hids(libresense_hid* hids, const size_t hids_length);

/**
 * @brief open a HID handle for processing
 * @param handle: pointer to the libresense HID handle, this value will hold the libresense_handle value when the function returns
 */
LIBRESENSE_EXPORT libresense_result libresense_open(libresense_hid* handle);

/**
 * @brief poll controllers for input data
 * @param handle: pointer to an array of handles, values will be set to LIBRESENSE_INVALID_HANDLE if they are invalid.
 * @param handle_count: number of handles to process
 * @param data: pointer to an array of data storage
 */
LIBRESENSE_EXPORT libresense_result libresense_pull(libresense_handle* handle, const size_t handle_count, libresense_data* data);

/**
 * @brief push output data to controllers
 * @param handle: pointer to an array of handles, values will be set to LIBRESENSE_INVALID_HANDLE if they are invalid.
 * @param handle_count: number of handles to process
 */
LIBRESENSE_EXPORT libresense_result libresense_push(libresense_handle* handle, const size_t handle_count);

/**
 * @brief update LED state of a controller
 * @param handle: the controller to update
 * @param data: led update data
 */
LIBRESENSE_EXPORT libresense_result libresense_update_led(const libresense_handle handle, const libresense_led_update data);

/**
 * @brief update audio state of a controller
 * @param handle: the controller to update
 * @param data: audio update data
 */
LIBRESENSE_EXPORT libresense_result libresense_update_audio(const libresense_handle handle, const libresense_audio_update data);

/**
 * @brief update control state flags of a controller
 * @param handle: the controller to update
 * @param data: control update data
 */
LIBRESENSE_EXPORT libresense_result libresense_update_control(const libresense_handle handle, const libresense_control_update data);

/**
 * @brief update effect state of a controller
 * @param handle: the controller to update
 * @param left_trigger: effect data for LT
 * @param right_trigger: effect data for RT
 * @param power_reduction: power reduction amount for trigger motors
 */
LIBRESENSE_EXPORT libresense_result libresense_update_effect(const libresense_handle handle, const libresense_effect_update left_trigger, const libresense_effect_update right_trigger,
															 const float power_reduction);

/**
 * @brief update rumble state of a controller
 * @param handle: the controller to update
 * @param large_motor: amplitude for the large motor
 * @param small_motor: amplitude for the small motor
 * @param power_reduction: power reduction amount for haptic motors
 * @param emulate_legacy_behavior: instructs the dualsense to emulate how rumble motors used to work
 */
LIBRESENSE_EXPORT libresense_result libresense_update_rumble(const libresense_handle handle, const float large_motor, const float small_motor, const float power_reduction,
															 const bool emulate_legacy_behavior);

/**
 * @brief update a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to store the profile into
 * @param profile: the profile data to store
 */
LIBRESENSE_EXPORT libresense_result libresense_update_profile(const libresense_handle handle, const libresense_edge_profile_id id, const libresense_edge_profile profile);

/**
 * @brief delete a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to delete
 */
LIBRESENSE_EXPORT libresense_result libresense_delete_profile(const libresense_handle handle, const libresense_edge_profile_id id);

/**
 * @brief close a controller device handle
 * @param handle: the controller to close
 */
LIBRESENSE_EXPORT libresense_result libresense_close(const libresense_handle handle);

/**
 * @brief cleans up library internals for exit
 */
LIBRESENSE_EXPORT void libresense_exit(void);

/**
 * @brief (debug) get a feature report
 * @param handle: the device to query
 * @param report_id: the report to fetch
 * @param buffer: where to store the buffer
 * @param size: the size of the buffer
 */
LIBRESENSE_EXPORT size_t libresense_debug_get_feature_report(const libresense_handle handle, const int report_id, uint8_t* buffer, const size_t size);

/**
 * @brief (debug) convert a merged dualsense profile to libresense's representation
 * @note merged dualsense profiles are made using the merge-edge-profile.py script
 * @param input: data to convert from
 * @param output: the profile to convert into
 */
LIBRESENSE_EXPORT libresense_result libresense_debug_convert_edge_profile(uint8_t input[174], libresense_edge_profile* output);
#endif
