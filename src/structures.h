//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_STRUCTURES_H
#define LIBRESENSE_STRUCTURES_H

#include <assert.h>
#include <stdint.h>

#include <hidapi.h>

#include "../include/libresense.h"

static_assert(__STDC_VERSION__ >= 202000L, "a c2x compiler is required");

#include "access.h"
#include "common.h"
#include "edge.h"
#include "enums.h"

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

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
	bool playstation : 1;
	bool touch : 1;
	bool mute : 1;
	bool reserved : 1;
	bool edge_f1 : 1;
	bool edge_f2 : 1;
	bool edge_left_paddle : 1;
	bool edge_right_paddle : 1;
	uint8_t edge_reserved : 8;
} dualsense_button;

static_assert(sizeof(dualsense_button) == 4, "dualsense_button is not 4 bytes");

typedef struct PACKED {
	uint8_t value : 7;
	bool idle : 1;
} dualsense_touch_id;

static_assert(sizeof(dualsense_touch_id) == 1, "dualsense_touch_id is not 1 byte");

typedef struct PACKED {
	dualsense_touch_id id;
	dualsense_vector2 pos;
} dualsense_touch;

static_assert(sizeof(dualsense_touch) == 4, "dualsense_touch is not 4 bytes");

typedef struct PACKED {
	uint8_t level : 4;
	uint8_t state : 4;
} dualsense_battery_state;

static_assert(sizeof(dualsense_battery_state) == 1, "dualsense_battery_state is not 1 byte");

typedef struct PACKED {
	bool headphones : 1;
	bool headset : 1;
	bool muted : 1;
	bool usb_data : 1;
	bool usb_power : 1;
	uint8_t reserved1 : 3;
	bool external_mic : 1;
	bool haptic_filter : 1;
	uint8_t reserved2 : 6;
} dualsense_device_state_flags;

static_assert(sizeof(dualsense_device_state_flags) == 2, "dualsense_device_state_flags is not 2 bytes");

typedef struct PACKED {
	uint8_t right : 4;
	uint8_t left : 4;
} dualsense_trigger_state;

static_assert(sizeof(dualsense_trigger_state) == 1, "dualsense_device_state_flags is not 1 byte");

typedef struct PACKED {
	dualsense_trigger_state trigger;

	union {
		dualsense_device_state_edge edge;
		uint32_t battery_time; // why tf is this not reserved, sony please
	};

	dualsense_battery_state battery;
	dualsense_device_state_flags device;
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

static_assert(sizeof(dualsense_adaptive_trigger) == 1, "dualsense_adaptive_trigger is not 1 byte");

typedef struct PACKED {
	bool has_hid : 1;
	bool unknown : 1;
	bool unknown2 : 1;
	bool unknown3 : 1;
	uint8_t seq : 4;
} dualsense_input_msg_bt;

static_assert(sizeof(dualsense_input_msg_bt) == 1, "dualsense_input_msg_bt is not 1 byte");

typedef struct PACKED {
	union {
		dualsense_input_msg_bt bt;
		dualsense_report_id id;
	};

	dualsense_vector2b sticks[2];
	uint8_t triggers[2];
	uint8_t sequence;
	dualsense_button buttons;
	uint32_t firmware_time;

	union {
		struct PACKED {
			dualsense_sensors sensors;
			dualsense_touch touch[2];
			uint8_t touch_sequence;
			dualsense_adaptive_trigger adaptive_triggers[2];
			uint32_t state_id;
			dualsense_device_state state;
		};

		struct PACKED {
			dualsense_access_raw_button raw_button;
			dualsense_vector2b raw_stick;
			uint32_t unknown0; // 00 * 4
			uint32_t unknown1; // 00 * 4
			uint32_t unknown2; // ? 80 00 00 00
			uint32_t unknown3; // ? 80 00 00 00
			uint8_t unknown4; // 00
			dualsense_battery_state battery;
			uint16_t unknown5; // some bit flags? 06 00
			uint8_t profile_id; // 1..3
			uint8_t combined_device_flags; // more bit flags: 4 = E3, 64 = E4. Where are E1 and E2???? I think E1 and E2 are for combining Access controllers? Likely related to unknown 2 and 3.
			uint8_t unknown6; // 1
			dualsense_access_stick sticks[2];
			uint32_t unknown7; // 0
			uint8_t unknown8; // 0
		} access;
	};

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
	struct PACKED {
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

static_assert(sizeof(dualsense_effect_output) == 11, "dualsense_effect_output is not 11 byte");

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
	bool led_effect_control : 1;
	bool advanced_rumble_control : 1;
	uint8_t reserved2 : 4;
	bool has_edge_flag : 1;

	// misc flags
	bool enable_lowpass_filter : 1;
	uint16_t reserved3 : 14;

	bool edge_extension : 1;
} dualsense_control2;

static_assert(sizeof(dualsense_control2) == 4, "dualsense_control2 is not 4 bytes");

typedef struct PACKED {
	uint8_t jack;
	uint8_t speaker;
	uint8_t mic;
	dualsense_audio_flags flags;
	uint8_t mic_led_flags;
} dualsense_audio_output;

static_assert(sizeof(dualsense_audio_output) == 5, "dualsense_audio_output is not 5 bytes");

typedef struct PACKED {
	uint8_t rumble_power_reduction : 4;
	uint8_t trigger_power_reduction : 4;
} dualsense_motor_flags;

static_assert(sizeof(dualsense_motor_flags) == 1, "dualsense_motor_flags is not 1 byte");

typedef struct PACKED {
	bool unknown : 1;
	bool enable_hid : 1;
	bool unknown2 : 1;
	bool unknown3 : 1;
	uint8_t seq : 4;
} dualsense_report_output_bt;

typedef struct PACKED {
	union {
		dualsense_report_output_bt bt;
		dualsense_report_id id;
	};

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

typedef struct PACKED {
	uint8_t report_id;
	dualsense_vector3s gyro_bias;
	dualsense_minmax gyro[3];
	dualsense_minmax gyro_speed;
	dualsense_minmax accelerometer[3];
	uint16_t flags;
	uint32_t checksum;
} dualsense_calibration_info;

static_assert(sizeof(dualsense_calibration_info) == 41, "dualsense_calibration_info is not 41 byte");

typedef union {
	struct PACKED {
		uint8_t revision;
		uint8_t generation;
		uint8_t variation;
		uint8_t reserved;
	} hardware;

	struct PACKED {
		uint16_t major;
		uint8_t minor;
		uint8_t revision;
	} update;

	struct PACKED {
		uint8_t major;
		uint8_t minor;
		uint16_t revision;
	} firmware;
} dualsense_firmware_version;

static_assert(sizeof(dualsense_firmware_version) == 4, "dualsense_firmware_version is 4 bytes");

typedef struct PACKED {
	uint8_t report_id;
	char date[DUALSENSE_FIRMWARE_VERSION_DATE_LEN];
	char time[DUALSENSE_FIRMWARE_VERSION_TIME_LEN];
	uint16_t type;
	uint16_t series;
	dualsense_firmware_version hardware;
	dualsense_firmware_version firmware;
	dualsense_firmware_version device;
	dualsense_firmware_version device2;
	dualsense_firmware_version device3;
	dualsense_firmware_version update;
	dualsense_firmware_version firmware2;
	dualsense_firmware_version firmware3;
	dualsense_firmware_version mcu_firmware;
	uint32_t checksum;
} dualsense_firmware_info;

static_assert(DUALSENSE_FIRMWARE_VERSION_DATE_LEN + 1 + DUALSENSE_FIRMWARE_VERSION_TIME_LEN + 1 < LIBRESENSE_FIRMWARE_DATE_LEN, "date + space + time + null is >= libresense_firmware_info.version");
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
	hid_device* hid;
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

typedef struct PACKED {
	dualsense_report_id id;
	uint8_t pair_mac[6];
	uint8_t link_key[0x10];
	uint32_t checksum;
} dualsense_bt_pair_msg;

static_assert(sizeof(dualsense_bt_pair_msg) == 0x1b, "dualsense_bt_pair_msg is not 27 bytes");

typedef struct PACKED {
	dualsense_report_id id;
	dualsense_bt_command command;
	uint8_t reserved[0x2a];
	uint32_t checksum;
} dualsense_bt_command_msg;

static_assert(sizeof(dualsense_bt_command_msg) == 0x30, "dualsense_bt_command_msg is not 48 bytes");

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef PACKED

extern uint32_t crc_seed_input;
extern uint32_t crc_seed_output;
extern uint32_t crc_seed_feature;
extern uint32_t crc_seed_feature_edge;
extern uint32_t crc_seed_libresense;

/**
 * @brief convert dualsense input report to libresense's representation
 * @param hid_info: hid device info
 * @param input: the input to convert
 * @param data: the data to convert into
 * @param calibration: calibration data
 */
void libresense_convert_input(const libresense_hid hid_info, const dualsense_input_msg input, libresense_data* data, libresense_calibration_bit calibration[6]);

/**
 * @brief convert a dualsense edge profile to libresense's representation
 * @param input: the input to convert
 * @param output: the profile to convert into
 */
libresense_result libresense_convert_edge_profile_input(dualsense_edge_profile_blob input[3], libresense_edge_profile* output);

/**
 * @brief todo
 * convert a libresense profile to dualsense edge's representation
 * @param input: the input to convert
 * @param output: the profile to convert into
 */
libresense_result libresense_convert_edge_profile_output(libresense_edge_profile input, dualsense_edge_profile_blob output[3]); // todo

/**
 * @brief initializes checksum tables
 */
void libresense_init_checksum(void);

/**
 * @brief calculates a bluetooth checksum
 * @param state: existing state.
 * @param buffer: data to hash
 * @param size: sizeof(buffer)
 */
uint32_t libresense_calc_checksum(const uint32_t state, const uint8_t* buffer, const size_t size);

#endif
