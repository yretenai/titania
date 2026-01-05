//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include "nanosleep.h"
#include "structures.h"
#include "titania.h"

#include <time.h>
#include <string.h>

void titania_timer_next(const titania_handle handle) {
	titania_timer* t = &state[handle].haptics.bt_timer;

#ifndef __APPLE__
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t->next, NULL);
#else
	struct timespec now, rel;
	clock_gettime(CLOCK_MONOTONIC, &now);

	rel.tv_sec = t->next.tv_sec - now.tv_sec;
	rel.tv_nsec = t->next.tv_nsec - now.tv_nsec;
	if (rel.tv_nsec < 0) {
		rel.tv_nsec += 1000000000L;
		rel.tv_sec--;
	}

	if (rel.tv_sec < 0) {
		return; // took too long.
	}

	if (rel.tv_sec >= 0 || rel.tv_nsec >= 0) {
		nanosleep(&rel, nullptr);
	}
#endif

	clock_gettime(CLOCK_MONOTONIC, &state[handle].haptics.bt_timer.next);
	t->next.tv_nsec += t->ticks;
	while (t->next.tv_nsec >= 1000000000L) {
		t->next.tv_nsec -= 1000000000L;
		t->next.tv_sec++;
	}
}

void* titania_bt_loop(void* arg) {
	titania_bt_loop_inner((titania_handle) (intptr_t) arg);
	return nullptr;
}

bool titania_thread_create(const titania_handle handle) {
	if (atomic_load(&state[handle].haptics.bt_timer.running) == true) {
		return true;
	}

	titania_timer* t = &state[handle].haptics.bt_timer;
	atomic_store(&t->running, true);
	return pthread_create(&t->thread, nullptr, titania_bt_loop, (void*) (intptr_t) handle) == 0;
}

void titania_thread_join(const titania_handle handle) {
	if (atomic_load(&state[handle].haptics.bt_timer.running) == false) {
		return;
	}

	titania_timer* t = &state[handle].haptics.bt_timer;
	atomic_store(&t->running, false);

	pthread_join(t->thread, nullptr);
}

titania_error titania_haptics_init_bt(const titania_handle handle) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_NOT_ACCESS(handle);

	clock_gettime(CLOCK_MONOTONIC, &state[handle].haptics.bt_timer.next);
	state[handle].haptics.bt_timer.ticks = 1000000000UL * DUALSENSE_SAMPLE_SIZE / (DUALSENSE_SAMPLE_RATE * 2);
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
	clock_gettime(CLOCK_MONOTONIC, &state[handle].haptics.bt_timer.next);
	titania_thread_create(handle);

	return TITANIA_ERROR_OK;
}
