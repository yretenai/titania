#pragma once

#ifndef LIBRESENSE_H
#define LIBRESENSE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#define LIBRESENSE_INVALID_HANDLE (-1)

#define LIBRESENSE_BUFFER_SIZE_USB 0x40
#define LIBRESENSE_BUFFER_SIZE_BT 0

typedef enum {
	ELIBRESENSE_OK = 0,
	ELIBRESENSE_NOT_INITIALIZED,
	ELIBRESENSE_INVALID_HANDLE,
	ELIBRESENSE_INVALID_DATA,
	ELIBRESENSE_OUT_OF_RANGE,
	ELIBRESENSE_NOT_IMPLEMENTED,
	ELIBRESENSE_NO_SLOTS,
	ELIBRESENSE_ERROR_MAX
} libresense_result;

typedef enum {
	ELIBRESENSE_BATTERY_UNKNOWN = 0,
	ELIBRESENSE_BATTERY_DISCHARGING = 1,
	ELIBRESENSE_BATTERY_CHARGING = 2,
	ELIBRESENSE_BATTERY_FULL = 3,
	ELIBRESENSE_BATTERY_MAX
} libresense_battery_state;

extern const char* libresense_error_msg[ELIBRESENSE_ERROR_MAX + 1];
extern const char* libresense_battery_state_msg[ELIBRESENSE_BATTERY_MAX + 1];
extern const int libresense_max_controllers;

typedef signed int libresense_handle;
typedef wchar_t libresense_serial[0x100]; // Max HID Parameter length is 256 on USB, 512 on BT. 
									      // HID serials are wide-chars, which are 2 bytes.

typedef struct {
	float x;
	float y;
} libresense_vector2;

typedef struct {
	float x;
	float y;
	float z;
} libresense_vector3;

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
	bool l2 : 1;
	bool l3 : 1;
	bool r1 : 1;
	bool r2 : 1;
	bool r3 : 1;
	bool share : 1;
	bool option : 1;
	bool ps : 1;
	bool mute : 1;
	bool touchpad : 1;
	bool touchpad_left : 1;
	bool touchpad_right : 1;
	bool fn1 : 1;
	bool fn2 : 1;
	bool left_paddle : 1;
	bool right_paddle : 1;
	bool mystery_button : 1;
} libresense_buttons;

typedef struct {
	float level;
	uint8_t adaptive_point;
} libresense_triggers;

typedef struct {
	libresense_vector2 left;
	libresense_vector2 right;
} libresense_sticks;

typedef struct {
	uint32_t id;
	bool active;
	libresense_vector2 coords;
} libresense_touchpad;

typedef struct {
	uint32_t system;
	uint32_t sensor;
	uint32_t audio;
	uint64_t checksum;
} libresense_time;

typedef struct {
	float speaker_volume;
	float mic_volume;
	float jack_volume;
} libresense_audio;

typedef struct {
	libresense_vector3 accelerometer;
	libresense_vector3 gyro;
} libresense_sensors;

typedef struct {
	uint8_t sensor_reserved;
	uint8_t adaptive_trigger_reserved;
	uint8_t adaptive_trigger_state;
	libresense_audio audio;
	uint8_t audio_reserved;
	uint8_t state_reserved;
	uint8_t state;
	uint8_t battery;
} libresense_internal;

typedef struct {
	bool rumble : 1;
	bool trigger_rumble : 1;
	bool led : 1;
	bool adaptive_trigger : 1;
	bool audio : 1;
	bool touch : 1;
} libresense_capabilities;

typedef struct {
	uint8_t todo;
} libresense_firmware_info;

typedef struct {
	uint8_t todo;
} libresense_calibration_info;

#ifdef LIBRESENSE_EDGE
typedef struct {
	wchar_t name[0x64];
	uint8_t todo;
} libresense_edge_profile;
#endif

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
	libresense_calibration_info calibration;
#ifdef LIBRESENSE_EDGE
	libresense_edge_profile edge_profiles[4];
#endif
#ifdef LIBRESENSE_DEBUG
	libresense_hid_report_id report_ids[0xFF];
#endif
} libresense_hid;

typedef union {
	libresense_handle handle;
	libresense_hid hid;
} libresense_hid_handle;
static_assert(offsetof(libresense_hid_handle, hid.handle) == 0, "hid.handle is not at zero");
static_assert(offsetof(libresense_hid_handle, handle) == 0, "hid handle is not at zero");

typedef struct {
	libresense_hid hid;
	libresense_time time;
	libresense_buttons buttons;
	libresense_triggers triggers[2];
	libresense_sticks sticks[2];
	libresense_touchpad touch[2];
	libresense_sensors sensors;
	libresense_internal internal;
} libresense_data;

void libresense_init(void);
libresense_result libresense_get_hids(libresense_hid* hids, size_t hids_length);
libresense_result libresense_open(libresense_hid* handle);
libresense_result libresense_poll(libresense_handle* handle, int handle_count, libresense_data* data);
libresense_result libresense_close(const libresense_handle handle);
void libresense_exit(void);
#ifdef LIBRESENSE_DEBUG
size_t libresense_debug_get_feature_report(const libresense_handle handle, int report_id, uint8_t* buffer, size_t size);
#endif
#ifdef LIBRESENSE_EDGE
libresense_result libresense_update_profile(const libresense_edge_profile_id id, const libresense_edge_profile profile);
libresense_result libresense_delete_profile(const libresense_edge_profile_id id);
#endif
#endif
