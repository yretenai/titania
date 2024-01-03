//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_H
#define LIBRESENSE_H
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
	LIBRESENSE_INVALID_HANDLE_DATA,
	LIBRESENSE_INVALID_HANDLE,
	LIBRESENSE_INVALID_DATA,
	LIBRESENSE_INVALID_PROFILE,
	LIBRESENSE_INVALID_ARGUMENT,
	LIBRESENSE_HIDAPI_FAIL,
	LIBRESENSE_ICU_FAIL,
	LIBRESENSE_OUT_OF_RANGE,
	LIBRESENSE_NOT_IMPLEMENTED,
	LIBRESENSE_NO_SLOTS,
	LIBRESENSE_NOT_EDGE,
	LIBRESENSE_NOT_ACCESS,
	LIBRESENSE_NOT_SUPPORTED,
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
	LIBRESENSE_PROFILE_MAX_VALID,

	LIBRESENSE_PROFILE_ALL,
	LIBRESENSE_PROFILE_MAX_META,

	LIBRESENSE_PROFILE_DEFAULT = LIBRESENSE_PROFILE_TRIANGLE,
	LIBRESENSE_PROFILE_1 = LIBRESENSE_PROFILE_SQUARE,
	LIBRESENSE_PROFILE_2 = LIBRESENSE_PROFILE_CROSS,
	LIBRESENSE_PROFILE_3 = LIBRESENSE_PROFILE_CIRCLE,
	LIBRESENSE_PROFILE_COUNT = 4,
} libresense_profile_id;

typedef enum {
	LIBRESENSE_LEVEL_HIGH = 0,
	LIBRESENSE_LEVEL_MEDIUM = 1,
	LIBRESENSE_LEVEL_LOW = 2,
	LIBRESENSE_LEVEL_OFF = 3
} libresense_level;

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

typedef enum {
	LIBRESENSE_BUTTON_ID_UP,
	LIBRESENSE_BUTTON_ID_LEFT,
	LIBRESENSE_BUTTON_ID_DOWN,
	LIBRESENSE_BUTTON_ID_RIGHT,
	LIBRESENSE_BUTTON_ID_CIRCLE,
	LIBRESENSE_BUTTON_ID_CROSS,
	LIBRESENSE_BUTTON_ID_SQUARE,
	LIBRESENSE_BUTTON_ID_TRIANGLE,
	LIBRESENSE_BUTTON_ID_R1,
	LIBRESENSE_BUTTON_ID_R2,
	LIBRESENSE_BUTTON_ID_R3,
	LIBRESENSE_BUTTON_ID_L1,
	LIBRESENSE_BUTTON_ID_L2,
	LIBRESENSE_BUTTON_ID_L3,
	LIBRESENSE_BUTTON_ID_LEFT_PADDLE,
	LIBRESENSE_BUTTON_ID_RIGHT_PADDLE,
	LIBRESENSE_BUTTON_ID_OPTION,
	LIBRESENSE_BUTTON_ID_TOUCH,
	LIBRESENSE_BUTTON_ID_MAX
} libresense_edge_button_id;

LIBRESENSE_EXPORT extern const char* const libresense_error_msg[LIBRESENSE_ERROR_MAX + 1];
LIBRESENSE_EXPORT extern const char* const libresense_battery_state_msg[LIBRESENSE_BATTERY_MAX + 1];
LIBRESENSE_EXPORT extern const char* const libresense_profile_id_msg[LIBRESENSE_PROFILE_MAX_META + 1];
LIBRESENSE_EXPORT extern const char* const libresense_profile_id_alt_msg[LIBRESENSE_PROFILE_MAX_META + 1];
LIBRESENSE_EXPORT extern const char* const libresense_level_msg[LIBRESENSE_LEVEL_LOW + 2];
LIBRESENSE_EXPORT extern const char* const libresense_trigger_effect_msg[LIBRESENSE_TRIGGER_EFFECT_MAX + 1];
LIBRESENSE_EXPORT extern const char* const libresense_edge_button_id_msg[LIBRESENSE_BUTTON_ID_MAX + 1];
LIBRESENSE_EXPORT extern const int libresense_max_controllers;

#define IS_LIBRESENSE_OKAY(result) (result == LIBRESENSE_OK)
#define IS_LIBRESENSE_BAD(result) (result != LIBRESENSE_OK)
typedef uint16_t libresense_wchar;

typedef signed int libresense_handle;
typedef wchar_t libresense_serial[0x100]; // Max HID Parameter length is 256 on USB, 512 on BT. HID serials are wide-chars, which are 2 bytes.

typedef char libresense_mac[0x12];
typedef char libresense_link_key[0x10];

typedef struct {
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
} libresense_vector2;

typedef struct {
	int x;
	int y;
} libresense_vector2i;

typedef struct {
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
} libresense_vector3;

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
	bool touch;
	bool mute;
	bool reserved;
	bool edge_f1;
	bool edge_f2;
	bool edge_left_paddle;
	bool edge_right_paddle;
	uint8_t edge_reserved;
	bool touchpad;
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
	// 0x0060 = Playstation Access (0xE5F, Leonardo)
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
	libresense_mac mac;
	libresense_mac paired_mac;
	uint32_t unknown;
} libresense_serial_info;

typedef enum {
	LIBRESENSE_EDGE_STICK_TEMPLATE_DEFAULT,
	LIBRESENSE_EDGE_STICK_TEMPLATE_QUICK,
	LIBRESENSE_EDGE_STICK_TEMPLATE_PRECISE,
	LIBRESENSE_EDGE_STICK_TEMPLATE_STEADY,
	LIBRESENSE_EDGE_STICK_TEMPLATE_DIGITAL,
	LIBRESENSE_EDGE_STICK_TEMPLATE_DYNAMIC,
	LIBRESENSE_EDGE_STICK_TEMPLATE_MAX
} libresense_edge_stick_template;

// NOTE: 1 and 2 may be valid, but it is untested!
// Also I don't know if this is actually interpolation!
// Only two templates that have this is Precise and Steady
typedef enum {
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_NONE = 0,
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_UNKNOWN1 = 1,
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_UNKNOWN2 = 2,
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_LINEAR = 3,
	LIBRESENSE_EDGE_INTERPOLATION_TYPE_SMOOTH = 4
} libresense_edge_interpolation_type;

typedef struct {
	libresense_edge_stick_template template_id;
	libresense_vector2 deadzone;
	libresense_vector2 curve_points[3];
	libresense_edge_interpolation_type interpolation_type;
	uint8_t unknown;
	bool disabled;
} libresense_edge_stick;

typedef struct {
	libresense_vector2 deadzone;
} libresense_edge_trigger;

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

	libresense_edge_button_id values[0x10];
} libresense_edge_button_remap;

typedef struct {
	bool valid;
	uint32_t version;
	char name[0x81];
	uint8_t id[0x10];
	libresense_edge_stick sticks[2];
	libresense_edge_trigger triggers[2];
	libresense_edge_button_remap buttons;
	libresense_level vibration;
	libresense_level trigger_effect;
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
	bool is_access;
	libresense_serial hid_serial;
	libresense_serial_info serial;
	libresense_firmware_info firmware;
	libresense_edge_profile edge_profiles[LIBRESENSE_PROFILE_COUNT];
	uint32_t checksum;
} libresense_hid;

typedef struct {
	uint8_t id;
	uint8_t type;
	uint32_t size;
} libresense_report_id;

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
	libresense_profile_id current_profile_id;

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
	uint8_t reserved;
} libresense_access_button;

typedef struct {
	libresense_vector2 pos;
	uint16_t unknown;
} libresense_access_stick;

typedef struct {
	libresense_access_button buttons;
	libresense_vector2 raw_stick;
	libresense_access_stick sticks[2];
	libresense_profile_id current_profile_id;
	uint32_t unknown_flags;
	uint32_t unknown0;
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t unknown4;
	uint32_t unknown5;
	uint32_t unknown6;
	uint32_t unknown7;
	uint32_t unknown8;
} libresense_access_state;

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
	libresense_access_state access_device;
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
	LIBRESENSE_LED_1 = 1,
	LIBRESENSE_LED_2 = 2,
	LIBRESENSE_LED_3 = 4,
	LIBRESENSE_LED_4 = 8,
	LIBRESENSE_LED_5 = 16,
	LIBRESENSE_LED_ACCESS_1 = 1,
	LIBRESENSE_LED_ACCESS_2 = 2,
	LIBRESENSE_LED_ACCESS_3 = 3,
	LIBRESENSE_LED_ACCESS_4 = 4,
	LIBRESENSE_LED_ALL = 31,
	LIBRESENSE_LED_FADE = 64,
	LIBRESENSE_LED_NO_UPDATE = 128
} libresense_led_index;

typedef struct {
	bool enable_profile_led;
	bool enable_center_led;
	bool enable_second_center_led;
	uint8_t profile_led;
} libresense_access_led_update;

typedef struct {
	libresense_led_index led;
	libresense_vector3 color;
	libresense_access_led_update access;
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
	LIBRESENSE_EFFECT_MUTIPLE_VIBRATE_SECTIONS
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
	LIBRESENSE_MIC_BOTH = 3
} libresense_audio_mic;

typedef enum {
	LIBRESENSE_MIC_LED_OFF = 0,
	LIBRESENSE_MIC_LED_ON = 1,
	LIBRESENSE_MIC_LED_FLASH = 2,
	LIBRESENSE_MIC_LED_FAST_FLASH = 3
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
	bool disable_beamforming;
	bool enable_lowpass_filter;
	bool disable_led_brightness_control;
	bool disable_led_effect_control;
	bool disable_rumble_emulation;
	uint8_t gain;
	libresense_level led_brightness;
	libresense_led_effect led_effect;
	uint8_t reserved1;
	uint8_t reserved2;
	uint16_t reserved3;

	bool edge_disable_switching_profiles;
	bool edge_disable_led_indicators;
	bool edge_disable_vibration_indicators;
} libresense_control_update;

#define libresense_init() libresense_init_checked(sizeof(libresense_hid))

/**
 * @brief initialize the library, this is mandatory.
 * @param size: sizeof(libresense_hid)
 */
LIBRESENSE_EXPORT libresense_result libresense_init_checked(const size_t size);

/**
 * @brief scan and return all HIDs that this library supports.
 * @param hids: pointer to where HID data should be stored
 * @param hids_length: array size of hids
 */
LIBRESENSE_EXPORT libresense_result libresense_get_hids(libresense_hid* hids, const size_t hids_length);

/**
 * @brief open a HID handle for processing
 * @param handle: pointer to the libresense HID handle, this value will hold the libresense_handle value when the function returns
 * @param use_calibration: whether or not to use calibration data for the gyroscope and accelerometer
 */
LIBRESENSE_EXPORT libresense_result libresense_open(libresense_hid* handle, bool use_calibration);

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
 * @note these are controller settings, some of these options persist through device restarts
 * @param handle: the controller to update
 * @param data: control update data
 */
LIBRESENSE_EXPORT libresense_result libresense_update_control(const libresense_handle handle, const libresense_control_update data);

/**
 * @brief get control state flags of a controller (if we've sent them this session.)
 * @param handle: the controller to update
 * @param control: control update data
 */
LIBRESENSE_EXPORT libresense_result libresense_get_control(const libresense_handle handle, libresense_control_update* control);

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
 * @brief pair a controller with a bluetooth adapter
 * @param handle: the controller to update
 * @param mac: mac address of the host bluetooth adapter
 * @param link_key: bluetooth link key
 */
LIBRESENSE_EXPORT libresense_result libresense_bt_pair(const libresense_handle handle, const libresense_mac mac, const libresense_link_key link_key);

/**
 * @brief tell a controller to connect with bluetooth
 * @param handle: the controller to update
 */
LIBRESENSE_EXPORT libresense_result libresense_bt_connect(const libresense_handle handle);

/**
 * @brief tell a controller to connect with usb
 * @param handle: the controller to update
 */
LIBRESENSE_EXPORT libresense_result libresense_bt_disconnect(const libresense_handle handle);

/**
 * @brief update a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to store the profile into
 * @param profile: the profile data to store
 */
LIBRESENSE_EXPORT libresense_result libresense_update_edge_profile(const libresense_handle handle, const libresense_profile_id id, const libresense_edge_profile profile);

/**
 * @brief reset a stick template to a specific template
 * @param stick: the stick to update
 * @param template_id: the stick template to apply
 * @param offset: range between -5 and 5 to offset the sticks
 */
LIBRESENSE_EXPORT libresense_result libresense_helper_edge_stick_template(libresense_edge_stick* stick, const libresense_edge_stick_template template_id, int32_t offset);

/**
 * @brief delete a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to delete
 */
LIBRESENSE_EXPORT libresense_result libresense_delete_edge_profile(const libresense_handle handle, const libresense_profile_id id);

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
 * @brief (debug) convert a merged dualsense profile to libresense's representation
 * @note merged dualsense profiles are made using the merge-edge-profile.py script
 * @param input: data to convert from
 * @param output: the profile to convert into
 */
LIBRESENSE_EXPORT libresense_result libresense_debug_convert_edge_profile(uint8_t input[174], libresense_edge_profile* output);

/**
 * @brief (debug) get the underlying hid device
 * @param handle: the device to query
 * @param hid: where to store the hid device pointer
 */
LIBRESENSE_EXPORT libresense_result libresense_debug_get_hid(const libresense_handle handle, intptr_t* hid);

/**
 * @brief (debug) get hid report ids
 * @param handle: the device to query
 * @param report_ids: where to store the hid report info
 */
LIBRESENSE_EXPORT libresense_result libresense_debug_get_hid_report_ids(const libresense_handle handle, libresense_report_id report_ids[0xFF]);
#ifdef __cplusplus
}
#endif
#endif
