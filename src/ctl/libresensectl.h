//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSECTL_H
#define LIBRESENSECTL_H

#include <libresense.h>

#define LIBRESENSECTL_CONTROLLER_COUNT (31)

#define errorf(fp, result, fmt) fprintf(fp, fmt ": %s\n", libresense_error_msg[result])

typedef struct {
	int connected_controllers;
	libresense_hid hids[LIBRESENSECTL_CONTROLLER_COUNT];
	libresense_handle handles[LIBRESENSECTL_CONTROLLER_COUNT];
	int filtered_controllers;
	libresense_serial filter[LIBRESENSECTL_CONTROLLER_COUNT];
	int argc;
	const char** argv;
} libresensectl_context;

typedef void (*libresensectl_callback_t)(const libresensectl_context context);

typedef struct {
	const char* const name;
	libresensectl_callback_t callback;
} libresensectl_mode;

void libresensectl_mode_stub(libresensectl_context context);
void libresensectl_mode_list(libresensectl_context context);
void libresensectl_mode_report(libresensectl_context context);
void libresensectl_mode_report_loop(libresensectl_context context);
void libresensectl_mode_dump(libresensectl_context context);
void libresensectl_mode_test(libresensectl_context context);
void libresensectl_mode_bench(libresensectl_context context);
void libresensectl_mode_led(libresensectl_context context);
void libresensectl_mode_bt_pair(libresensectl_context context);
void libresensectl_mode_bt_connect(libresensectl_context context);
void libresensectl_mode_bt_disconnect(libresensectl_context context);

#define LIBREPRINT_SEP() printf(",")
#define LIBREPRINT_STR(struc, field) printf(" " #field " = %s", struc.field)
#define LIBREPRINT_FIRMWARE_HW(struc, field) \
	printf(" " #field " { generation = %d, variation = %d, revision = %d, reserved = %d }", struc.field.generation, struc.field.variation, struc.field.revision, struc.field.reserved)
#define LIBREPRINT_UPDATE(struc, field) printf(" " #field " = %04x (%d.%d.%d)", struc.field.major, struc.field.major, struc.field.minor, struc.field.revision)
#define LIBREPRINT_FIRMWARE(struc, field) printf(" " #field " = %d.%d.%d", struc.field.major, struc.field.minor, struc.field.revision)
#define LIBREPRINT_U32(struc, field) printf(" " #field " = %u", struc.field)
#define LIBREPRINT_U64(struc, field) printf(" " #field " = %lu", struc.field)
#define LIBREPRINT_X16(struc, field) printf(" " #field " = 0x%04x", struc.field)
#define LIBREPRINT_FLOAT(struc, field) printf(" " #field " = %f", struc.field)
#define LIBREPRINT_PERCENT(struc, field) printf(" " #field " = %f%%", struc.field * 100.0f)
#define LIBREPRINT_PERCENT_LABEL(struc, field, name) printf(" " name " = %f%%", struc.field * 100.0f)
#define LIBREPRINT_ENUM(struc, field, strs, name) printf(" " name " = %s", strs[struc.field])
#define LIBREPRINT_TEST(struc, field) printf(" " #field " = %s", struc.field ? "Y" : "N")
#define LIBREPRINT_BUTTON_TEST(field) printf(" " #field " = %s", data.buttons.field ? "Y" : "N")
#define LIBREPRINT_EDGE_BUTTON_TEST(field) printf(" " #field " = %s", data.edge_device.raw_buttons.field ? "Y" : "N")
#define LIBREPRINT_PROFILE_BUTTON_TEST(field) printf(" " #field " = %s", profile.disabled_buttons.field ? "Y" : "N")

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <conio.h>
#include <windows.h>

// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void usleep(__int64 usec) {
	HANDLE timer;
	LARGE_INTEGER ft;
	ft.QuadPart = -(10 * usec);
	timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#else
#define __USE_XOPEN_EXTENDED
#include <unistd.h>
#define clrscr() printf("\033[1;1H\033[2J")
#endif

#endif
