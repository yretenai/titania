#include <hidapi/hidapi.h>
#include <libresense.h>
#include <stdio.h>

int main(void) {
	libresense_init();
	libresense_hid hid;
	libresense_get_hids(&hid, 1);
	if(hid.handle != LIBRESENSE_INVALID_HANDLE) {
		libresense_open(&hid);
#ifdef LIBRESENSE_DEBUG
		uint8_t buffer[0x4096];
		for (int i = 0; i < 0xFF; i++) {
			if(hid.report_ids[i].id == 0) {
				break;
			}

			printf("report %d: reported size is %ld\n\t", hid.report_ids[i].id, hid.report_ids[i].size);

			size_t size = libresense_debug_get_feature_report(hid.handle, hid.report_ids[i].id, (uint8_t*) buffer, hid.report_ids[i].size <= 0x40 ? 0x40 : hid.report_ids[i].size);
			printf("actual size is %ld\n\t", size);
			if(size > 1 && size <= 0x4096) {
				for(size_t j = 0; j < size; j++) {
					printf("%02x ", buffer[j]);
				}
			}
			printf("\n");
		}
#endif
	} else {
		libresense_exit();
		return 1;
	}
	libresense_data data;
	libresense_handle handle = hid.handle;
	// libresense_poll(&handle, 1, &data);
	libresense_close(handle);
	libresense_exit();
	return 0;
}
