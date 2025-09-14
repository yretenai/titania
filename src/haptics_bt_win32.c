//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"
#include "titania.h"

#include <windows.h>

void titania_timer_next(titania_timer* t) {
	// todo
}

DWORD WINAPI titania_bt_loop(void* arg) {
	titania_bt_loop_inner((titania_handle) (intptr_t) arg);
	return 0;
}

bool titania_thread_create(const titania_handle handle) {
	if (atomic_load(&state[handle].haptics.bt_timer.running) == true) {
		return true;
	}
	titania_timer* t = &state[handle].haptics.bt_timer;
	atomic_store(&t->running, true);
	t->thread = CreateThread(nullptr, 0, titania_bt_loop, (void*) (intptr_t) handle, 0, nullptr);
	return t->thread != nullptr;
}

void titania_thread_join(const titania_handle handle) {
	if (atomic_load(&state[handle].haptics.bt_timer.running) == false) {
		return;
	}

	titania_timer* t = &state[handle].haptics.bt_timer;
	atomic_store(&t->running, false);
	WaitForSingleObject(t->thread, INFINITE);
	CloseHandle(t);
}

titania_error titania_haptics_init_bt(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_NOT_ACCESS(handle);

	titania_thread_create(handle);

	return TITANIA_ERROR_OK;
}

titania_error titania_haptics_close_bt(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_NOT_ACCESS(handle);

	titania_thread_join(handle);

	return TITANIA_ERROR_OK;
}

titania_error titania_haptics_flush_bt(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_NOT_ACCESS(handle);

	titania_thread_join(handle);
	state[handle].haptics.read_offset = 0;
	memset(state[handle].haptics.buffer, 0, sizeof(state[handle].haptics.buffer));
	titania_thread_join(handle);

	return TITANIA_ERROR_OK;
}
