//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIA_UNICODE_H
#define TITANIA_UNICODE_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t titania_char32;
typedef uint16_t titania_char16;
typedef uint8_t titania_char8;

typedef enum titania_unicode_error {
	TITANIA_UNICODE_EMPTY = 0,
	TITANIA_UNICODE_EXPECTED_CONTINUATION_CHAR = -1,
	TITANIA_UNICODE_EXPECTED_REGULAR_CHAR = -2,
	TITANIA_UNICODE_EXPECTED_SURROGATE_HIGH = -3,
	TITANIA_UNICODE_EXPECTED_SURROGATE_LOW = -4,
	TITANIA_UNICODE_MALFORMED = -5,
	TITANIA_UNICODE_OUT_OF_SPACE = -6
} titania_unicode_error;

typedef union titania_unicode_result {
	titania_unicode_error error;
	size_t size;
} titania_unicode_result;

titania_unicode_result titania_utf8_to_utf32(const titania_char8* utf8, const size_t utf8_size, titania_char32* utf32, const size_t utf32_size);

titania_unicode_result titania_utf16_to_utf32(const titania_char16* utf16, const size_t utf16_size, titania_char32* utf32, const size_t utf32_size);

titania_unicode_result titania_utf32_to_utf8(const titania_char32* utf32, const size_t utf32_size, titania_char8* utf8, const size_t utf8_size);

titania_unicode_result titania_utf32_to_utf16(const titania_char32* utf32, const size_t utf32_size, titania_char16* utf16, const size_t utf16_size);

#endif
