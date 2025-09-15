//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#ifndef TITANIACTL_JSON_HELPERS_H
#define TITANIACTL_JSON_HELPERS_H

#include <json.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// retroported functions

static inline void json_object_add_string(struct json* obj, const char* key, const char* value) { json_set(obj, key, json_string(value)); }

static inline void json_object_add_bool(struct json* obj, const char* key, const bool value) { json_set(obj, key, json_bool(value)); }

static inline void json_object_add_number(struct json* obj, const char* key, const double num) { json_set(obj, key, json_number(num)); }

static inline struct json* json_object_add_object(struct json* obj, const char* key) { return json_set(obj, key, json_object()); }

static inline struct json* json_object_add_array(struct json* obj, const char* key) { return json_set(obj, key, json_array()); }

static inline void json_array_add_number(struct json* obj, const double num) { json_append(obj, json_number(num)); }

static inline struct json* json_array_add_object(struct json* obj) {
	struct json* arr = json_object();
	json_append(obj, arr);
	return arr;
}

static inline struct json* json_array_add_array(struct json* obj) {
	struct json* arr = json_array();
	json_append(obj, arr);
	return arr;
}

// safe wrappers and conversions

static inline const char* titania_json_object_get_string(struct json* obj, const char* key, const char* default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	const char* str = json_string_get(value);
	if (str == nullptr) {
		return default_value;
	}
	return str;
}

static inline uint32_t titania_json_object_get_enum(struct json* obj, const char* key, const char* const* const values, const uint32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	const char* str = json_string_get(value);
	if (str == nullptr) {
		return default_value;
	}

	for (int i = 0; values[i] != nullptr; ++i) {
		if (strcmp(values[i], str) == 0) {
			return i;
		}
	}

	const uint32_t temp = strtoul(str, nullptr, 16);
	if (temp == 0) {
		return default_value;
	}
	return temp;
}

static inline double titania_json_object_get_float(struct json* obj, const char* key, const double default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (obj->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline int32_t titania_json_object_get_int32(struct json* obj, const char* key, const int32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (obj->type == JSON_STRING) {
		const int32_t temp = (int32_t) strtol(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (obj->type != JSON_NUMBER) {
		return default_value;
	}

	return (int32_t) value->num;
}

static inline int64_t titania_json_object_get_int64(struct json* obj, const char* key, const int64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (obj->type == JSON_STRING) {
		const int64_t temp = strtoll(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (obj->type != JSON_NUMBER) {
		return default_value;
	}

	return (int64_t) value->num;
}

static inline uint32_t titania_json_object_get_uint32(struct json* obj, const char* key, const uint32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		const uint32_t temp = strtoul(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return (uint32_t) value->num;
}

static inline uint64_t titania_json_object_get_uint64(struct json* obj, const char* key, const uint64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		const uint64_t temp = strtoull(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return (uint64_t) value->num;
}

static inline double titania_json_array_get_float(struct json* obj, const int index, const double default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_at(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline int32_t titania_json_array_get_int32(struct json* obj, const int index, const int32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_at(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		const int32_t temp = (int32_t) strtol(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return (int32_t) value->num;
}

static inline int64_t titania_json_array_get_int64(struct json* obj, const int index, const int64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_at(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		const int64_t temp = strtoll(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return (int64_t) value->num;
}

static inline uint32_t titania_json_array_get_uint32(struct json* obj, const int index, const uint32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_at(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		const uint32_t temp = strtoul(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return (uint32_t) value->num;
}

static inline uint64_t titania_json_array_get_uint64(struct json* obj, const int index, const uint64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_at(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		const uint64_t temp = strtoull(value->string, nullptr, 10);
		if (temp == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return (uint64_t) value->num;
}

static inline bool titania_json_object_get_bool(struct json* obj, const char* key) {
	if (obj == nullptr) {
		return false;
	}

	const struct json* value = json_get(obj, key);
	if (value == nullptr || value->type != JSON_BOOL) {
		return false;
	}

	return value->boolean;
}

static inline void titania_json_object_add_uint64(struct json* obj, const char* key, const uint64_t value) {
	if (obj == nullptr || key == nullptr) {
		return;
	}

	char strbuffer[64] = { 0 };
	sprintf(strbuffer, "%llx", (unsigned long long) value);
	json_object_add_string(obj, key, strbuffer);
}

#endif
