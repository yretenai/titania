//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_COMMON_H
#define LIBRESENSE_COMMON_H

#include <assert.h>
#include <stdint.h>

#include <libresense_config_internal.h>

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

#ifndef LIBRESENSE_HAS_NULLPTR
#define nullptr ((void*) 0)
#endif

#define DUALSENSE_LEFT (0)
#define DUALSENSE_RIGHT (1)
#define DUALSENSE_SMALL_MOTOR (0)
#define DUALSENSE_LARGE_MOTOR (1)
#define ADAPTIVE_TRIGGER_LEFT (1)
#define ADAPTIVE_TRIGGER_RIGHT (0)
#define DUALSENSE_ACCESS_LEFT (1)
#define DUALSENSE_ACCESS_RIGHT (0)
#define DUALSENSE_ACCESS_EXTENSION1 (0)
#define DUALSENSE_ACCESS_EXTENSION2 (1)
#define DUALSENSE_ACCESS_EXTENSION3 (2)
#define DUALSENSE_ACCESS_EXTENSION4 (3)
#define DUALSENSE_CRC_INPUT (0xA1)
#define DUALSENSE_CRC_OUTPUT (0xA2)
#define DUALSENSE_CRC_FEATURE (0xA3)
#define DUALSENSE_CRC_FEATURE_EDGE (0x53)
#define LIBRESENSE_CRC (0xDEE2CAFE)
#define DUALSENSE_TRIGGER_MAX (0xEE)
#define DUALSENSE_TRIGGER_AMPLITUDE_MAX (0x3F)
#define DUALSENSE_TRIGGER_VIBRATION_MAX (0xA8)
#define DUALSENSE_TRIGGER_STEP (7)
#define DUALSENSE_TRIGGER_SHIFT (3)
#define DUALSENSE_TRIGGER_FREQ_BITS (48)
#define DUALSENSE_TRIGGER_PERD_BITS (56)

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

#define DUALSENSE_FIRMWARE_VERSION_DATE_LEN 0xB
#define DUALSENSE_FIRMWARE_VERSION_TIME_LEN 0x8

#define NORM_CLAMP(value, max) ((value) >= 1.0f ? (max) : (value) <= 0.0f ? 0 : (uint8_t) ((value) * (max)))
#define NORM_CLAMP_UINT8(value) NORM_CLAMP(value, UINT8_MAX)
#define NORM_CLAMP_INT8(value) NORM_CLAMP_UINT8(((uint8_t) value))

#define DENORM_CLAMP(value, max) ((value) / ((float) max))
#define DENORM_CLAMP_UINT8(value) DENORM_CLAMP(value, UINT8_MAX)
#define DENORM_CLAMP_INT8(value) (DENORM_CLAMP(value, INT8_MAX + 1) / 2.0f)

// Check if the library is initialized
#define CHECK_INIT()     \
	if (!is_initialized) \
	return LIBRESENSE_NOT_INITIALIZED

// Check if a handle is a valid number.
#define CHECK_HANDLE(h)                                                             \
	if (h == LIBRESENSE_INVALID_HANDLE || h < 0 || h >= LIBRESENSE_MAX_CONTROLLERS) \
	return LIBRESENSE_INVALID_HANDLE

// Check if a handle is a valid number, and that it has been initialized.
#define CHECK_HANDLE_VALID(h)    \
	CHECK_HANDLE(h);             \
	if (state[h].hid == nullptr) \
	return LIBRESENSE_INVALID_HANDLE

#define HID_FAIL(s) (s == -1)
#define HID_PASS(s) (s != -1)

#define IS_EDGE(h) (h.vendor_id == 0x054C && h.product_id == 0x0DF2)
#define IS_ACCESS(h) (h.vendor_id == 0x054C && h.product_id == 0x0E5F)

#define CHECK_EDGE(h)                \
	if (!IS_EDGE(state[h].hid_info)) \
	return LIBRESENSE_NOT_EDGE

#define CHECK_ACCESS(h)                \
	if (!IS_ACCESS(state[h].hid_info)) \
	return LIBRESENSE_NOT_ACCESS

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
#ifdef _WIN32
	uint8_t x1 : 8;
	uint8_t x2 : 4;
	uint8_t y1 : 4;
	uint8_t y2 : 8;
#else
	uint16_t x : 12;
	uint16_t y : 12;
#endif
} dualsense_vector2;

static_assert(sizeof(dualsense_vector2) == 3, "dualsense_vector2 is not 3 bytes");

typedef struct PACKED {
	uint8_t x;
	uint8_t y;
} dualsense_vector2b;

static_assert(sizeof(dualsense_vector2b) == 2, "dualsense_vector2b is not 2 bytes");

typedef struct PACKED {
	int16_t x;
	int16_t y;
	int16_t z;
} dualsense_vector3s;

static_assert(sizeof(dualsense_vector3s) == 6, "dualsense_vector3s is not 6 bytes");

typedef struct PACKED {
	int16_t max;
	int16_t min;
} dualsense_minmax;

static_assert(sizeof(dualsense_minmax) == 4, "dualsense_minmax is not 4 bytes");

typedef struct {
	uint16_t vendor_id;
	uint16_t product_id;
} libresense_device_info;

typedef uint8_t dualsense_profile_uuid[0x10];

static_assert(sizeof(libresense_wchar) == 2, "libresense_wchar size is not 2");

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef PACKED

#endif // LIBRESENSE_COMMON_H
