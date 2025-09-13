//  titania project
//  https://git.sr.ht/~chronovore/titania
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
	TITANIA_UNICODE_OK = 0,
	TITANIA_UNICODE_EMPTY,
	TITANIA_UNICODE_EXPECTED_CONTINUATION_CHAR,
	TITANIA_UNICODE_EXPECTED_REGULAR_CHAR,
	TITANIA_UNICODE_EXPECTED_SURROGATE_HIGH,
	TITANIA_UNICODE_EXPECTED_SURROGATE_LOW,
	TITANIA_UNICODE_MALFORMED,
	TITANIA_UNICODE_OUT_OF_SPACE
} titania_unicode_error;

typedef struct titania_unicode_result {
	titania_unicode_error error;
	size_t size;
} titania_unicode_result;

titania_unicode_result titania_utf8_to_utf32(const titania_char8* utf8, size_t utf8_size, titania_char32* utf32, size_t utf32_size);

titania_unicode_result titania_utf16_to_utf32(const titania_char16* utf16, size_t utf16_size, titania_char32* utf32, size_t utf32_size);

titania_unicode_result titania_utf32_to_utf8(const titania_char32* utf32, size_t utf32_size, titania_char8* utf8, size_t utf8_size);

titania_unicode_result titania_utf32_to_utf16(const titania_char32* utf32, size_t utf32_size, titania_char16* utf16, size_t utf16_size);

#endif
