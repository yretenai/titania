//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"
#include "titania.h"

titania_error titania_haptics_copy_samples(const titania_handle handle, float* samples, size_t* num_frames) {
	const size_t write = state[handle].haptics.write_offset;
	const size_t read = state[handle].haptics.read_offset;
	if (read > write) {
		state[handle].haptics.read_offset = write;
		return TITANIA_ERROR_OK;
	}

	if (read == write) {
		return TITANIA_ERROR_OK;
	}

	size_t remain = (TITANIA_MAXIMUM_HAPTICS_SIZE - ((write - read) % TITANIA_MAXIMUM_HAPTICS_SIZE)) >> 1;

	const size_t avail = *num_frames;
	if (remain > avail) {
		remain = avail;
	}

	if (remain < TITANIA_MINIMUM_HAPTICS_SIZE >> 1) {
		return TITANIA_ERROR_HAPTICS_NO_AUDIO;
	}

	for (size_t sample_idx = 0; sample_idx < remain; ++sample_idx) {
		size_t x = (read + (sample_idx << 1)) % TITANIA_MAXIMUM_HAPTICS_SIZE;
		float y = state[handle].haptics.buffer[x];
		samples[(sample_idx << 2) + 2] = state[handle].haptics.buffer[(read + (sample_idx << 1)) % TITANIA_MAXIMUM_HAPTICS_SIZE];
		samples[(sample_idx << 2) + 3] = state[handle].haptics.buffer[(read + (sample_idx << 1) + 1) % TITANIA_MAXIMUM_HAPTICS_SIZE];
	}

	*num_frames = remain;

	state[handle].haptics.read_offset += (remain << 1);

	return TITANIA_ERROR_OK;
}

#ifdef TITANIA_HAS_HAPTICS_FALLBACK
titania_error titania_haptics_init(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return TITANIA_ERROR_OK;
	}

	if (state[handle].hid_info.is_bluetooth) {
		return titania_haptics_init_bt(handle);
	}

	return TITANIA_ERROR_OK;
}

titania_error titania_haptics_close(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return TITANIA_ERROR_OK;
	}

	if (state[handle].hid_info.is_bluetooth) {
		return titania_haptics_close_bt(handle);
	}

	return TITANIA_ERROR_OK;
}

titania_error titania_haptics_flush(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);

	if (IS_ACCESS(state[handle].hid_info)) {
		return TITANIA_ERROR_OK;
	}

	if (state[handle].hid_info.is_bluetooth) {
		return titania_haptics_flush_bt(handle);
	}

	return TITANIA_ERROR_OK;
}
#endif

#ifdef TITANIA_HAS_HAPTICS_DISABLE_BT
titania_error titania_haptics_init_bt(const titania_handle handle) {
	return TITANIA_ERROR_NOT_IMPLEMENTED;
}

titania_error titania_haptics_close_bt(const titania_handle handle) {
	return TITANIA_ERROR_NOT_IMPLEMENTED;
}

titania_error titania_haptics_flush_bt(const titania_handle handle) {
	return TITANIA_ERROR_NOT_IMPLEMENTED;
}
#endif
