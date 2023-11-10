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

typedef struct PACKED {
	bool headphones : 1;
	bool headset : 1;
	bool muted : 1;
	bool cable_connected : 1;
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
	uint8_t sensor_reserved; // likely padding, but non-zero
	dualsense_touch touch[2];
	uint8_t adaptive_trigger_state; // random values? seems to go up every time the trigger info chages (including rumble)
	uint8_t adaptive_triggers[2];
	uint8_t adaptive_trigger_reserved; // likely padding, zero
	uint8_t speaker_volume;			   // assumption, zero
	uint8_t jack_volume;			   // assumption, zero
	uint8_t mic_volume;				   // assumption, zero
	uint8_t audio_reserved;			   // likely padding, zero
	uint32_t audio_time;			   // assumption, but definitely time
	dualsense_battery_state battery;
	dualsense_device_state state;
	uint8_t state_reserved; // likely padding, usually non-zero
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
	bool jack2 : 1; // always set when jack is set. maybe related to headset vs headphones?
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
	uint8_t vol; // todo
} dualsense_audio_output;

typedef struct PACKED {
	dualsense_mutator_flags flags;
	// todo
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
libresense_convert_input(const dualsense_input_msg input, libresense_data *data);

libresense_result
libresense_convert_edge_profile_input(void); // todo

libresense_result
libresense_convert_edge_profile_output(void); // todo

size_t
libresense_get_feature_report(hid_device *handle, int report_id, uint8_t *buffer, size_t size, bool preserve);

size_t
libresense_send_feature_report(hid_device *handle, int report_id, uint8_t *buffer, size_t size, bool preserve);
