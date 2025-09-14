//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#ifdef __APPLE__
#include <IOKit/hid/IOHIDManager.h>
#endif

#include "structures.h"
#include "titania.h"

size_t resample_48000_to_3000(const titania_handle handle, int8_t samples3khz[DUALSENSE_SAMPLE_SIZE]) {
	constexpr size_t size = DUALSENSE_NUM_SAMPLES * 4 * 16; // 4 channel float @ 48KHz
	float samples[size];
	memset(samples, 0, size);
	size_t num_frames = DUALSENSE_NUM_SAMPLES; // 32 samples,
	if (IS_TITANIA_BAD(titania_haptics_copy_samples(handle, samples, &num_frames))) {
		return 0;
	}

	for (size_t i = 0; i < num_frames * 4; i += 16 * 4) {
		float l = samples[i * 4 + 2];
		float r = samples[i * 4 + 3];

		for (size_t j = 1; j < 16; j++) {
			l += samples[(i + j) * 4 + 2];
			r += samples[(i + j) * 4 + 3];
		}

		samples3khz[i / 4 * 2 + 0] = (int8_t) (l / 16.0f * 127.0f);
		samples3khz[i / 4 * 2 + 1] = (int8_t) (r / 16.0f * 127.0f);
	}

	return num_frames;
}

#ifdef __APPLE__
void stub(void* context, IOReturn, void*, IOHIDReportType, uint32_t, uint8_t*, CFIndex) {
	atomic_store(&state[(titania_handle) (intptr_t) context].haptics.bt_timer.slop_mutex, false);
}
#endif

void titania_bt_loop_inner(const titania_handle handle) {
#ifdef __APPLE__
	atomic_store(&state[handle].haptics.bt_timer.slop_mutex, false);
#endif

	while (1) {
		if (!atomic_load(&state[handle].haptics.bt_timer.running)) {
			break;
		}

#ifdef __APPLE__
		if (atomic_load(&state[handle].haptics.bt_timer.slop_mutex)) {
			continue;
		}
		atomic_store(&state[handle].haptics.bt_timer.slop_mutex, true);
#endif

		dualsense_bt_cmd_packet packet_0x11 = { 0 };
		packet_0x11.packet = DUALSENSE_BT_REPORT_HAPTICS_SETUP;
		packet_0x11.length_prefixed.length = 7;
		packet_0x11.length_prefixed.data[0] = 0b11111110;
		packet_0x11.length_prefixed.data[5] = 0xFF;
		packet_0x11.length_prefixed.data[6] = state[handle].haptics.bt_timer.frame++;

		dualsense_bt_cmd_packet packet_0x12 = { 0 };
		packet_0x12.packet = DUALSENSE_BT_REPORT_HAPTICS_GRANULE;
		packet_0x12.length_prefixed.length = DUALSENSE_SAMPLE_SIZE;
		if (resample_48000_to_3000(handle, (int8_t*) &packet_0x12.length_prefixed.data[0]) > 0) {
			dualsense_bt_cmd cmd = { 0 };
			cmd.report_id = DUALSENSE_REPORT_BLUETOOTH_08C;
			uint8_t* data_ptr = cmd.data;
			memcpy(data_ptr, &packet_0x11, packet_0x11.length_prefixed.length + 2);
			data_ptr += packet_0x11.length_prefixed.length + 2;
			memcpy(data_ptr, &packet_0x12, packet_0x12.length_prefixed.length + 2);
			cmd.checksum = titania_calc_checksum(crc_seed_output, (uint8_t*) &cmd, sizeof(cmd) - 4);
#ifdef __APPLE__
			IOHIDDeviceSetReportWithCallback(*(IOHIDDeviceRef*) state[handle].hid,
									   kIOHIDReportTypeOutput,
									   DUALSENSE_REPORT_BLUETOOTH_08C,
									   (uint8_t*) &cmd, sizeof(cmd), 8, stub, (void*) (intptr_t) handle);
#else
			hid_write(state[handle].hid, (uint8_t*) &cmd, sizeof(cmd)); // this takes 50ms on macOS??
#endif
		}

		titania_timer_next(handle);
	}

	atomic_store(&state[handle].haptics.bt_timer.running, false);
	state[handle].haptics.bt_timer.frame = 0;
}
