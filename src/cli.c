//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <hidapi/hidapi.h>
#include <libresense.h>

#ifdef __WIN32__
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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"

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

#define libresense_errorf(fp, result, fmt) fprintf(fp, "[" LIBRESENSE_PROJECT_NAME "] " fmt ": %s\n", libresense_error_msg[result])
#define libresense_printf(fmt, ...) printf("[" LIBRESENSE_PROJECT_NAME "] " fmt "\n", __VA_ARGS__)
#define libresense_print(fmt) printf("[" LIBRESENSE_PROJECT_NAME "] " fmt "\n")

bool report_hid_trigger(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
	bool should_exit = false;
	while (true) {
		libresense_data data[libresense_max_controllers];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if (IS_LIBRESENSE_BAD(result)) {
			printf("invalid pull response");
			return true;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (data[i].buttons.option) {
				should_exit = true;
			}
			if (!data[i].buttons.option && should_exit) {
				printf("\n");
				return true;
			}
			if (i == 0) {
				printf("\r");
			}
			printf("left = { %06.2f%%, %1X, %1X }, right = { %06.2f%%, %1X, %1X } ",
				data[i].triggers[0].level * 100,
				data[i].triggers[0].id,
				data[i].triggers[0].effect,
				data[i].triggers[1].level * 100,
				data[i].triggers[1].id,
				data[i].triggers[1].effect);
		}
		usleep(delay);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = useconds;
		useconds -= delay;
		if (useconds > old) { // second failsafe.
			break;
		}
	}
	printf("\n");
	return should_exit;
}

bool report_hid_close(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
	bool should_exit = false;
	while (true) {
		libresense_data data[libresense_max_controllers];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if (IS_LIBRESENSE_BAD(result)) {
			printf("invalid pull response");
			return true;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (data[i].buttons.option) {
				should_exit = true;
			}
			if (!data[i].buttons.option && should_exit) {
				return true;
			}
		}

		usleep(delay);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = useconds;
		useconds -= delay;
		if (useconds > old) { // second failsafe.
			break;
		}
	}
	return should_exit;
}

void wait_until_options_clear(libresense_handle* handles, const size_t handle_count, __useconds_t timeout) {
	while (true) {
		libresense_data data[libresense_max_controllers];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if (IS_LIBRESENSE_BAD(result)) {
			return;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (!data[i].buttons.option) {
				return;
			}
		}
		usleep(16000);
		if (timeout < 16000 || timeout - 16000 == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = timeout;
		timeout -= 16000;
		if (timeout > old) { // second failsafe.
			break;
		}
	}
}

void print_profile(libresense_profile_id profile_id, libresense_edge_profile profile) {
	if (!profile.valid) {
		return;
	}

	printf("profile %s:\n", libresense_edge_profile_id_msg[profile_id]);

	// clang-format off
	LIBREPRINT_TEST(profile, valid); printf("\n");
	LIBREPRINT_STR(profile, name); printf("\n");
	// clang-format on

	printf(" id = ");
	for (int i = 0; i < 0x10; ++i) {
		printf("%02x", profile.id[i]);
	}

	// clang-format off
	printf("\n");
	LIBREPRINT_TEST(profile, sticks_swapped); printf("\n");
	LIBREPRINT_TEST(profile, trigger_deadzone_mirrored); printf("\n");
	LIBREPRINT_U32(profile, vibration); printf("\n");
	LIBREPRINT_U32(profile, trigger_effect); printf("\n");
	LIBREPRINT_U64(profile, timestamp); printf("\n");
	LIBREPRINT_U32(profile, unknown); printf("\n");

	printf(" left stick =\n\t");
	LIBREPRINT_TEST(profile.sticks[0], disabled); printf("\n\t");
	LIBREPRINT_U32(profile.sticks[0], template_id); printf("\n\t");
	LIBREPRINT_PERCENT_LABEL(profile.sticks[0], deadzone.x, "deadzone"); printf("\n\t");
	LIBREPRINT_U32(profile.sticks[0], interpolation_type); printf("\n\t");
	LIBREPRINT_PERCENT(profile.sticks[0], unknown); printf("\n\t");
	// clang-format on

	printf(" curve = [");
	for (int i = 0; i < 3; ++i) {
		printf("{ x = %f%%, y = %f%% }", profile.sticks[0].curve_points[i].x * 100, profile.sticks[0].curve_points[i].y * 100);
		if (i < 2) {
			printf(", ");
		}
	}
	printf("]\n");

	// clang-format off
	printf(" right stick =\n\t");
	LIBREPRINT_TEST(profile.sticks[1], disabled); printf("\n\t");
	LIBREPRINT_U32(profile.sticks[1], template_id); printf("\n\t");
	LIBREPRINT_PERCENT_LABEL(profile.sticks[1], deadzone.x, "deadzone"); printf("\n\t");
	LIBREPRINT_U32(profile.sticks[1], interpolation_type); printf("\n\t");
	LIBREPRINT_PERCENT(profile.sticks[1], unknown); printf("\n\t");
	// clang-format on

	printf(" curve = [");
	for (int i = 0; i < 3; ++i) {
		printf("{ x = %f%%, y = %f%% }", profile.sticks[1].curve_points[i].x * 100, profile.sticks[1].curve_points[i].y * 100);
		if (i < 2) {
			printf(", ");
		}
	}
	printf("]\n");

	printf(" trigger deadzones = [[ %f%%, %f%% ], [ %f%%, %f%% ]]\n", profile.triggers[0].deadzone.x, profile.triggers[0].deadzone.y, profile.triggers[1].deadzone.x, profile.triggers[1].deadzone.y);

	// clang-format off
	printf(" disabled =");
	LIBREPRINT_PROFILE_BUTTON_TEST(dpad_up); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(dpad_right); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(dpad_down); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(dpad_left); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(square); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(cross); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(circle); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(triangle); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(l1); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(r1); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(l2); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(r2); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(share); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(option); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(l3); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(r3); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(playstation); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(touchpad); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(touch); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(mute); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(edge_f1); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(edge_f2); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(edge_left_paddle); LIBREPRINT_SEP();
	LIBREPRINT_PROFILE_BUTTON_TEST(edge_right_paddle); printf("\n");
	// clang-format on

	printf(" remapped =");

	for (int i = 0; i < 0x10; ++i) {
		printf(" %s = %s", libresense_edge_button_id_msg[i], libresense_edge_button_id_msg[profile.buttons.values[i]]);
		if (i < 0xF) {
			printf(",");
		}
	}

	printf("\n");
}

// todo: move benchmark, profile, and test to libresensectl
int main(int argc, const char** argv) {
	libresense_printf("version %s", LIBRESENSE_PROJECT_VERSION);

	libresense_profile_id update_id;
	libresense_edge_profile update_profile;
	if (argc > 2) {
		if (strcmp(argv[1], "profile") == 0) {
			for (int i = 2; i < argc; ++i) {
				FILE* file = fopen(argv[i], "r+b");
				uint8_t buffer[174];
				if (file == nullptr) {
					continue;
				}
				size_t n = fread(buffer, 1, sizeof(buffer), file);
				fclose(file);
				if (n != sizeof(buffer)) {
					continue;
				}

				libresense_edge_profile profile;
				libresense_debug_convert_edge_profile(buffer, &profile);
				printf("%s: \n", argv[i]);
				print_profile(LIBRESENSE_PROFILE_NONE, profile);
			}

			return 0;
		}

		if (argc > 3 && strcmp(argv[1], "update") == 0) {
			switch (argv[2][0]) {
				case 'x': update_id = LIBRESENSE_PROFILE_CROSS; break;
				case 's': update_id = LIBRESENSE_PROFILE_SQUARE; break;
				case 'c': update_id = LIBRESENSE_PROFILE_CIRCLE; break;
				default: update_id = LIBRESENSE_PROFILE_NONE; break;
			}

			FILE* file = fopen(argv[3], "r+b");
			if (file != nullptr) {
				uint8_t buffer[174];
				size_t n = fread(buffer, 1, sizeof(buffer), file);
				fclose(file);
				if (n == sizeof(buffer)) {
					libresense_debug_convert_edge_profile(buffer, &update_profile);
				}
			}
		}
	}

	if (argc > 1 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "version") == 0)) {
		return 0;
	}

	libresense_result result = libresense_init();
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error initializing " LIBRESENSE_PROJECT_NAME);
		return result;
	}
	libresense_hid hid[libresense_max_controllers];
	libresense_handle handles[libresense_max_controllers];
	result = libresense_get_hids(hid, libresense_max_controllers);
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting hids");
		libresense_exit();
		return result;
	}
	size_t connected = 0;
	for (int hid_id = 0; hid_id < libresense_max_controllers; ++hid_id) {
		if (hid[hid_id].handle != LIBRESENSE_INVALID_HANDLE_ID) {
			result = libresense_open(&hid[hid_id], true);
			if (IS_LIBRESENSE_BAD(result)) {
				libresense_errorf(stderr, result, "error initializing hid");
				libresense_exit();
				return result;
			}
			libresense_printf("connected to hid %s", hid[hid_id].serial.mac);
			handles[connected++] = hid[hid_id].handle;
#ifdef LIBRESENSE_DEBUG
			char name[0x30] = { 0 };
			sprintf(name, "report_%s_%%d.bin", hid[hid_id].serial.mac);
			libresense_report_id report_ids[0xFF];
			hid_device* device;
			if (IS_LIBRESENSE_OKAY(libresense_debug_get_hid(hid[hid_id].handle, (intptr_t*) &device)) && IS_LIBRESENSE_OKAY(libresense_debug_get_hid_report_ids(hid[hid_id].handle, report_ids))) {
				for (int i = 0; i < 0xFF; i++) {
					uint8_t buffer[0x4096];
					if (report_ids[i].id == 0) {
						break;
					}

					int32_t hid_report_size = report_ids[i].size + 1;
					printf("report %d (%x): reported size is %d, type is %s",
						report_ids[i].id,
						report_ids[i].id,
						hid_report_size,
						report_ids[i].type == 0	  ? "INPUT"
						: report_ids[i].type == 1 ? "OUTPUT"
												  : "FEATURE");

					if (report_ids[i].type < 2) {
						printf("\n");
						continue;
					}

					printf(", actual size is ");
					buffer[0] = report_ids[i].id;
					size_t size = hid_get_feature_report(device, buffer, hid_report_size);
					if (size > 1 && size <= 0x4096) {
						printf("%ld\n", size);
						char report_name[0x30] = { 0 };
						sprintf(report_name, name, report_ids[i].id);
						FILE* file = fopen(report_name, "w+b");
						if (file != nullptr) {
							fwrite(buffer, 1, size, file);
							fclose(file);
						}
					} else {
						printf("??\n");
					}
				}
			}
#endif
		}
	}

	if (connected == 0) {
		fprintf(stderr, "[" LIBRESENSE_PROJECT_NAME "] no hids... connect a device\n");
		libresense_exit();
		return 1;
	}

	libresense_data datum[libresense_max_controllers];
	bool clr = argc > 1 && strcmp(argv[1], "report") == 0;
	bool bench = argc > 1 && strcmp(argv[1], "benchmark") == 0;
	while (true) {
		if (IS_LIBRESENSE_BAD(result)) {
			libresense_errorf(stderr, result, "error getting report");
			return result;
		}

		result = libresense_pull(handles, connected, datum);
		for (size_t i = 0; i < connected; ++i) {
			libresense_data data = datum[i];
			// clang-format off
			printf("hid {");
			LIBREPRINT_U32(data.hid, handle); LIBREPRINT_SEP();
			LIBREPRINT_X16(data.hid, product_id); LIBREPRINT_SEP();
			LIBREPRINT_X16(data.hid, vendor_id); LIBREPRINT_SEP();
			LIBREPRINT_STR(data.hid.serial, mac); LIBREPRINT_SEP();
			LIBREPRINT_STR(data.hid.serial, paired_mac); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.hid, is_bluetooth); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.hid, is_edge); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.hid, is_access);
			printf(" }\n");

			if(data.hid.is_bluetooth && !data.hid.is_access) {
				printf("bt {");
				LIBREPRINT_TEST(data.bt, has_hid); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.bt, unknown); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.bt, unknown2); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.bt, unknown3); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.bt, seq);
				printf(" }\n");
			}

			printf("firmware {");
			LIBREPRINT_X16(hid->firmware, type); LIBREPRINT_SEP();
			LIBREPRINT_X16(hid->firmware, series); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE_HW(hid->firmware, hardware); LIBREPRINT_SEP();
			LIBREPRINT_UPDATE(hid->firmware, update); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, firmware); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, firmware2); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, firmware3); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, device); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, device2); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, device3); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid->firmware, mcu_firmware);
			printf(" }\n");

			printf("time {");
			LIBREPRINT_U32(data.time, system); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, sensor); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, battery); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, sequence); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, touch_sequence); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, driver_sequence); LIBREPRINT_SEP();
			LIBREPRINT_U64(data.time, checksum);
			printf(" }\n");

			printf("buttons {");
			LIBREPRINT_BUTTON_TEST(dpad_up); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(dpad_right); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(dpad_down); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(dpad_left); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(square); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(cross); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(circle); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(triangle); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(l1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(r1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(l2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(r2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(share); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(option); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(l3); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(r3); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(playstation); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(touchpad); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(touch); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(mute); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_f1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_f2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_left_paddle); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_right_paddle); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.buttons, reserved); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.buttons, edge_reserved);
			printf(" }\n");

			printf("sticks { left = {");
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_LEFT], x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_LEFT], y);
			printf(" }, right = {");
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_RIGHT], x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_RIGHT], y);
			printf(" } }\n");

			if(data.hid.is_access) {
				continue;
			}

			printf("triggers { left = {");
			LIBREPRINT_PERCENT(data.triggers[LIBRESENSE_LEFT], level); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.triggers[LIBRESENSE_LEFT], id); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.triggers[LIBRESENSE_LEFT], section); LIBREPRINT_SEP();
			LIBREPRINT_ENUM(data.triggers[LIBRESENSE_LEFT], effect, libresense_trigger_effect_msg, "effect");
			printf(" }, right = {");
			LIBREPRINT_PERCENT(data.triggers[LIBRESENSE_RIGHT], level); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.triggers[LIBRESENSE_RIGHT], id); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.triggers[LIBRESENSE_RIGHT], section); LIBREPRINT_SEP();
			LIBREPRINT_ENUM(data.triggers[LIBRESENSE_RIGHT], effect, libresense_trigger_effect_msg, "effect");
			printf(" } }\n");

			printf("touch { primary = {");
			LIBREPRINT_TEST(data.touch[LIBRESENSE_PRIMARY], active); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.touch[LIBRESENSE_PRIMARY], id); LIBREPRINT_SEP();
			printf(" pos = {");
			LIBREPRINT_U32(data.touch[LIBRESENSE_PRIMARY].pos, x); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.touch[LIBRESENSE_PRIMARY].pos, y);
			printf(" } }, secondary = {");
			LIBREPRINT_TEST(data.touch[LIBRESENSE_SECONDARY], active); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.touch[LIBRESENSE_SECONDARY], id); LIBREPRINT_SEP();
			printf(" pos = {");
			LIBREPRINT_U32(data.touch[LIBRESENSE_SECONDARY].pos, x); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.touch[LIBRESENSE_SECONDARY].pos, y);
			printf(" } } }\n");


			printf("sensors {");
			LIBREPRINT_U32(data.sensors, temperature); LIBREPRINT_SEP();
			printf(" accelerometer = {");
			LIBREPRINT_FLOAT(data.sensors.accelerometer, x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sensors.accelerometer, y); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sensors.accelerometer, z);
			printf(" }, gyro = {");
			LIBREPRINT_FLOAT(data.sensors.gyro, x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sensors.gyro, y); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sensors.gyro, z);
			printf(" } }\n");

			printf("battery {");
			LIBREPRINT_PERCENT(data.battery, level); LIBREPRINT_SEP();
			LIBREPRINT_ENUM(data.battery, state, libresense_battery_state_msg, "state");
			printf(" }\n");

			printf("state {");
			LIBREPRINT_TEST(data.device, headphones); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.device, headset); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.device, muted); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.device, usb_data); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.device, usb_power); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.device, external_mic); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.device, haptic_filter); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.device, reserved);
			printf(" }\n");

			if (hid->is_edge) {
				printf("raw buttons {");
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_up); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_right); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_down); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_left); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(square); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(cross); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(circle); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(triangle); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(share); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(option); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(playstation); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(mute); LIBREPRINT_SEP();
				printf(" }\n");


				printf("edge state { stick {");
				LIBREPRINT_TEST(data.edge_device.stick, disconnected); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.stick, errored); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.stick, calibrating); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.stick, unknown);
				printf(" }, trigger {");
				LIBREPRINT_ENUM(data.edge_device, trigger_levels[LIBRESENSE_LEFT], libresense_level_msg, "left"); LIBREPRINT_SEP();
				LIBREPRINT_ENUM(data.edge_device, trigger_levels[LIBRESENSE_RIGHT], libresense_level_msg, "right");
				printf(" },");
				LIBREPRINT_ENUM(data.edge_device, current_profile_id, libresense_edge_profile_id_msg, "profile"); LIBREPRINT_SEP();
				printf(" indicator = {");
				LIBREPRINT_TEST(data.edge_device.profile_indicator, led); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, vibration); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, switching_disabled); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, unknown1); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, unknown2);
				printf(" },");
				LIBREPRINT_ENUM(data.edge_device, brightness, libresense_level_msg, "brightness"); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device, emulating_rumble); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.edge_device, unknown);
				printf(" }\n");
			}
			// clang-format on
		}

		if (!clr) {
			for (size_t i = 0; i < connected; ++i) {
				for (libresense_profile_id j = LIBRESENSE_PROFILE_1; j < LIBRESENSE_PROFILE_MAX; ++j) {
					print_profile(j, hid->edge_profiles[j]);
				}
			}

			break;
		}

		clrscr();
	}

	if (argc > 1 && !bench && update_id == LIBRESENSE_PROFILE_NONE) {
		goto shutdown;
	}

	if (bench) {
		libresense_print("testing latency, this will take 10 seconds");
		struct timespec max = { INT64_MIN, INT64_MIN };
		struct timespec min = { INT64_MAX, INT64_MAX };
		struct timespec ts1, ts2;
		libresense_data data;
		libresense_handle handle = handles[0];
		for (int i = 0; i < 10000; ++i) {
			timespec_get(&ts1, TIME_UTC);
			libresense_pull(&handle, connected, &data);
			timespec_get(&ts2, TIME_UTC);
			if (ts1.tv_nsec < ts2.tv_sec) {
				struct timespec delta = { ts2.tv_sec - ts1.tv_sec, ts2.tv_nsec - ts1.tv_nsec };
				if (delta.tv_sec < min.tv_sec || delta.tv_nsec < min.tv_nsec) {
					min = delta;
				}
				if (delta.tv_sec > max.tv_sec || delta.tv_nsec > max.tv_nsec) {
					max = delta;
				}
			}
			usleep(1000);
		}
		libresense_printf("min: %ld s %ld us, max: %ld s %ld us", min.tv_sec, min.tv_nsec, max.tv_sec, max.tv_nsec);

		goto shutdown;
	}

	if (update_id != LIBRESENSE_PROFILE_NONE) {
		for (size_t i = 0; i < connected; ++i) {
			if (!hid[i].is_edge) {
				continue;
			}

			libresense_update_profile(handles[i], update_id, update_profile);
		}

		goto shutdown;
	}

	libresense_print("press OPTIONS to skip test");

	{
		wait_until_options_clear(handles, connected, 250000);
		libresense_print("testing adaptive triggers");
		libresense_effect_update update = { 0 };

		update.mode = LIBRESENSE_EFFECT_UNIFORM;
		update.effect.uniform.position = 0.5;
		update.effect.uniform.resistance = 1.0;
		libresense_print("uniform");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_SECTION;
		update.effect.section.position.x = 0.25;
		update.effect.section.position.y = 0.75;
		update.effect.section.resistance = 1.0;
		libresense_print("section");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_MUTIPLE_SECTIONS;
		update.effect.multiple_sections.resistance[0] = 0;
		update.effect.multiple_sections.resistance[1] = 0;
		update.effect.multiple_sections.resistance[2] = 0.3f;
		update.effect.multiple_sections.resistance[3] = 0.3f;
		update.effect.multiple_sections.resistance[4] = 0.6f;
		update.effect.multiple_sections.resistance[5] = 0.6f;
		update.effect.multiple_sections.resistance[6] = 0.3f;
		update.effect.multiple_sections.resistance[7] = 0.3f;
		update.effect.multiple_sections.resistance[8] = 1.0f;
		update.effect.multiple_sections.resistance[9] = 1.0f;
		libresense_print("multiple section");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_TRIGGER;
		update.effect.trigger.position.x = 0.50f;
		update.effect.trigger.position.y = 1.00f;
		update.effect.trigger.resistance = 0.5f;
		libresense_print("trigger");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_SLOPE;
		update.effect.slope.position.x = 0.20;
		update.effect.slope.position.y = 1.00;
		update.effect.slope.resistance.x = 0.25f;
		update.effect.slope.resistance.y = 1.0f;
		libresense_print("slope");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_VIBRATE;
		update.effect.vibrate.position = 0.33;
		update.effect.vibrate.amplitude = 0.75;
		update.effect.vibrate.frequency = 201;
		libresense_print("vibrate");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_VIBRATE_SLOPE;
		update.effect.vibrate_slope.position.x = 0.20;
		update.effect.vibrate_slope.position.y = 1.00;
		update.effect.vibrate_slope.amplitude.x = 0.25f;
		update.effect.vibrate_slope.amplitude.y = 1.0f;
		update.effect.vibrate_slope.frequency = 201;
		update.effect.vibrate_slope.period = 4;
		libresense_print("vibrate slope");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_MUTIPLE_VIBRATE;
		update.effect.multiple_vibrate.amplitude[0] = 0;
		update.effect.multiple_vibrate.amplitude[1] = 0;
		update.effect.multiple_vibrate.amplitude[2] = 0.3f;
		update.effect.multiple_vibrate.amplitude[3] = 0.3f;
		update.effect.multiple_vibrate.amplitude[4] = 0.6f;
		update.effect.multiple_vibrate.amplitude[5] = 0.6f;
		update.effect.multiple_vibrate.amplitude[6] = 0.3f;
		update.effect.multiple_vibrate.amplitude[7] = 0.3f;
		update.effect.multiple_vibrate.amplitude[8] = 1.0f;
		update.effect.multiple_vibrate.amplitude[9] = 1.0f;
		update.effect.multiple_vibrate.frequency = 201;
		update.effect.multiple_vibrate.period = 4;
		libresense_print("multiple vibrate");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_MUTIPLE_VIBRATE_SECTIONS;
		update.effect.multiple_vibrate_sections.amplitude[0] = 0;
		update.effect.multiple_vibrate_sections.amplitude[1] = 0;
		update.effect.multiple_vibrate_sections.amplitude[2] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[3] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[4] = 0.6f;
		update.effect.multiple_vibrate_sections.amplitude[5] = 0.6f;
		update.effect.multiple_vibrate_sections.amplitude[6] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[7] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[8] = 1.0f;
		update.effect.multiple_vibrate_sections.amplitude[9] = 1.0f;
		update.effect.multiple_vibrate_sections.resistance[0] = 0;
		update.effect.multiple_vibrate_sections.resistance[1] = 0;
		update.effect.multiple_vibrate_sections.resistance[2] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[3] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[4] = 0.6f;
		update.effect.multiple_vibrate_sections.resistance[5] = 0.6f;
		update.effect.multiple_vibrate_sections.resistance[6] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[7] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[8] = 1.0f;
		update.effect.multiple_vibrate_sections.resistance[9] = 1.0f;
		libresense_print("multiple vibrate sections");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		if (report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

	reset_trigger:
		update.mode = LIBRESENSE_EFFECT_OFF;
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update, 0.0f);
		}
		libresense_push(handles, connected);
		usleep(100000);
	}

	{
		wait_until_options_clear(handles, connected, 250000);
		libresense_print("testing mic led...");
		libresense_audio_update update = { 0 };
		update.jack_volume = 1.0;
		update.speaker_volume = 1.0;
		update.microphone_volume = 1.0;
		update.mic_selection = LIBRESENSE_MIC_AUTO;
		update.mic_balance = LIBRESENSE_MIC_AUTO;
		update.disable_audio_jack = false;
		update.force_enable_speaker = false;
		update.mic_led = LIBRESENSE_MIC_LED_ON;

		libresense_print("mic led should be on...");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if (report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FLASH;
		libresense_print("mic led should be flashing...");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if (report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FAST_FLASH;
		libresense_print("mic led should be flashing faster (maybe)...");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if (report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_OFF;
		libresense_print("mic led should be off...");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		report_hid_close(handles, connected, 5000000, 10000);

	reset_mic:
		libresense_print("restoring mic based on state...");
		for (size_t i = 0; i < connected; ++i) {
			update.mic_led = datum[i].device.muted ? LIBRESENSE_MIC_LED_ON : LIBRESENSE_MIC_LED_OFF;
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		usleep(1000000);
	}

	{
		wait_until_options_clear(handles, connected, 250000);
		libresense_print("testing rumble...");
		float rumble;

		const float ONE_OVER_255 = 1.0f / 255.0f;
		libresense_print("large motor...");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, 0.0f, 0.0f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("small motor...");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], 0, rumble, 0.0f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("both motors...");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, rumble, 0.0f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("rumble feedback test...");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble_test % 2 == 0 ? 1.0f : 0.1f, rumble_test % 2 == 0 ? 1.0f : 0.1f, 0.0f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 250000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("large motor (legacy)...");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, 0.0f, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("small motor (legacy)...");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], 0, rumble, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("both motors (legacy)...");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, rumble, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		libresense_print("rumble feedback test (legacy)...");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble_test % 2 == 0 ? 1.0f : 0.1f, rumble_test % 2 == 0 ? 1.0f : 0.1f, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 250000, 10000)) {
				goto reset_motor;
			}
		}

	reset_motor:
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_rumble(handles[i], 0, 0, 0.0f, false);
		}
		libresense_push(handles, connected);
		usleep(100000);
	}

	{
		wait_until_options_clear(handles, connected, 250000);
		libresense_print("testing touchpad leds...");

		libresense_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.led = LIBRESENSE_LED_NONE;
		int i = 0;
		while (true) {
			if (i++ > 4 * 30) { // 250 ms per frame, 4 frames per second. 4 * 30 = 30 seconds worth of frames.
				break;
			}

			if (i < 10) {
				if (i == 1) {
					update.led = LIBRESENSE_LED_PLAYER_1;
				} else if (i == 2) {
					update.led = LIBRESENSE_LED_PLAYER_2;
				} else if (i == 3) {
					update.led = LIBRESENSE_LED_PLAYER_3;
				} else if (i == 4) {
					update.led = LIBRESENSE_LED_PLAYER_4;
				} else if (i == 5) {
					update.led = LIBRESENSE_LED_PLAYER_5;
				} else if (i == 6) {
					update.led = LIBRESENSE_LED_PLAYER_6;
				} else if (i == 7) {
					update.led = LIBRESENSE_LED_PLAYER_7;
				} else if (i == 8) {
					update.led = LIBRESENSE_LED_PLAYER_8;
				} else if (i == 9) {
					update.led = LIBRESENSE_LED_ALL;
				}
			} else if (i < 42) {
				update.led = i - 10;
			} else {
				const int v = (i - 43) % 8;
				if (v == 0) {
					update.led = LIBRESENSE_LED_1;
				} else if (v == 1) {
					update.led = LIBRESENSE_LED_2;
				} else if (v == 2) {
					update.led = LIBRESENSE_LED_3;
				} else if (v == 3) {
					update.led = LIBRESENSE_LED_4;
				} else if (v == 4) {
					update.led = LIBRESENSE_LED_5;
				} else if (v == 5) {
					update.led = LIBRESENSE_LED_4;
				} else if (v == 6) {
					update.led = LIBRESENSE_LED_3;
				} else if (v == 7) {
					update.led = LIBRESENSE_LED_2;
				}
			}

			for (size_t j = 0; j < connected; ++j) {
				libresense_update_led(handles[j], update);
			}
			libresense_push(handles, connected);
			const libresense_vector3 color = update.color;
			update.color.x = color.z;
			update.color.y = color.x;
			update.color.z = color.y;

			if (report_hid_close(handles, connected, 250000, 10000)) {
				goto reset_led;
			}
		}
	}

shutdown:
reset_led:
	{
		libresense_led_update update;
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		for (size_t j = 0; j < connected; ++j) {
			switch (j) {
				case 0: update.led = LIBRESENSE_LED_PLAYER_1; break;
				case 1: update.led = LIBRESENSE_LED_PLAYER_2; break;
				case 2: update.led = LIBRESENSE_LED_PLAYER_3; break;
				case 3: update.led = LIBRESENSE_LED_PLAYER_4; break;
				case 4: update.led = LIBRESENSE_LED_PLAYER_5; break;
				case 5: update.led = LIBRESENSE_LED_PLAYER_6; break;
				case 6: update.led = LIBRESENSE_LED_PLAYER_7; break;
				case 7: update.led = LIBRESENSE_LED_PLAYER_8; break;
				default: update.led = LIBRESENSE_LED_PLAYER_1; break;
			}
			libresense_update_led(handles[j], update);
		}
		libresense_push(handles, connected);
	}

	usleep(100000);

	for (size_t i = 0; i < connected; ++i) {
		libresense_close(handles[i]);
		if (IS_LIBRESENSE_BAD(result)) {
			libresense_errorf(stderr, result, "error closing hid");
		}
	}
	libresense_exit();
	return 0;
}
