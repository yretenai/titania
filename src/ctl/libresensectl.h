//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSECTL_H
#define LIBRESENSECTL_H

#include <libresense.h>

#include <libresense_config_internal.h>

#define LIBRESENSECTL_CONTROLLER_COUNT (31)

#ifndef LIBRESENSE_HAS_NULLPTR
#define nullptr ((void*) 0)
#endif

#define errorf(fp, result, fmt) fprintf(fp, fmt ": %s\n", libresense_error_msg[result])

typedef struct {
	int connected_controllers;
	libresense_hid hids[LIBRESENSECTL_CONTROLLER_COUNT];
	libresense_handle handles[LIBRESENSECTL_CONTROLLER_COUNT];
	int argc;
	const char** argv;
} libresensectl_context;

typedef enum {
	LIBRESENSECTL_OK = LIBRESENSE_OK,
	LIBRESENSECTL_HID_ERROR,
	LIBRESENSECTL_INTERRUPTED,
	LIBRESENSECTL_NOT_IMPLEMENTED,
	LIBRESENSECTL_INVALID_ARGUMENTS,
	LIBRESENSECTL_ERROR_MAX
} libresensectl_error;

#define IS_LIBRESENSECTL_OKAY(result) (result == LIBRESENSECTL_OK)
#define IS_LIBRESENSECTL_BAD(result) (result != LIBRESENSECTL_OK)

typedef libresensectl_error (*libresensectl_callback_t)(libresensectl_context* context);

typedef struct {
	const char* const name;
	libresensectl_callback_t callback;
} libresensectl_mode;

libresensectl_error libresensectl_mode_list(libresensectl_context* context);
libresensectl_error libresensectl_mode_report(libresensectl_context* context);
libresensectl_error libresensectl_mode_report_loop(libresensectl_context* context);
libresensectl_error libresensectl_mode_dump(libresensectl_context* context);
libresensectl_error libresensectl_mode_test(libresensectl_context* context);
libresensectl_error libresensectl_mode_bench(libresensectl_context* context);
libresensectl_error libresensectl_mode_led(libresensectl_context* context);
libresensectl_error libresensectl_mode_bt_pair(libresensectl_context* context);
libresensectl_error libresensectl_mode_bt_connect(libresensectl_context* context);
libresensectl_error libresensectl_mode_bt_disconnect(libresensectl_context* context);
libresensectl_error libresensectl_mode_profile_funnel(libresensectl_context* context);

libresensectl_error libresensectl_mode_edge_import(libresense_profile_id profile, void* data, libresense_hid handle);
libresensectl_error libresensectl_mode_edge_export(libresense_profile_id profile, const char* path, libresense_hid handle);
libresensectl_error libresensectl_mode_edge_delete(libresense_profile_id profile, libresense_hid handle);

libresensectl_error libresensectl_mode_access_import(libresense_profile_id profile, void* data, libresense_hid handle);
libresensectl_error libresensectl_mode_access_export(libresense_profile_id profile, const char* path, libresense_hid handle);
libresensectl_error libresensectl_mode_access_delete(libresense_profile_id profile, libresense_hid handle);

#include "libreprint.h"

extern bool should_stop;

#ifdef _WIN32
typedef __int64 __useconds_t;
void usleep(__useconds_t usec);
#endif

#endif
