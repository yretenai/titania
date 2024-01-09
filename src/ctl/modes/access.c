//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <json.h>

#include "../titaniactl.h"

titaniactl_error titaniactl_mode_access_import(titania_profile_id profile, const struct json* data, titania_hid handle) { return TITANIACTL_NOT_IMPLEMENTED; }

struct json* titaniactl_mode_access_convert(const titania_access_profile profile, const bool include_success) { return nullptr; }

titaniactl_error titaniactl_mode_access_export(titania_profile_id profile, const char* path, titania_hid handle) { return TITANIACTL_NOT_IMPLEMENTED; }

titaniactl_error titaniactl_mode_access_delete(titania_profile_id profile, titania_hid handle) { return TITANIACTL_NOT_IMPLEMENTED; }
