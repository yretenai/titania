//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <config.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "libresensectl.h"

libresensectl_error libresensectl_mode_stub(libresensectl_context*) { return LIBRESENSECTL_NOT_IMPLEMENTED; }

const libresensectl_mode modes[] = { { "report", libresensectl_mode_report },
	{ "report-loop", libresensectl_mode_report_loop },
	{ "watch", libresensectl_mode_report_loop },
	{ "list", libresensectl_mode_list },
	{ "test", libresensectl_mode_test },
	{ "dump", libresensectl_mode_dump },
	{ "bench", libresensectl_mode_bench },
	{ "benchmark", libresensectl_mode_bench },
	{ "led", libresensectl_mode_led },
	{ "light", libresensectl_mode_led },
	{ "pair", libresensectl_mode_bt_pair },
	{ "disconnect", libresensectl_mode_bt_disconnect },
	{ "connect", libresensectl_mode_bt_connect },
	{ "usb", libresensectl_mode_bt_disconnect },
	{ "bt", libresensectl_mode_bt_connect },
	// Edge, Access
	{ "profile", libresensectl_mode_profile_funnel },
	//
	{ nullptr, nullptr } };

static libresensectl_context context = { 0 };
static bool interrupted = false; // separate in case i accidentally set should_stop somewhere else.
bool should_stop = false;

void shutdown() {
	if (interrupted) {
		return;
	}

	interrupted = should_stop = true;

	for (int i = 0; i < context.connected_controllers; ++i) {
		const libresense_result result = libresense_close(context.hids[i].handle);
		if (IS_LIBRESENSE_BAD(result)) {
			errorf(stderr, result, "error closing hid");
		}

		context.hids[i].handle = LIBRESENSE_INVALID_HANDLE_ID;
	}

	memset(&context, 0, sizeof(context));

	libresense_exit();
}

#ifdef _WIN32
BOOL WINAPI handle_sigint(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		shutdown();
	}

	return TRUE;
}
#else
#include <signal.h>

void handle_sigint(int signal) { shutdown(); }
#endif

int main(const int argc, const char** const argv) {
	printf(LIBRESENSE_PROJECT_NAME " version %s\n", LIBRESENSE_PROJECT_VERSION);

#ifdef _WIN32
	SetConsoleCtrlHandler(handle_sigint, true);
#else
	signal(SIGINT, handle_sigint);
#endif

	const char* mode = nullptr;
	bool calibrate = false;
	bool disable_regular = false;
	bool disable_edge = false;
	bool disable_access = false;
	bool disable_bt = false;
	bool disable_usb = false;

	int filtered_controllers = 0;
	libresense_serial filter[LIBRESENSECTL_CONTROLLER_COUNT] = { 0 };

	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			const int arglen = strlen(argv[i]);
			if (arglen < 1 || arglen > 254) {
				continue; // too long; skipped
			}

			char text[255];
			strcpy(text, argv[i]);
			for (char* p = text; *p; ++p) {
				*p = tolower(*p);
			}

			if (strcmp(text, "-h") == 0 || strcmp(text, "--help") == 0 || strcmp(text, "help") == 0) {
				printf("usage: libresensectl [opts] [mode [mode opts]]\n");
				printf("\n");
				printf("available options:\n");
				printf("\t-h, --help: print this help text\n");
				printf("\t-v, --version: print version and exit\n");
				printf("\t-d, --device: filter specific device id (up to 32)\n");
				printf("\t-c, --no-calibration: do not use calibration\n");
				printf("\t-r, --no-regular: disable regular controllers from being considered\n");
				printf("\t-e, --no-edge: disable edge controllers from being considered\n");
				printf("\t-a, --no-access: disable access controllers from being considered\n");
				printf("\t-b, --no-bt: disable bluetooth controllers from being considered\n");
				printf("\t-u, --no-usb: disable usb controllers from being considered\n");
				printf("\n");
				printf("available modes:\n");
				printf("\tlist: list every controller\n");
				printf("\treport: print the input report of connected controllers\n");
				printf("\ttest: test various features of the connected controllers\n");
				printf("\tdump: dump every feature report from connected controllers\n");
				printf("\tbench: benchmark report parsing speed\n");
				printf("\tled #rrggbb|off player-led: update LED color\n");
				printf("\tprofile convert path/to/report.{bin, json}: convert merged dualsense edge profile from or to json\n");
				printf("\tprofile import {square, cross, triangle, 1, 2, 3} path/to/profile.json: import a controller profile to the specified slot\n");
				printf("\tprofile export {triangle, square, cross, triangle, 0, 1, 2, 3} path/to/profile.json: export a controller profile to json\n");
				printf("\tprofile delete: delete a given profile\n");
				printf("\tpair address link-key: pair with a bluetooth adapter\n");
				printf("\tbt: instruct controller to connect via bluetooth\n");
				printf("\tusb: instruct controller to connect via usb\n");
				return 0;
			}

			if (strcmp(text, "-v") == 0 || strcmp(text, "--version") == 0 || strcmp(text, "version") == 0) {
				return 0;
			}

			if (strcmp(text, "-d") == 0 || strcmp(text, "--device") == 0) {
				const char* device_ptr = argv[++i];
				if (filtered_controllers >= LIBRESENSECTL_CONTROLLER_COUNT) {
					continue;
				}

				const int devlen = strlen(device_ptr);
				if (devlen > 0xff) {
					continue;
				}

				for (int j = 0; j < devlen; ++j) {
					filter[filtered_controllers][j] = device_ptr[j];
				}

				filtered_controllers++;
			} else if (strcmp(text, "-r") == 0 || strcmp(text, "--no-regular") == 0) {
				disable_regular = true;
			} else if (strcmp(text, "-e") == 0 || strcmp(text, "--no-edge") == 0) {
				disable_edge = true;
			} else if (strcmp(text, "-a") == 0 || strcmp(text, "--no-access") == 0) {
				disable_access = true;
			} else if (strcmp(text, "-b") == 0 || strcmp(text, "--no-bt") == 0 || strcmp(text, "--no-bluetooth") == 0) {
				disable_bt = true;
			} else if (strcmp(text, "-u") == 0 || strcmp(text, "--no-usb") == 0 || strcmp(text, "--only-bt") == 0 || strcmp(text, "--only-bluetooth") == 0) {
				disable_usb = true;
			} else if (strcmp(text, "-c") == 0 || strcmp(text, "--no-calibration") == 0) {
				calibrate = false;
			} else if (text[0] != '-') {
				mode = text;
				if (argc - i > 1) {
					context.argc = argc - i - 1;
					context.argv = argv + i + 1;
				}
				break;
			}
		}
	}

	if (should_stop) {
		return 0;
	}

	libresense_result result = libresense_init();
	if (IS_LIBRESENSE_BAD(result)) {
		errorf(stderr, result, "error initializing " LIBRESENSE_PROJECT_NAME);
		return result;
	}
	libresense_hid hids[LIBRESENSECTL_CONTROLLER_COUNT];
	result = libresense_get_hids(hids, LIBRESENSECTL_CONTROLLER_COUNT);
	if (IS_LIBRESENSE_BAD(result)) {
		errorf(stderr, result, "error getting hids");
		libresense_exit();
		return result;
	}

	for (int hid_id = 0; hid_id < LIBRESENSECTL_CONTROLLER_COUNT; ++hid_id) {
		if (should_stop) {
			return 0;
		}

		if (hids[hid_id].handle == LIBRESENSE_INVALID_HANDLE_ID) {
			continue;
		}

		if (disable_regular && !(hids[hid_id].is_edge || hids[hid_id].is_access)) {
			continue;
		}

		if (disable_edge && hids[hid_id].is_edge) {
			continue;
		}

		if (disable_access && hids[hid_id].is_access) {
			continue;
		}

		if (disable_bt && hids[hid_id].is_bluetooth) {
			continue;
		}

		if (disable_usb && !hids[hid_id].is_bluetooth) {
			continue;
		}

		if (filtered_controllers > 0) {
			for (int filter_id = 0; filter_id < filtered_controllers; ++filter_id) {
				if (memcmp(filter[filter_id], hids[hid_id].hid_serial, sizeof(libresense_serial)) != 0) {
					goto pass;
				}
			}

			continue;
		}

	pass:
		if (should_stop) {
			return 0;
		}

		context.hids[context.connected_controllers] = hids[hid_id];
		result = libresense_open(&context.hids[context.connected_controllers], calibrate, true);
		if (IS_LIBRESENSE_BAD(result)) {
			errorf(stderr, result, "error initializing hid");
			context.connected_controllers--;
			continue;
		}
		context.handles[context.connected_controllers] = context.hids[context.connected_controllers].handle;
		context.connected_controllers++;
	}

	if (context.connected_controllers == 0) {
		fprintf(stderr, "no hids... connect a device\n");
		libresense_exit();
		return 1;
	}

	if (mode == nullptr) {
		mode = "report";
	}

	const libresensectl_mode* current_mode = &modes[0];
	while (current_mode->name != nullptr) {
		if (strcmp(current_mode->name, mode) != 0) {
			current_mode++;
			continue;
		}

		const libresensectl_error error = current_mode->callback(&context);
		if (IS_LIBRESENSE_BAD(error)) {
			switch (error) {
				case LIBRESENSECTL_HID_ERROR: fprintf(stderr, "hid error while processing command\n"); break;
				case LIBRESENSECTL_INTERRUPTED: fprintf(stderr, "caught ctrl+c\n"); break;
				case LIBRESENSECTL_NOT_IMPLEMENTED: fprintf(stderr, "not implemented\n"); break;
				case LIBRESENSECTL_INVALID_ARGUMENTS: fprintf(stderr, "invalid arguments\n"); break;
				default: fprintf(stderr, "unknown error\n"); break;
			}
		}
		break;
	}

	shutdown();

	return 0;
}
