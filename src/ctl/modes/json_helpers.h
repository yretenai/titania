//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIACTL_JSON_HELPERS_H
#define TITANIACTL_JSON_HELPERS_H

#include <json.h>

// safe wrappers and conversions

static inline const char* titania_json_object_get_string(const struct json* obj, const char* key, const char* default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	const char* str = json_string_get(value);
	if (str == nullptr) {
		return default_value;
	}
	return str;
}

static inline uint32_t titania_json_object_get_enum(const struct json* obj, const char* key, const char* const* const values, const uint32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
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

	uint32_t temp;
	if (sscanf(value->string, "%u", &temp) == 0) {
		return default_value;
	}
	return temp;
}

static inline double titania_json_object_get_float(const struct json* obj, const char* key, const double default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (obj->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline int32_t titania_json_object_get_int32(const struct json* obj, const char* key, const int32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (obj->type == JSON_STRING) {
		int32_t temp;
		if (sscanf(value->string, "%d", &temp) == 0) {
			return default_value;
		}
		return temp;
	}

	if (obj->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline int64_t titania_json_object_get_int64(const struct json* obj, const char* key, const int64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (obj->type == JSON_STRING) {
		long long temp;
		if (sscanf(value->string, "%lld", &temp) == 0) {
			return default_value;
		}
		return (int64_t) temp;
	}

	if (obj->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline uint32_t titania_json_object_get_uint32(const struct json* obj, const char* key, const uint32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		uint32_t temp;
		if (sscanf(value->string, "%u", &temp) == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline uint64_t titania_json_object_get_uint64(const struct json* obj, const char* key, const uint64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		unsigned long long temp;
		if (sscanf(value->string, "%llu", &temp) == 0) {
			return default_value;
		}
		return (uint64_t) temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline double titania_json_array_get_float(const struct json* obj, const int index, const double default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_array_get(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline int32_t titania_json_array_get_int32(const struct json* obj, const int index, const int32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_array_get(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		int32_t temp;
		if (sscanf(value->string, "%d", &temp) == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline int64_t titania_json_array_get_int64(const struct json* obj, const int index, const int64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_array_get(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		long long temp;
		if (sscanf(value->string, "%lld", &temp) == 0) {
			return default_value;
		}
		return (int64_t) temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline uint32_t titania_json_array_get_uint32(const struct json* obj, const int index, const uint32_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_array_get(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		uint32_t temp;
		if (sscanf(value->string, "%u", &temp) == 0) {
			return default_value;
		}
		return temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline uint64_t titania_json_array_get_uint64(const struct json* obj, const int index, const uint64_t default_value) {
	if (obj == nullptr) {
		return default_value;
	}

	const struct json* value = json_array_get(obj, index);
	if (value == nullptr) {
		return default_value;
	}

	if (value->type == JSON_STRING) {
		unsigned long long temp;
		if (sscanf(value->string, "%llu", &temp) == 0) {
			return default_value;
		}
		return (uint64_t) temp;
	}

	if (value->type != JSON_NUMBER) {
		return default_value;
	}

	return value->num;
}

static inline bool titania_json_object_get_bool(const struct json* obj, const char* key) {
	if (obj == nullptr) {
		return false;
	}

	const struct json* value = json_object_get(obj, key);
	if (value == nullptr) {
		return false;
	}

	return value->type != JSON_FALSE;
}

#endif
