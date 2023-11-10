#pragma once

#ifndef LIBRESENSE_H
#define LIBRESENSE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LIBRESENSE_INVALID_HANDLE (-1)

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
	ELIBRESENSE_BATTERY_DISCHARGING,
	ELIBRESENSE_BATTERY_CHARGING,
	ELIBRESENSE_BATTERY_FULL,
	ELIBRESENSE_BATTERY_MAX
} libresense_battery_state;

typedef enum {
	ELIBRESENSE_PROFILE_TRIANGLE,
	ELIBRESENSE_PROFILE_SQUARE,
	ELIBRESENSE_PROFILE_CROSS,
	ELIBRESENSE_PROFILE_CIRCLE,
	ELIBRESENSE_PROFILE_MAX
} libresense_edge_profile_id;

extern const char *libresense_error_msg[ELIBRESENSE_ERROR_MAX + 1];
extern const char *libresense_battery_state_msg[ELIBRESENSE_BATTERY_MAX + 1];
extern const char *libresense_edge_profile_id_msg[ELIBRESENSE_PROFILE_MAX + 1];
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
	uint8_t todo;
} libresense_firmware_info;

typedef struct {
	uint8_t todo;
} libresense_calibration_info;

typedef struct {
	wchar_t name[0x64];
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
	libresense_calibration_info calibration;
	libresense_edge_profile edge_profiles[4];
#ifdef LIBRESENSE_DEBUG
	libresense_hid_report_id report_ids[0xFF];
#endif
} libresense_hid;

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

/**
 * Initialize the library, this is mandatory.
 */
void
libresense_init(void);

/**
 * scan and return all HIDs that this library supports.
 * @param libresense_hid *hids: pointer to where HID data should be stored
 * @param size_t hids_length: array size of hids
 */
libresense_result
libresense_get_hids(libresense_hid *hids, const size_t hids_length);

/**
 * open a HID handle for processing
 * @param libresense_hid *handle: pointer to the libresense HID handle, this value will hold the libresense_handle value when the function returns
 */
libresense_result
libresense_open(libresense_hid *handle);

/**
 * poll controllers for input data
 * @param libresense_handle *handle: pointer to an array of handles, values will be set to LIBRESENSE_INVALID_HANDLE if they are invalid.
 * @param int handle_count: number of handles to process
 * @param libresense_data *data: pointer to an array of data storage
 */
libresense_result
libresense_poll(libresense_handle *handle, const size_t handle_count, libresense_data *data);

// todo: libresense_update_led, libresense_update_effect, libresense_update_audio

/*
 * update a dualsense edge profile 
 * @param const libresense_handle handle: the controller to update
 * @param const libresense_edge_profile_id id: the profile id to store the profile into
 * @param const libresense_edge_profile profile: the profile data to store
 */
libresense_result
libresense_update_profile(const libresense_handle handle, const libresense_edge_profile_id id, const libresense_edge_profile profile);

/*
 * delete a dualsense edge profile
 * @param const libresense_handle handle: the controller to update
 * @param const libresense_edge_profile_id id: the profile id to store the profile into
 * @param const libresense_edge_profile profile: the profile data to store
 */
libresense_result
libresense_delete_profile(const libresense_handle handle, const libresense_edge_profile_id id);

/*
 * close a controller device handle
 * @param const libresense_handle handle: the controller to close
 */
libresense_result
libresense_close(const libresense_handle handle);

/*
 * cleans up library internals for exit
 */
void
libresense_exit(void);

#ifdef LIBRESENSE_DEBUG
/*
 * debug: get a feature report
 * @param const libresense_handle handle: the device to query
 * @param int report_id: the report to fetch
 * @param uint8_t *buffer: where to store the buffer
 * @param size_t size: the size of the buffer
 */
size_t
libresense_debug_get_feature_report(const libresense_handle handle, const int report_id, uint8_t *buffer, const size_t size);
#endif
#endif
