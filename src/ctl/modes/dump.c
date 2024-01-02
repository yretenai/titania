//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <hidapi.h>
#include <stdio.h>

const char* const REPORT_TYPES[3] = { "INPUT", "OUTPUT", "FEATURE" };

libresensectl_error libresensectl_mode_dump(libresensectl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		char name[0x30] = { 0 };
		sprintf(name, "report_%s_%%d.bin", context->hids[i].serial.mac);
		libresense_report_id report_ids[0xFF];
		hid_device* device;
		if (IS_LIBRESENSE_OKAY(libresense_debug_get_hid(context->hids[i].handle, (intptr_t*) &device)) &&
			IS_LIBRESENSE_OKAY(libresense_debug_get_hid_report_ids(context->hids[i].handle, report_ids))) {
			for (int j = 0; j < 0xFF; j++) {
				if (should_stop) {
					return LIBRESENSECTL_INTERRUPTED;
				}

				uint8_t buffer[0x4096];
				if (report_ids[j].id == 0) {
					break;
				}

				const int32_t hid_report_size = report_ids[j].size + 1;
				printf("report %d (%x): reported size is %d, type is %s", report_ids[j].id, report_ids[j].id, hid_report_size, REPORT_TYPES[report_ids[j].type % 3]);

				if (report_ids[j].type < 2) {
					printf("\n");
					continue;
				}

				printf(", actual size is ");
				buffer[0] = report_ids[j].id;
				const size_t size = hid_get_feature_report(device, buffer, hid_report_size);
				if (size > 1 && size <= 0x4096) {
					printf("%ld\n", size);
					char report_name[0x30] = { 0 };
					sprintf(report_name, name, report_ids[j].id);
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
	}

	return LIBRESENSECTL_OK;
}
