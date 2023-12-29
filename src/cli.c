//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <hidapi/hidapi.h>
#include <libresense.h>

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>

// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void
usleep(__int64 usec) {
	HANDLE timer;
	LARGE_INTEGER ft;
	ft.QuadPart = -(10 * usec);
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#else
#define __USE_XOPEN_EXTENDED
#include <unistd.h>
#define clrscr() printf("\033[1;1H\033[2J")
#endif

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
#define LIBREPRINT_ENUM(struc, field, strs, name) printf(" " name " = %s", strs[struc.field])
#define LIBREPRINT_TEST(struc, field) printf(" " #field " = %s", struc.field ? "Y" : "N")
#define LIBREPRINT_BUTTON_TEST(field) printf(" " #field " = %s", data.buttons.field ? "Y" : "N")
#define LIBREPRINT_EDGE_BUTTON_TEST(field) printf(" " #field " = %s", data.edge_device.raw_buttons.field ? "Y" : "N")

#define libresense_errorf(fp, result, fmt) fprintf(fp, "[libresense] " fmt ": %s\n", libresense_error_msg[result])

bool
report_hid_trigger(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
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

bool
report_hid_close(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
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

void
wait_until_options_clear(libresense_handle* handles, const size_t handle_count, __useconds_t timeout) {
	libresense_data data;

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

int
main(int argc, const char **argv) {
	libresense_result result = libresense_init();
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error initializing libresense");
		return result;
	}
	libresense_hid hid[LIBRESENSE_MAX_CONTROLLERS];
	libresense_handle handles[LIBRESENSE_MAX_CONTROLLERS];
	result = libresense_get_hids(hid, LIBRESENSE_MAX_CONTROLLERS);
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting hids");
		libresense_exit();
		return result;
	}
	size_t connected = 0;
	for (int hid_id = 0; hid_id < LIBRESENSE_MAX_CONTROLLERS; ++hid_id) {
		if (hid[hid_id].handle != LIBRESENSE_INVALID_HANDLE_ID) {
			result = libresense_open(&hid[hid_id]);
			if (IS_LIBRESENSE_BAD(result)) {
				libresense_errorf(stderr, result, "error initializing hid");
				libresense_exit();
				return result;
			}
			printf("connected to hid %s\n", hid[hid_id].serial.mac);
			handles[connected++] = hid[hid_id].handle;
#ifdef LIBRESENSE_DEBUG
			char name[0x30] = { 0 };
			sprintf(name, "report_%s_%%d.bin", hid[hid_id].serial.paired_mac);

			for (int i = 0; i < 0xFF; i++) {
				uint8_t buffer[0x4096];
				if (hid[hid_id].report_ids[i].id == 0) {
					break;
				}

				int32_t hid_report_size = hid[hid_id].report_ids[i].size + 1;
				printf("report %d (%x): reported size is %d, type is %s",
					   hid[hid_id].report_ids[i].id,
					   hid[hid_id].report_ids[i].id,
					   hid_report_size,
					   hid[hid_id].report_ids[i].type == 0	 ? "INPUT"
					   : hid[hid_id].report_ids[i].type == 1 ? "OUTPUT"
															 : "FEATURE");

				if (hid[hid_id].report_ids[i].type < 2) {
					printf("\n");
					continue;
				}

				printf(", actual size is ");
				size_t size = libresense_debug_get_feature_report(hid[hid_id].handle, hid[hid_id].report_ids[i].id, buffer, hid_report_size);
				if (size > 1 && size <= 0x4096) {
					printf("%ld\n", size);
					char report_name[0x30] = { 0 };
					sprintf(report_name, name, hid[hid_id].report_ids[i].id);
					FILE* file = fopen(report_name, "w+b");
					if (file != NULL) {
						fwrite(buffer, 1, size, file);
						fclose(file);
					}
				} else {
					printf("??\n");
				}
			}
#endif
		}
	}

	if (connected == 0) {
		fprintf(stderr, "[libresense] no hids... connect a device\n");
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
			LIBREPRINT_TEST(data.hid, is_edge);
			printf(" }\n");

			printf("firmware {");
			LIBREPRINT_X16(hid->firmware, type);
			LIBREPRINT_X16(hid->firmware, series);
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
			LIBREPRINT_BUTTON_TEST(ps); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(touch); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(mute); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_f1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_f2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_lb); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_rb);
			printf(" }\n");

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

			printf("sticks { left = {");
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_LEFT], x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_LEFT], y);
			printf(" }, right = {");
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_RIGHT], x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_RIGHT], y);
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
				LIBREPRINT_EDGE_BUTTON_TEST(l1); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(r1); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(l2); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(r2); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(share); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(option); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(l3); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(r3); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(ps); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(touch); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(mute); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(edge_f1); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(edge_f2); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(edge_lb); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(edge_rb);
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
				LIBREPRINT_TEST(data.edge_device, unknown);
				printf(" }\n");
			}
			// clang-format on
		}

		if (!clr) {
			break;
		}

		clrscr();
	}

	if (argc > 1 && !bench) {
		goto shutdown;
	}

	{
		printf("testing latency, this will take 10 seconds\n");
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
		printf("min: %ld s %ld us, max: %ld s %ld us\n", min.tv_sec, min.tv_nsec, max.tv_sec, max.tv_nsec);
	}

	if (argc > 1) {
		goto shutdown;
	}

	printf("press OPTIONS to skip test\n");

	{
		wait_until_options_clear(handles, connected, 250000);
		printf("testing adaptive triggers\n");
		libresense_effect_update update = { 0 };

		update.mode = LIBRESENSE_EFFECT_UNIFORM;
		update.effect.uniform.position = 0.5;
		update.effect.uniform.resistance = 1.0;
		printf("uniform\n");
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
		printf("section\n");
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
		printf("multiple section\n");
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
		printf("trigger\n");
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
		printf("slope\n");
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
		printf("vibrate\n");
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
		printf("vibrate slope\n");
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
		printf("multiple vibrate\n");
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
		printf("multiple vibrate sections\n");
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
		printf("testing mic led...\n");
		libresense_audio_update update = { 0 };
		update.jack_volume = 1.0;
		update.speaker_volume = 1.0;
		update.microphone_volume = 1.0;
		update.mic_selection = LIBRESENSE_MIC_AUTO;
		update.mic_balance = LIBRESENSE_MIC_AUTO;
		update.disable_audio_jack = false;
		update.force_enable_speaker = false;
		update.mic_led = LIBRESENSE_MIC_LED_ON;

		printf("mic led should be on...\n");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if (report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FLASH;
		printf("mic led should be flashing...\n");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if (report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FAST_FLASH;
		printf("mic led should be flashing faster (maybe)...\n");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if (report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_OFF;
		printf("mic led should be off...\n");
		for (size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		report_hid_close(handles, connected, 5000000, 10000);

	reset_mic:
		printf("restoring mic based on state...\n");
		for (size_t i = 0; i < connected; ++i) {
			update.mic_led = datum[i].device.muted ? LIBRESENSE_MIC_LED_ON : LIBRESENSE_MIC_LED_OFF;
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		usleep(1000000);
	}

	{
		wait_until_options_clear(handles, connected, 250000);
		printf("testing rumble...\n");
		float rumble;

		const float ONE_OVER_255 = 1.0f / 255.0f;
		printf("large motor...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, 0.0f, 0.5f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], 0, rumble, 0.5f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, rumble, 0.5f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test...\n");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble_test % 2 == 0 ? 1.0f : 0.1f, rumble_test % 2 == 0 ? 1.0f : 0.1f, 0.5f, false);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 250000, 10000)) {
				goto reset_motor;
			}
		}

		printf("large motor (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, 0.0f, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], 0, rumble, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, rumble, 0.0f, true);
			}
			libresense_push(handles, connected);
			if (report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test (legacy)...\n");
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
		printf("testing touchpad leds...\n");

		libresense_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.brightness = LIBRESENSE_LEVEL_HIGH;
		update.led = LIBRESENSE_LED_NONE;
		update.effect = LIBRESENSE_LED_EFFECT_OFF;
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
		update.brightness = LIBRESENSE_LEVEL_HIGH;
		update.effect = LIBRESENSE_LED_EFFECT_OFF;
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
