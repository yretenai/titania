#include "structures.h"

const char *libresense_error_msg[ELIBRESENSE_ERROR_MAX + 1] = { "ok", "not initialized", "invalid handle", "invalid data", "out of range", "not implemented", "no available slots", NULL };
const char *libresense_battery_state_msg[ELIBRESENSE_BATTERY_MAX + 1] = { "unknown", "discharging", "charging", "full", NULL };
const char *libresense_edge_profile_id_msg[ELIBRESENSE_PROFILE_MAX + 1] = { "triangle", "cross", "square", "circle", NULL };
