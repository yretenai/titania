//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif

#include <titania_config.h>

#include "titaniactl.h"

#include <json.h>

titaniactl_error titaniactl_mode_stub(titaniactl_context* _unused) { return TITANIACTL_NOT_IMPLEMENTED; }

const titaniactl_mode modes[] = { { "report", titaniactl_mode_report, titaniactl_mode_report_json, "print the input report of connected controllers", nullptr },
	{ "watch", titaniactl_mode_report_loop, titaniactl_mode_report_loop_json, "repeatedly print the input report of connected controllers", nullptr },
	{ "report-loop", titaniactl_mode_report_loop, titaniactl_mode_report_loop_json, nullptr, nullptr },
	{ "list", titaniactl_mode_list, titaniactl_mode_list_json, "list every controller", nullptr },
	{ "test", titaniactl_mode_test, nullptr, "test various features of the connected controllers", nullptr },
	{ "dump", titaniactl_mode_dump, nullptr, "dump every feature report from connected controllers", nullptr },
	{ "benchmark", titaniactl_mode_bench, nullptr, "benchmark report parsing speed", nullptr },
	{ "bench", titaniactl_mode_bench, nullptr, nullptr, nullptr },
	{ "led", titaniactl_mode_led, titaniactl_mode_led, "update LED color", "#rrggbb|off player-led" },
	{ "light", titaniactl_mode_led, titaniactl_mode_led, nullptr, nullptr },
	{ "pair", titaniactl_mode_bt_pair, titaniactl_mode_bt_pair, "pair with a bluetooth adapter", "address link-key" },
	{ "usb", titaniactl_mode_bt_disconnect, titaniactl_mode_bt_disconnect, "instruct controller to connect via bluetooth", nullptr },
	{ "bt", titaniactl_mode_bt_connect, titaniactl_mode_bt_connect, "instruct controller to connect via usb", nullptr },
	{ "disconnect", titaniactl_mode_bt_disconnect, titaniactl_mode_bt_disconnect, nullptr, nullptr },
	{ "connect", titaniactl_mode_bt_connect, titaniactl_mode_bt_connect, nullptr, nullptr },
	// Edge, Access
	{ "profile", titaniactl_mode_profile_funnel, titaniactl_mode_profile_funnel, nullptr, nullptr },
	{ "profile convert", nullptr, nullptr, "convert merged dualsense edge profile from or to json", "path/to/report.{bin, json}" },
	{ "profile import", nullptr, nullptr, "import a controller profile to the specified slot", "{square, cross, circle, 1, 2, 3} path/to/profile.json" },
	{ "profile export", nullptr, nullptr, "export a controller profile to json", "{triangle, square, cross, circle, 0, 1, 2, 3} path/to/profile.json" },
	{ "profile delete", nullptr, nullptr, "delete a given profile", nullptr },
	//
	{ nullptr, nullptr, nullptr, nullptr, nullptr } };

static titaniactl_context context = { 0 };
static bool interrupted = false; // separate in case i accidentally set should_stop somewhere else.
bool should_stop = false;

void shutdown(void) {
	if (interrupted) {
		return;
	}

	interrupted = should_stop = true;

	for (int i = 0; i < context.connected_controllers; ++i) {
		titania_close(context.hids[i].handle);
		context.hids[i].handle = TITANIA_INVALID_HANDLE_ID;
	}

	memset(&context, 0, sizeof(context));

	titania_exit();
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

void titaniactl_errorf(const char* error, const char* message) {
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

void titania_errorf(const titania_result result, const char* message) {
	if (CHECK_ENUM_SAFE(result, titania_error_msg)) {
		titaniactl_errorf(titania_error_msg[result], message);
	} else {
		titaniactl_errorf("malformed titania error", message);
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
	titania_serial filter[TITANIACTL_CONTROLLER_COUNT] = { 0 };

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
				printf(TITANIA_PROJECT_NAME " version %s\n", TITANIA_PROJECT_VERSION);
				printf("usage: titaniactl [opts] [mode [mode opts]]\n");
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
				for (size_t j = 0; j < sizeof(modes) / sizeof(titaniactl_mode); ++j) {
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
				for (size_t j = 0; j < sizeof(modes) / sizeof(titaniactl_mode); ++j) {
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
					json_object_add_string(obj, "version", TITANIA_PROJECT_VERSION);
					json_object_add_string(obj, "name", TITANIA_PROJECT_NAME);
					json_object_add_number(obj, "max_controllers", titania_max_controllers);
					char* json_text = json_print(obj);
					printf("%s\n", json_text);
					json_delete(obj);
					free(json_text);
				} else {
					printf(TITANIA_PROJECT_NAME " version %s\n", TITANIA_PROJECT_VERSION);
				}
				return 0;
			}

			if (strcmp(text, "-j") == 0 || strcmp(text, "--json") == 0) {
				is_json = true;
			}

			if (strcmp(text, "-d") == 0 || strcmp(text, "--device") == 0) {
				const char* device_ptr = argv[++i];
				if (filtered_controllers >= TITANIACTL_CONTROLLER_COUNT) {
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

	const titaniactl_mode* current_mode = &modes[0];
	titaniactl_callback_t cb = nullptr;
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
		printf(TITANIA_PROJECT_NAME " version %s\n", TITANIA_PROJECT_VERSION);
	}

	if (cb == nullptr) {
		titaniactl_errorf("invalid mode", "mode not recognized");
	}

	titania_result result = titania_init();
	if (IS_TITANIA_BAD(result)) {
		titania_errorf(result, "error initializing " TITANIA_PROJECT_NAME);
		return result;
	}
	titania_query query[TITANIACTL_CONTROLLER_COUNT];
	result = titania_get_hids(query, TITANIACTL_CONTROLLER_COUNT);
	if (IS_TITANIA_BAD(result)) {
		titania_errorf(result, "error getting hids");
		titania_exit();
		return result;
	}

	for (int hid_id = 0; hid_id < TITANIACTL_CONTROLLER_COUNT; ++hid_id) {
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
				if (query[hid_id].hid_serial[0] != 0 && memcmp(filter[filter_id], query[hid_id].hid_serial, sizeof(titania_serial)) != 0) {
					goto pass;
				}
			}

			continue;
		}

	pass:
		if (should_stop) {
			return 0;
		}

		result = titania_open(query[hid_id].hid_path, query[hid_id].is_bluetooth, &context.hids[context.connected_controllers], calibrate, blocking);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "error initializing hid");
			continue;
		}
		context.handles[context.connected_controllers] = context.hids[context.connected_controllers].handle;
		context.connected_controllers++;
	}

	if (context.connected_controllers < 1) {
		titaniactl_errorf("no hids", "connect a device");
		titania_exit();
		return 1;
	}

	titaniactl_error error = cb(&context);
	if (IS_TITANIACTL_BAD(error)) {
		switch (error & TITANIACTL_MASK) {
			case TITANIACTL_TITANIA_ERROR: break;
			case TITANIACTL_HID_ERROR: titaniactl_errorf("hid error", "errored while processing command"); break;
			case TITANIACTL_INTERRUPTED:
				if (is_json) {
					error = TITANIACTL_OK_NO_JSON_INTERRUPTED;
				} else {
					titaniactl_errorf("caught ctrl+c", "exiting");
				}
				break;
			case TITANIACTL_NOT_IMPLEMENTED: titaniactl_errorf("not implemented", "sorry"); break;
			case TITANIACTL_INVALID_ARGUMENTS: titaniactl_errorf("invalid arguments", "one of the arguments was invalid"); break;
			case TITANIACTL_INVALID_PAIR_ARGUMENTS: titaniactl_errorf("invalid arguments", "you need to provide a mac address and a bluetooth link key"); break;
			case TITANIACTL_INVALID_MAC_ADDRESS: titaniactl_errorf("invalid arguments", "mac address is not valid"); break;
			case TITANIACTL_INVALID_LINK_KEY: titaniactl_errorf("invalid arguments", "bluetooth link key is not 16 characters"); break;
			case TITANIACTL_INVALID_PROFILE: titaniactl_errorf("invalid profile", "profile data is not valid to import"); break;
			case TITANIACTL_EMPTY_PROFILE: titaniactl_errorf("empty profile", "profile is empty"); break;
			case TITANIACTL_FILE_READ_ERROR: titaniactl_errorf("file read error", "could not open or read file"); break;
			case TITANIACTL_FILE_WRITE_ERROR: titaniactl_errorf("file write error", "could not open or write file"); break;
			default: titaniactl_errorf("unexpected error", "goodbye"); break;
		}
	}

	shutdown();

	if ((error & TITANIACTL_MASK) == TITANIACTL_TITANIA_ERROR) {
		return error;
	}

	if (is_json && error > TITANIACTL_OK_NO_JSON) {
		struct json* obj = json_new_object();
		json_object_add_bool(obj, "success", true);
		char* json_text = json_print(obj);
		printf("%s\n", json_text);
		json_delete(obj);
		free(json_text);
	}

	if (error == TITANIACTL_OK_NO_JSON_INTERRUPTED) {
		return TITANIACTL_INTERRUPTED;
	}

	if (error == TITANIACTL_OK_NO_JSON) {
		return TITANIACTL_OK;
	}

	return error;
}
