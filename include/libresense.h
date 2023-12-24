#pragma once

#ifndef LIBRESENSE_H
#define LIBRESENSE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LIBRESENSE_INVALID_HANDLE_ID (-1)

#define LIBRESENSE_LEFT (0)
#define LIBRESENSE_RIGHT (1)
#define LIBRESENSE_PRIMARY (0)
#define LIBRESENSE_SECONDARY (1)
#define LIBRESENSE_MIN_DELAY (125)
#define LIBRESENSE_POLLING_RATE_BT (1)
#define LIBRESENSE_POLLING_RATE_USB (4)
#define LIBRESENSE_TRIGGER_GRANULARITY (10)

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
	LIBRESENSE_PROFILE_TRIANGLE,
	LIBRESENSE_PROFILE_SQUARE,
	LIBRESENSE_PROFILE_CROSS,
	LIBRESENSE_PROFILE_CIRCLE,
	LIBRESENSE_PROFILE_MAX
} libresense_edge_profile_id;

extern const char *libresense_error_msg[LIBRESENSE_ERROR_MAX + 1];
extern const char *libresense_battery_state_msg[LIBRESENSE_BATTERY_MAX + 1];
extern const char *libresense_edge_profile_id_msg[LIBRESENSE_PROFILE_MAX + 1];
extern const int libresense_max_controllers;

#define IS_LIBRESENSE_OKAY(result) (result == LIBRESENSE_OK)
#define IS_LIBRESENSE_BAD(result) (result != LIBRESENSE_OK)
typedef wchar_t libresense_wchar;

typedef signed int libresense_handle;
typedef libresense_wchar libresense_serial[0x100]; // Max HID Parameter length is 256 on USB, 512 on BT. HID serials are wide-chars, which are 2 bytes.

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
	int16_t x;
	int16_t y;
	int16_t z;
} libresense_vector3s;

static_assert(sizeof(libresense_vector3s) == 6, "libresense_vector3s is not 6 bytes");

typedef struct {
	int16_t max;
	int16_t min;
} libresense_minmax;

typedef struct {
	bool dpad_up : 1;
	bool dpad_right : 1;
	bool dpad_down : 1;
	bool dpad_left : 1;
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
	bool edge_unknown : 1;
	bool edge_f1 : 1;
	bool edge_f2 : 1;
	bool edge_lb : 1;
	bool edge_rb : 1;
} libresense_buttons;

typedef struct {
	float level;
	uint8_t id;
	uint8_t effect;
} libresense_trigger;

typedef struct {
	uint32_t id;
	bool active;
	libresense_vector2i coords;
} libresense_touchpad;

typedef struct {
	uint8_t sequence;
	uint8_t touch_sequence;
	uint16_t driver_sequence;
	uint32_t system;
	uint64_t sensor;
	uint64_t checksum;
} libresense_time;

typedef struct {
	libresense_vector3 accelerometer;
	libresense_vector3 gyro;
} libresense_sensors;

typedef struct {
	bool headphones : 1;
	bool headset : 1;
	bool muted : 1;
	bool cable_connected : 1;
} libresense_device_state;

typedef struct {
	bool stick_disconnected : 1;
	bool stick_error : 1;
	bool stick_calibrating : 1;
} libresense_edge_state;

typedef struct {
	float level;
	libresense_battery_state state;
	uint8_t battery_error;
} libresense_battery;

typedef struct {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint8_t revision;
} libresense_firmware_version;

typedef struct {
	uint8_t major;
	uint8_t minor;
} libresense_version;

#ifdef _MSC_VER
#pragma pack(push, 1)
typedef struct {
#else
typedef struct __attribute__((__packed__)) {
#endif
	char datetime[0x15];
	libresense_firmware_version device;
	libresense_firmware_version hardware;
	libresense_firmware_version software;
	uint16_t device_code;
	uint64_t unknown3;
	libresense_version update;
	uint8_t unknown4;
	libresense_firmware_version unknownVersion1;
	libresense_firmware_version unknownVersion2;
	uint16_t unknown5;
} libresense_firmware_info;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct {
	libresense_wchar name[0x64];
	uint8_t todo;
} libresense_edge_profile;

#ifdef LIBRESENSE_DEBUG
typedef struct {
	uint8_t id;
	size_t size;
} libresense_hid_report_id;
#endif

typedef struct {
	libresense_handle handle;
	uint16_t product_id;
	uint16_t vendor_id;
	bool is_bluetooth;
	libresense_serial serial;
	char mac[18];
	libresense_firmware_info firmware;
	libresense_edge_profile edge_profiles[4];
#ifdef LIBRESENSE_DEBUG
	libresense_hid_report_id report_ids[0xFF];
#endif
} libresense_hid;

typedef struct {
	libresense_hid hid;
	libresense_time time;
	libresense_buttons buttons;
	libresense_trigger triggers[2];
	libresense_vector2 sticks[2];
	libresense_touchpad touch[2];
	libresense_sensors sensors;
	libresense_battery battery;
	libresense_device_state device;
	libresense_edge_state edge_device;
	uint64_t state;
	uint64_t state_id;
} libresense_data;

typedef enum {
	LIBRESENSE_LED_MODE_OFF = 0,
	LIBRESENSE_LED_MODE_BRIGHTNESS = 1,
	LIBRESENSE_LED_MODE_UNINTERRUPTABLE = 2
} libresense_led_mode;

typedef enum {
	LIBRESENSE_LED_EFFECT_OFF = 0,
	LIBRESENSE_LED_EFFECT_RESET = 1,
	LIBRESENSE_LED_EFFECT_FADE_OUT = 2
} libresense_led_effect;

typedef enum {
	LIBRESENSE_LED_BRIGHTNESS_HIGH = 0,
	LIBRESENSE_LED_BRIGHTNESS_MEDIUM = 1,
	LIBRESENSE_LED_BRIGHTNESS_LOW = 2,
} libresense_led_brightness;

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
	LIBRESENSE_LED_ALL = 31,
	LIBRESENSE_LED_FADE = 64
} libresense_led_index;

typedef struct {
	libresense_led_mode mode;
	libresense_led_effect effect;
	libresense_led_brightness brightness;
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
	bool enable_mic;
	bool enable_audio;
} libresense_audio_update;

#define libresense_init() libresense_init_checked(sizeof(libresense_hid))

/**
 * @brief initialize the library, this is mandatory.
 * @param size: sizeof(libresense_hid)
 */
libresense_result
libresense_init_checked(const int size);

/**
 * @brief scan and return all HIDs that this library supports.
 * @param hids: pointer to where HID data should be stored
 * @param hids_length: array size of hids
 */
libresense_result
libresense_get_hids(libresense_hid *hids, const size_t hids_length);

/**
 * @brief open a HID handle for processing
 * @param handle: pointer to the libresense HID handle, this value will hold the libresense_handle value when the function returns
 */
libresense_result
libresense_open(libresense_hid *handle);

/**
 * @brief poll controllers for input data
 * @param handle: pointer to an array of handles, values will be set to LIBRESENSE_INVALID_HANDLE if they are invalid.
 * @param handle_count: number of handles to process
 * @param data: pointer to an array of data storage
 */
libresense_result
libresense_pull(libresense_handle *handle, const size_t handle_count, libresense_data *data);

/**
 * @brief push output data to controllers
 * @param handle: pointer to an array of handles, values will be set to LIBRESENSE_INVALID_HANDLE if they are invalid.
 * @param handle_count: number of handles to process
 */
libresense_result
libresense_push(const libresense_handle *handle, const size_t handle_count);

/**
 * @brief update LED state of a controller
 * @param handle: the controller to update
 * @param data: led update data
 */
libresense_result
libresense_update_led(const libresense_handle handle, const libresense_led_update data);

/**
 * @brief update audio state of a controller
 * @param handle: the controller to update
 * @param data: audio update data
 */
libresense_result
libresense_update_audio(const libresense_handle handle, const libresense_audio_update data);

/**
 * @brief update effect state of a controller
 * @param handle: the controller to update
 * @param left_trigger: effect data for LT
 * @param right_trigger: effect data for RT
 */
libresense_result
libresense_update_effect(const libresense_handle handle, const libresense_effect_update left_trigger, const libresense_effect_update right_trigger);

/**
 * @brief update rumble state of a controller
 * @param handle: the controller to update
 * @param large_motor: amplitude for the large motor
 * @param small_motor: amplitude for the small motor
 */
libresense_result
libresense_update_rumble(const libresense_handle handle, const float large_motor, const float small_motor);

/**
 * @brief update a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to store the profile into
 * @param profile: the profile data to store
 */
libresense_result
libresense_update_profile(const libresense_handle handle, const libresense_edge_profile_id id, const libresense_edge_profile profile);

/**
 * @brief delete a dualsense edge profile
 * @param handle: the controller to update
 * @param id: the profile id to delete
 */
libresense_result
libresense_delete_profile(const libresense_handle handle, const libresense_edge_profile_id id);

/**
 * @brief close a controller device handle
 * @param handle: the controller to close
 */
libresense_result
libresense_close(const libresense_handle handle);

/**
 * @brief cleans up library internals for exit
 */
void
libresense_exit(void);

#ifdef LIBRESENSE_DEBUG
/**
 * @brief (debug) get a feature report
 * @param handle: the device to query
 * @param report_id: the report to fetch
 * @param buffer: where to store the buffer
 * @param size: the size of the buffer
 */
size_t
libresense_debug_get_feature_report(const libresense_handle handle, const int report_id, uint8_t *buffer, const size_t size);
#endif
#endif
