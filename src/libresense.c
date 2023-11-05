#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include <assert.h>

#include <hidapi/hidapi.h>

#include "../include/libresense.h"
#include "config.h"

#ifdef MSVC
	#define PACKED
	#define enum_u8 enum : uint8_t
	#define dense_struct struct
#else
	#define PACKED __attribute__((__packed__))
	#define enum_u8 enum PACKED
	#define dense_struct struct PACKED
	#define static_assert _Static_assert
#endif

typedef enum_u8 {
	DUALSENSE_USB_TX = 0x1,
	DUALSENSE_USB_RX = 0x2,
	DUALSENSE_BT_TX = 0x31,
	DUALSENSE_BT_RX = 0x31,
} dualsense_mode;
static_assert(sizeof(dualsense_mode) == 1, "Dualsense Mode is not 1 byte");

typedef struct {
	dualsense_mode mode;
	uint8_t flags;
} dualsense_msg;

typedef struct {
	dualsense_msg msg;
	uint32_t checksuma;
	uint32_t checksumb;
} dualsense_msg_ex;

typedef struct {
	hid_device* hid;
	bool bt_initialized;
	dualsense_msg_ex msg;
} dualsense_state;

static dualsense_state state[LIBRESENSE_MAX_CONTROLLERS];
bool is_initialized = false;

#define CHECK_INIT if(!is_initialized) return ELIBRESENSE_NOT_INITIALIZED
#define CHECK_HANDLE(h) if(h < 0 || h > LIBRESENSE_MAX_CONTROLLERS) return ELIBRESENSE_INVALID_HANDLE
#define CHECK_HANDLE_VALID(h) \
	CHECK_HANDLE(h) \
	if(state[h].hid == NULL) return ELIBRESENSE_INVALID_HANDLE

void libresense_init(void) {
	if (is_initialized) {
		return;
	}

	if (hid_init() != 0) {
		return;
	}

	is_initialized = true;
}

libresense_result libresense_open(int id, libresense_handle *handle) {
	CHECK_INIT;
	CHECK_HANDLE(id);

	// already allocated?
	if(state[id].hid != NULL) {
		*handle = id;
		return ELIBRESENSE_OK;
	}

	return ELIBRESENSE_NOT_IMPLEMENTED;
}

libresense_result libresense_close(libresense_handle handle) {
	CHECK_INIT;
	CHECK_HANDLE(handle);

	state[handle].hid = (hid_close(state[handle].hid), NULL);

	return ELIBRESENSE_OK;
}

void libresense_exit(void) {
	if (!is_initialized) {
		return;
	}

	for (int i = 0; i < LIBRESENSE_MAX_CONTROLLERS; i++) {
		libresense_close(i);
	}

	is_initialized = false;
}
