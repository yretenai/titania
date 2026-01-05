#include <titania_config_internal.h>

#define _POSIX_C_SOURCE 200809L

#ifndef TITANIA_HAS_NANOSLEEP
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void nanosleep(const struct timespec* tspec, void* nullvoid) {
	HANDLE timer;
	LARGE_INTEGER ft;
	ft.QuadPart = -(10000 * tspec->tv_nsec);
	timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#else
#error "no nanosleep"
#endif
#else
#include <time.h>
#endif

#ifndef TITANIA_HAS_UNISTD_USECONDS
#include <stdint.h>
typedef uint32_t useconds_t;
#else
#include <unistd.h>
#endif
