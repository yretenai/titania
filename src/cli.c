//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <hidapi.h>
#include <libresense.h>

#include <stdio.h>
#include <string.h>

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

int main(int argc, const char** argv) {
	libresense_printf("version %s", LIBRESENSE_PROJECT_VERSION);

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
		}
	}

	if (connected == 0) {
		fprintf(stderr, "[" LIBRESENSE_PROJECT_NAME "] no hids... connect a device\n");
		libresense_exit();
		return 1;
	}

	libresense_data datum[libresense_max_controllers];
	result = libresense_pull(handles, connected, datum);
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting report");
		return result;
	}

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

	for (size_t i = 0; i < connected; ++i) {
		for (libresense_profile_id j = LIBRESENSE_PROFILE_1; j < LIBRESENSE_PROFILE_MAX; ++j) {
			print_profile(j, hid->edge_profiles[j]);
		}
	}

	for (size_t i = 0; i < connected; ++i) {
		libresense_close(handles[i]);
		if (IS_LIBRESENSE_BAD(result)) {
			libresense_errorf(stderr, result, "error closing hid");
		}
	}

	libresense_exit();
	return 0;
}
