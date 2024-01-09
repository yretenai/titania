//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIACTL_H
#define TITANIACTL_H

#include <json.h>

#include <titania.h>

#include <titania_config_internal.h>

#define TITANIACTL_CONTROLLER_COUNT (31)

#ifndef TITANIA_HAS_NULLPTR
#define nullptr ((void*) 0)
#endif

#define MAKE_TITANIA_ERROR(result) (TITANIACTL_TITANIA_ERROR | ((titaniactl_error) result << 16))

typedef struct titaniactl_context {
	int connected_controllers;
	titania_hid hids[TITANIACTL_CONTROLLER_COUNT];
	titania_handle handles[TITANIACTL_CONTROLLER_COUNT];
	int argc;
	const char** argv;
} titaniactl_context;

typedef enum titaniactl_error {
	TITANIACTL_OK_NO_JSON_INTERRUPTED = -2,
	TITANIACTL_OK_NO_JSON = -1,
	TITANIACTL_OK = TITANIA_OK,
	TITANIACTL_HID_ERROR,
	TITANIACTL_INTERRUPTED,
	TITANIACTL_NOT_IMPLEMENTED,
	TITANIACTL_INVALID_ARGUMENTS,
	TITANIACTL_INVALID_PAIR_ARGUMENTS,
	TITANIACTL_INVALID_MAC_ADDRESS,
	TITANIACTL_INVALID_LINK_KEY,
	TITANIACTL_INVALID_PROFILE,
	TITANIACTL_EMPTY_PROFILE,
	TITANIACTL_FILE_WRITE_ERROR,
	TITANIACTL_FILE_READ_ERROR,
	TITANIACTL_TITANIA_ERROR = 0xFFFF,
	TITANIACTL_MASK = 0xFFFF,
	TITANIACTL_ERROR_MAX
} titaniactl_error;

#define IS_TITANIACTL_OKAY(result) (result == TITANIACTL_OK)
#define IS_TITANIACTL_BAD(result) (result > TITANIACTL_OK)

typedef titaniactl_error (*titaniactl_callback_t)(titaniactl_context* context);

typedef struct titaniactl_mode {
	const char* const name;
	titaniactl_callback_t callback;
	titaniactl_callback_t json_callback;
	const char* const help;
	const char* const args;
} titaniactl_mode;

void titania_errorf(const titania_result result, const char* message);
void titaniactl_errorf(const char* error, const char* message);

titaniactl_error titaniactl_mode_list(titaniactl_context* context);
titaniactl_error titaniactl_mode_report(titaniactl_context* context);
titaniactl_error titaniactl_mode_report_loop(titaniactl_context* context);
titaniactl_error titaniactl_mode_dump(titaniactl_context* context);
titaniactl_error titaniactl_mode_test(titaniactl_context* context);
titaniactl_error titaniactl_mode_bench(titaniactl_context* context);
titaniactl_error titaniactl_mode_led(titaniactl_context* context);
titaniactl_error titaniactl_mode_bt_pair(titaniactl_context* context);
titaniactl_error titaniactl_mode_bt_connect(titaniactl_context* context);
titaniactl_error titaniactl_mode_bt_disconnect(titaniactl_context* context);
titaniactl_error titaniactl_mode_profile_funnel(titaniactl_context* context);

titaniactl_error titaniactl_mode_list_json(titaniactl_context* context);
titaniactl_error titaniactl_mode_report_json(titaniactl_context* context);
titaniactl_error titaniactl_mode_report_loop_json(titaniactl_context* context);

struct json* titaniactl_mode_edge_convert(const titania_edge_profile profile, const bool include_success);
titaniactl_error titaniactl_mode_edge_import(titania_profile_id profile, const struct json* data, titania_hid handle);
titaniactl_error titaniactl_mode_edge_export(titania_profile_id profile, const char* path, titania_hid handle);
titaniactl_error titaniactl_mode_edge_delete(titania_profile_id profile, titania_hid handle);

struct json* titaniactl_mode_access_convert(const titania_access_profile profile, const bool include_success);
titaniactl_error titaniactl_mode_access_import(titania_profile_id profile, const struct json* data, titania_hid handle);
titaniactl_error titaniactl_mode_access_export(titania_profile_id profile, const char* path, titania_hid handle);
titaniactl_error titaniactl_mode_access_delete(titania_profile_id profile, titania_hid handle);

static inline uint16_t titania_parse_octet_safe(const char ch) {
	uint8_t value = ch;
	if (value >= '0' && value <= '9') {
		return value - '0';
	}

	value |= 0x20;

	if (value >= 'a' && value <= 'f') {
		return 9 + (value & 0xF);
	}

	return 0x100;
}

static inline uint8_t titania_parse_octet(const char ch) {
	const uint16_t value = titania_parse_octet_safe(ch);
	if (value > 0xFF) {
		return 0;
	}
	return value;
}

static uint64_t xoroshiro_s[2];

// https://prng.di.unimi.it/splitmix64.c
static inline void xoroshiro_init(uint64_t seed) {
	uint64_t z = (seed += 0x9e3779b9'7f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d'1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb'133111eb;
	xoroshiro_s[0] = z ^ (z >> 31);
	z = (xoroshiro_s[0] += 0x9e3779b9'7f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d'1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb'133111eb;
	xoroshiro_s[1] = z ^ (z >> 31);
}

// https://prng.di.unimi.it/xoroshiro128plusplus.c
static inline uint64_t rotl(const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

static inline uint64_t xoroshiro_next(void) {
	const uint64_t s0 = xoroshiro_s[0];
	uint64_t s1 = xoroshiro_s[1];
	const uint64_t result = rotl(s0 + s1, 17) + s0;

	s1 ^= s0;
	xoroshiro_s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	xoroshiro_s[1] = rotl(s1, 28); // c

	return result;
}

#include "titaniaprint.h"

extern bool should_stop;
extern bool is_json;
extern bool preserve_data;

#ifdef _WIN32
#include <time.h>
void nanosleep(const struct timespec* tspec, void* nullvoid);
typedef uint64_t useconds_t;
#endif

#endif
