//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIACTL_H
#define TITANIACTL_H

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

titaniactl_error titaniactl_mode_edge_import(titania_profile_id profile, titania_edge_profile data, titania_hid handle);
titaniactl_error titaniactl_mode_edge_export(titania_profile_id profile, const char* path, titania_hid handle);
titaniactl_error titaniactl_mode_edge_delete(titania_profile_id profile, titania_hid handle);

titaniactl_error titaniactl_mode_access_import(titania_profile_id profile, titania_access_profile data, titania_hid handle);
titaniactl_error titaniactl_mode_access_export(titania_profile_id profile, const char* path, titania_hid handle);
titaniactl_error titaniactl_mode_access_delete(titania_profile_id profile, titania_hid handle);

#include "titaniaprint.h"

extern bool should_stop;
extern bool is_json;

#ifdef _WIN32
#include <time.h>
void nanosleep(const struct timespec* tspec, void* nullvoid);
typedef uint64_t useconds_t;
#endif

#endif
