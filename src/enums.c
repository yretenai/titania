#include "structures.h"

const char *libresense_error_msg[LIBRESENSE_ERROR_MAX + 1] = {
	"ok", "not initialized", "invalid library", "invalid handle", "invalid data", "hidapi failure", "out of range", "not implemented", "no available slots", NULL
};
const char *libresense_battery_state_msg[LIBRESENSE_BATTERY_MAX + 1] = { "unknown", "discharging", "charging", "full", NULL };
const char *libresense_edge_profile_id_msg[LIBRESENSE_PROFILE_MAX + 1] = { "triangle", "cross", "square", "circle", NULL };
const char *libresense_version_msg[LIBRESENSE_VERSION_MAX + 1] = {
	"unknown0", "unknown1", "unknown2", "unknown3", "unknown4", "unknown5", "firmware", "unknown7", "unknown8", "unknown9", NULL
};
