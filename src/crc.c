//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"

uint32_t crc_table[256];

uint32_t crc_seed_input = 0;
uint32_t crc_seed_output = 0;
uint32_t crc_seed_feature = 0;
uint32_t crc_seed_feature_profile = 0;
uint32_t crc_seed_titania = 0;

uint32_t checksum(uint32_t crc, const uint8_t* buffer, const size_t size) {
	for (size_t i = 0; i < size; ++i) {
		crc = crc_table[((uint8_t) crc ^ buffer[i])] ^ crc >> 8;
	}
	return crc;
}

void titania_init_checksum(void) {
	for (int i = 1; i < 256; ++i) {
		uint32_t crc = i;
		for (int j = 8; j; --j) {
			crc = crc & 1 ? crc >> 1 ^ 0xedb88320 : crc >> 1;
		}
		crc_table[i] = crc;
	}

	crc_seed_input = DUALSENSE_CRC_INPUT;
	crc_seed_input = checksum(UINT32_MAX, (uint8_t*) &crc_seed_input, 1);
	crc_seed_output = DUALSENSE_CRC_OUTPUT;
	crc_seed_output = checksum(UINT32_MAX, (uint8_t*) &crc_seed_output, 1);
	crc_seed_feature = DUALSENSE_CRC_FEATURE;
	crc_seed_feature = checksum(UINT32_MAX, (uint8_t*) &crc_seed_feature, 1);
	crc_seed_feature_profile = DUALSENSE_CRC_FEATURE_EDGE;
	crc_seed_feature_profile = checksum(UINT32_MAX, (uint8_t*) &crc_seed_feature_profile, 1);
	crc_seed_titania = TITANIA_CRC;
	crc_seed_titania = checksum(UINT32_MAX, (uint8_t*) &crc_seed_titania, 4);
}

uint32_t titania_calc_checksum(const uint32_t state, const uint8_t* buffer, const size_t size) { return ~checksum(state, buffer, size); }
