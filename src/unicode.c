//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "unicode.h"

#define TEST_CONTINUATION_CHAR(t, buf, n) \
	if (++buf >= endp) { \
		return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE }; \
	} \
	const t n = *buf; \
	if ((n & 0xC0) != 0x80 || n == 0) { \
		return (titania_unicode_result) { .error = TITANIA_UNICODE_EXPECTED_CONTINUATION_CHAR }; \
	}

titania_unicode_result titania_utf8_to_utf32(const titania_char8* utf8, const size_t utf8_size, titania_char32* utf32, const size_t utf32_size) {
	if (utf8 == nullptr || utf32 == nullptr) {
		return (titania_unicode_result) { .error = TITANIA_UNICODE_MALFORMED };
	}

	size_t size = 0;
	const titania_char8* endp = utf8 + utf8_size;
	do {
		const titania_char8 char0 = *utf8;
		if ((char0 & 0x80) == 0) { // ASCII, Non-Extended
			utf32[size++] = char0;
			continue;
		}

		if ((char0 & 0xE0) == 0xC0) { // UTF-8 Continuation 1
			TEST_CONTINUATION_CHAR(titania_char8, utf8, char1);

			utf32[size++] = (char0 & 0x1F) << 6 | (char1 & 0x3F);
			continue;
		}

		if ((char0 & 0xF0) == 0xE0) { // UTF-8 Continuation 2
			TEST_CONTINUATION_CHAR(titania_char8, utf8, char1);
			TEST_CONTINUATION_CHAR(titania_char8, utf8, char2);

			utf32[size++] = (char0 & 0xF) << 12 | ((char1 & 0x3F) << 6) | (char2 & 0x3F);
			continue;
		}

		if ((char0 & 0xF8) == 0xF0) { // UTF-8 Continuation 3
			TEST_CONTINUATION_CHAR(titania_char8, utf8, char1);
			TEST_CONTINUATION_CHAR(titania_char8, utf8, char2);
			TEST_CONTINUATION_CHAR(titania_char8, utf8, char3);

			utf32[size++] = (char0 & 0x7) << 12 | ((char1 & 0x3F) << 12) | ((char2 & 0x3F) << 6) | (char3 & 0x3F);
			continue;
		}

		return (titania_unicode_result) { .error = TITANIA_UNICODE_EXPECTED_REGULAR_CHAR };
	} while (*utf8++ != 0 && size < utf32_size && utf8 < endp);

	if (size + 1 >= utf32_size) { // + 1 for null byte
		return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
	}

	utf32[size++] = 0;

	return (titania_unicode_result) { .size = size };
}

titania_unicode_result titania_utf16_to_utf32(const titania_char16* utf16, const size_t utf8_size, titania_char32* utf32, const size_t utf32_size) {
	size_t size = 0;
	const titania_char16* endp = utf16 + utf8_size;
	do {
		const titania_char16 char0 = *utf16;
		if (char0 < 0xD800 || char0 > 0xDFFF) { // Non-surrogate
			utf32[size++] = char0;
			continue;
		}

		if (char0 > 0xDBFF) {
			return (titania_unicode_result) { .error = TITANIA_UNICODE_EXPECTED_SURROGATE_HIGH };
		}

		if (++utf16 >= endp) {
			return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
		}

		const titania_char16 char1 = *++utf16;
		if (char0 < 0xDC00 || char0 > 0xDFFF) {
			return (titania_unicode_result) { .error = TITANIA_UNICODE_EXPECTED_SURROGATE_LOW };
		}

		utf32[size++] = ((char0 & 0x3FF) << 10 | (char1 & 0x3FF)) + 0x10000;
	} while (*utf16++ != 0 && size < utf32_size && utf16 < endp);

	if (size + 1 >= utf32_size) { // + 1 for null byte
		return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
	}

	utf32[size++] = 0;

	return (titania_unicode_result) { .size = size };
}

titania_unicode_result titania_utf32_to_utf8(const titania_char32* utf32, const size_t utf32_size, titania_char8* utf8, const size_t utf8_size) {
	size_t size = 0;
	const titania_char32* endp = utf32 + utf32_size;
	do {
		const titania_char32 char0 = *utf32;

		if (char0 < 0x80) {
			utf8[size++] = char0;
			continue;
		}

		if (char0 < 0x800) {
			if (size + 2 >= utf8_size) {
				return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
			}

			utf8[size++] = 0xC0 | (char0 >> 6);
			utf8[size++] = 0x80 | (char0 & 0x3F);
			continue;
		}

		if (char0 < 0x10000) {
			if (size + 3 >= utf8_size) {
				return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
			}

			utf8[size++] = 0xC0 | (char0 >> 12);
			utf8[size++] = 0x80 | (char0 >> 6 & 0x3F);
			utf8[size++] = 0x80 | (char0 & 0x3F);
			continue;
		}

		if (char0 < 0x110000) {
			if (size + 4 >= utf8_size) {
				return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
			}

			utf8[size++] = 0xF0 | (char0 >> 18);
			utf8[size++] = 0x80 | (char0 >> 12 & 0x3F);
			utf8[size++] = 0x80 | (char0 >> 6 & 0x3F);
			utf8[size++] = 0x80 | (char0 & 0x3F);
			continue;
		}

		return (titania_unicode_result) { .error = TITANIA_UNICODE_EXPECTED_REGULAR_CHAR };
	} while (*utf32++ != 0 && size < utf8_size && utf32 < endp);

	if (size + 1 >= utf8_size) { // + 1 for null byte
		return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
	}

	utf8[size++] = 0;

	return (titania_unicode_result) { .size = size };
}

titania_unicode_result titania_utf32_to_utf16(const titania_char32* utf32, const size_t utf32_size, titania_char16* utf16, const size_t utf16_size) {
	size_t size = 0;
	const titania_char32* endp = utf32 + utf32_size;
	do {
		if (size >= utf16_size - 1) {
			break;
		}

		titania_char32 char0 = *utf32;

		if (char0 <= 0xFFFF) { // Non-Surrogate
			if (char0 >= 0xD800 && char0 <= 0xDFFF) { // Surrogate bytes are not allowed in UTF-32
				return (titania_unicode_result) { .error = TITANIA_UNICODE_MALFORMED };
			}

			utf16[size++] = char0;
			continue;
		}

		if (char0 < 0x110000) { // Unicode limit
			if (size + 2 >= utf16_size) {
				return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
			}

			char0 -= 0x10000;
			utf16[size++] = (char0 >> 10) + 0xD800;
			utf16[size++] = (char0 & 0x3FF) + 0xDC00;
			continue;
		}

		return (titania_unicode_result) { .error = TITANIA_UNICODE_EXPECTED_REGULAR_CHAR };
	} while (*utf32++ != 0 && utf32 < endp);

	if (size >= utf16_size) { // + 1 for null byte
		return (titania_unicode_result) { .error = TITANIA_UNICODE_OUT_OF_SPACE };
	}

	utf16[size++] = 0;

	return (titania_unicode_result) { .size = size };
}
