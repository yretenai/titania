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
	int16_t x;
	int16_t y;
	int16_t z;
} libresense_vector3s;
static_assert(sizeof(libresense_vector3s) == 6, "size mismatch");

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
	uint64_t reserved;
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
