//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif

#include <libresense_config.h>

#include "libresensectl.h"

#include <json.h>

libresensectl_error libresensectl_mode_stub(libresensectl_context* _unused) { return LIBRESENSECTL_NOT_IMPLEMENTED; }

const libresensectl_mode modes[] = { { "report", libresensectl_mode_report, libresensectl_mode_report_json, "print the input report of connected controllers", nullptr },
	{ "watch", libresensectl_mode_report_loop, libresensectl_mode_report_loop_json, "repeatedly print the input report of connected controllers", nullptr },
	{ "report-loop", libresensectl_mode_report_loop, libresensectl_mode_report_loop_json, nullptr, nullptr },
	{ "list", libresensectl_mode_list, libresensectl_mode_list_json, "list every controller", nullptr },
	{ "test", libresensectl_mode_test, nullptr, "test various features of the connected controllers", nullptr },
	{ "dump", libresensectl_mode_dump, nullptr, "dump every feature report from connected controllers", nullptr },
	{ "benchmark", libresensectl_mode_bench, nullptr, "benchmark report parsing speed", nullptr },
	{ "bench", libresensectl_mode_bench, nullptr, nullptr, nullptr },
	{ "led", libresensectl_mode_led, libresensectl_mode_led, "update LED color", "#rrggbb|off player-led" },
	{ "light", libresensectl_mode_led, libresensectl_mode_led, nullptr, nullptr },
	{ "pair", libresensectl_mode_bt_pair, libresensectl_mode_bt_pair, "pair with a bluetooth adapter", "address link-key" },
	{ "usb", libresensectl_mode_bt_disconnect, libresensectl_mode_bt_disconnect, "instruct controller to connect via bluetooth", nullptr },
	{ "bt", libresensectl_mode_bt_connect, libresensectl_mode_bt_connect, "instruct controller to connect via usb", nullptr },
	{ "disconnect", libresensectl_mode_bt_disconnect, libresensectl_mode_bt_disconnect, nullptr, nullptr },
	{ "connect", libresensectl_mode_bt_connect, libresensectl_mode_bt_connect, nullptr, nullptr },
	// Edge, Access
	{ "profile", libresensectl_mode_profile_funnel, libresensectl_mode_profile_funnel, nullptr, nullptr },
	{ "profile convert", nullptr, nullptr, "convert merged dualsense edge profile from or to json", "path/to/report.{bin, json}" },
	{ "profile import", nullptr, nullptr, "import a controller profile to the specified slot", "{square, cross, triangle, 1, 2, 3} path/to/profile.json" },
	{ "profile export", nullptr, nullptr, "export a controller profile to json", "{triangle, square, cross, triangle, 0, 1, 2, 3} path/to/profile.json" },
	{ "profile delete", nullptr, nullptr, "delete a given profile", nullptr },
	//
	{ nullptr, nullptr, nullptr, nullptr, nullptr } };

static libresensectl_context context = { 0 };
static bool interrupted = false; // separate in case i accidentally set should_stop somewhere else.
bool should_stop = false;

void shutdown(void) {
	if (interrupted) {
		return;
	}

	interrupted = should_stop = true;

	for (int i = 0; i < context.connected_controllers; ++i) {
		libresense_close(context.hids[i].handle);
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

bool is_json = false;

void libresensectl_errorf(const char* error, const char* message) {
	if (is_json) {
		struct json* obj = json_new_object();
		json_object_add_bool(obj, "success", false);
		json_object_add_string(obj, "error", error);
		json_object_add_string(obj, "message", message);
		char* json_text = json_print(obj);
		printf("%s\n", json_text);
		json_delete(obj);
		free(json_text);
		return;
	}

	fprintf(stderr, "%s: %s\n", message, error);
}

void libresense_errorf(const libresense_result result, const char* message) {
	if (CHECK_ENUM_SAFE(result, libresense_error_msg)) {
		libresensectl_errorf(libresense_error_msg[result], message);
	} else {
		libresensectl_errorf("malformed libresense error", message);
	}
}

int main(const int argc, const char** const argv) {
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
	bool blocking = true;

	int filtered_controllers = 0;
	libresense_serial filter[LIBRESENSECTL_CONTROLLER_COUNT] = { 0 };

	char text[255];
	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			const int arglen = strlen(argv[i]);
			if (arglen < 1 || arglen > 254) {
				continue; // too long; skipped
			}

			strcpy(text, argv[i]);
			for (char* p = text; *p; ++p) {
				*p = tolower(*p);
			}

			if (strcmp(text, "-h") == 0 || strcmp(text, "--help") == 0 || strcmp(text, "help") == 0) {
				printf(LIBRESENSE_PROJECT_NAME " version %s\n", LIBRESENSE_PROJECT_VERSION);
				printf("usage: libresensectl [opts] [mode [mode opts]]\n");
				printf("\n");
				printf("available options:\n");
				printf("\t-h, --help: print this help text\n");
				printf("\t-v, --version: print version and exit\n");
				printf("\t-j, --json: output json to stdout\n");
				printf("\t-d, --device: filter specific device id (up to 32)\n");
				printf("\t-c, --no-calibration: do not use calibration\n");
				printf("\t-r, --no-regular: disable regular controllers from being considered\n");
				printf("\t-e, --no-edge: disable edge controllers from being considered\n");
				printf("\t-a, --no-access: disable access controllers from being considered\n");
				printf("\t-b, --no-bt: disable bluetooth controllers from being considered\n");
				printf("\t-u, --no-usb: disable usb controllers from being considered\n");
				printf("\t-z, --non-blocking: disable blocking reads\n");
				printf("\n");
				printf("available modes:\n");
				for (size_t j = 0; j < sizeof(modes) / sizeof(libresensectl_mode); ++j) {
					if (modes[j].help == nullptr) {
						continue;
					}

					printf("\t%s", modes[j].name);
					if (modes[j].args != nullptr) {
						printf(" %s", modes[j].args);
					}

					printf(": %s\n", modes[j].help);
				}

				bool has_json_lead = false;
				for (size_t j = 0; j < sizeof(modes) / sizeof(libresensectl_mode); ++j) {
					if (modes[j].json_callback == nullptr || modes[j].name == nullptr) {
						continue;
					}

					if (!has_json_lead) {
						printf("\nthese modes support json output: ");
						has_json_lead = true;
					} else {
						printf(", ");
					}

					printf("%s", modes[j].name);
				}

				if (has_json_lead) {
					printf("\n");
				}
				return 0;
			}

			if (strcmp(text, "-v") == 0 || strcmp(text, "--version") == 0 || strcmp(text, "version") == 0) {
				if (is_json) {
					struct json* obj = json_new_object();
					json_object_add_string(obj, "version", LIBRESENSE_PROJECT_VERSION);
					json_object_add_string(obj, "name", LIBRESENSE_PROJECT_NAME);
					json_object_add_number(obj, "max_controllers", libresense_max_controllers);
					char* json_text = json_print(obj);
					printf("%s\n", json_text);
					json_delete(obj);
					free(json_text);
				} else {
					printf(LIBRESENSE_PROJECT_NAME " version %s\n", LIBRESENSE_PROJECT_VERSION);
				}
				return 0;
			}

			if (strcmp(text, "-j") == 0 || strcmp(text, "--json") == 0) {
				is_json = true;
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
			} else if (strcmp(text, "-z") == 0 || strcmp(text, "--non-blocking") == 0) {
				blocking = false;
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

	if (mode == nullptr) {
		mode = "report";
	}

	const libresensectl_mode* current_mode = &modes[0];
	libresensectl_callback_t cb = nullptr;
	while (current_mode->name != nullptr) {
		if (strcmp(current_mode->name, mode) != 0) {
			current_mode++;
			continue;
		}

		cb = current_mode->callback;
		if (is_json && current_mode->json_callback != nullptr) {
			cb = current_mode->json_callback;
		} else {
			is_json = false;
		}

		break;
	}

	if (!is_json) {
		printf(LIBRESENSE_PROJECT_NAME " version %s\n", LIBRESENSE_PROJECT_VERSION);
	}

	if (cb == nullptr) {
		libresensectl_errorf("invalid mode", "mode not recognized");
	}

	libresense_result result = libresense_init();
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(result, "error initializing " LIBRESENSE_PROJECT_NAME);
		return result;
	}
	libresense_query query[LIBRESENSECTL_CONTROLLER_COUNT];
	result = libresense_get_hids(query, LIBRESENSECTL_CONTROLLER_COUNT);
	if (IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(result, "error getting hids");
		libresense_exit();
		return result;
	}

	for (int hid_id = 0; hid_id < LIBRESENSECTL_CONTROLLER_COUNT; ++hid_id) {
		if (should_stop) {
			return 0;
		}

		if (query[hid_id].hid_path[0] == 0) {
			continue;
		}

		if (disable_regular && !(query[hid_id].is_edge || query[hid_id].is_access)) {
			continue;
		}

		if (disable_edge && query[hid_id].is_edge) {
			continue;
		}

		if (disable_access && query[hid_id].is_access) {
			continue;
		}

		if (disable_bt && query[hid_id].is_bluetooth) {
			continue;
		}

		if (disable_usb && !query[hid_id].is_bluetooth) {
			continue;
		}

		if (filtered_controllers > 0) {
			for (int filter_id = 0; filter_id < filtered_controllers; ++filter_id) {
				if (query[hid_id].hid_serial[0] != 0 && memcmp(filter[filter_id], query[hid_id].hid_serial, sizeof(libresense_serial)) != 0) {
					goto pass;
				}
			}

			continue;
		}

	pass:
		if (should_stop) {
			return 0;
		}

		result = libresense_open(query[hid_id].hid_path, query[hid_id].is_bluetooth, &context.hids[context.connected_controllers], calibrate, blocking);
		if (IS_LIBRESENSE_BAD(result)) {
			libresense_errorf(result, "error initializing hid");
			context.connected_controllers--;
			continue;
		}
		context.handles[context.connected_controllers] = context.hids[context.connected_controllers].handle;
		context.connected_controllers++;
	}

	if (context.connected_controllers == 0) {
		libresensectl_errorf("no hids", "connect a device");
		libresense_exit();
		return 1;
	}

	libresensectl_error error = cb(&context);
	if (IS_LIBRESENSECTL_BAD(error)) {
		switch (error & LIBRESENSECTL_MASK) {
			case LIBRESENSECTL_LIBRESENSE_ERROR: break;
			case LIBRESENSECTL_HID_ERROR: libresensectl_errorf("hid error", "errored while processing command"); break;
			case LIBRESENSECTL_INTERRUPTED:
				if (is_json) {
					error = LIBRESENSECTL_OK_NO_JSON_INTERRUPTED;
				} else {
					libresensectl_errorf("caught ctrl+c", "exiting");
				}
				break;
			case LIBRESENSECTL_NOT_IMPLEMENTED: libresensectl_errorf("not implemented", "sorry"); break;
			case LIBRESENSECTL_INVALID_ARGUMENTS: libresensectl_errorf("invalid arguments", "one of the arguments was invalid"); break;
			case LIBRESENSECTL_INVALID_PAIR_ARGUMENTS: libresensectl_errorf("invalid arguments", "you need to provide a mac address and a bluetooth link key"); break;
			case LIBRESENSECTL_INVALID_MAC_ADDRESS: libresensectl_errorf("invalid arguments", "mac address is not valid"); break;
			case LIBRESENSECTL_INVALID_LINK_KEY: libresensectl_errorf("invalid arguments", "bluetooth link key is not 16 characters"); break;
			case LIBRESENSECTL_INVALID_PROFILE: libresensectl_errorf("invalid profile", "profile data is not valid to import"); break;
			case LIBRESENSECTL_EMPTY_PROFILE: libresensectl_errorf("empty profile", "profile is empty"); break;
			case LIBRESENSECTL_FILE_READ_ERROR: libresensectl_errorf("file read error", "could not open or read file"); break;
			case LIBRESENSECTL_FILE_WRITE_ERROR: libresensectl_errorf("file write error", "could not open or write file"); break;
			default: libresensectl_errorf("unexpected error", "goodbye"); break;
		}
	}

	shutdown();

	if ((error & LIBRESENSECTL_MASK) == LIBRESENSECTL_LIBRESENSE_ERROR) {
		return error;
	}

	if (is_json && error > LIBRESENSECTL_OK_NO_JSON) {
		struct json* obj = json_new_object();
		json_object_add_bool(obj, "success", true);
		char* json_text = json_print(obj);
		printf("%s\n", json_text);
		json_delete(obj);
		free(json_text);
	}

	if (error == LIBRESENSECTL_OK_NO_JSON_INTERRUPTED) {
		return LIBRESENSECTL_INTERRUPTED;
	}

	if (error == LIBRESENSECTL_OK_NO_JSON) {
		return LIBRESENSECTL_OK;
	}

	return error;
}
