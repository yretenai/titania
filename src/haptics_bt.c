//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"
#include "titania.h"

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

titania_error titania_haptics_init_bt(const titania_handle handle) { return TITANIA_ERROR_OK; }

titania_error titania_haptics_close_bt(const titania_handle handle) { return TITANIA_ERROR_OK; }

titania_error titania_haptics_flush_bt(const titania_handle handle) { return TITANIA_ERROR_OK; }
