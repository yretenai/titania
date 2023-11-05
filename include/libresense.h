#pragma once

#ifndef LIBRESENSE_H
#define LIBRESENSE_H

typedef enum {
	ELIBRESENSE_OK = 0,
	ELIBRESENSE_NOT_INITIALIZED = 1,
	ELIBRESENSE_INVALID_HANDLE = 2,
	ELIBRESENSE_OUT_OF_RANGE = 3,
	ELIBRESENSE_NOT_IMPLEMENTED = 4,
	ELIBRESENSE_ERROR_MAX
} libresense_result;

static const char* libresense_error_msg[ELIBRESENSE_ERROR_MAX] = {
	"ok",
	"not initialized",
	"invalid handle",
	"out of range",
	"not implemented"
};

typedef signed int libresense_handle;

void libresense_init(void);
libresense_result libresense_open(int id, libresense_handle *handle);
libresense_result libresense_close(libresense_handle handle);
void libresense_exit(void);

#endif
