/*
 * Copyright 2025 Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/APRSlib *
 *
 * This is based on other projects:
 *    VP-Digi: https://github.com/sq8vps/vp-digi
 *    ESP32APRS: https://github.com/nakhonthai/ESP32APRS_Audio
 *    LibAPRS: https://github.com/markqvist/LibAPRS
 *
 *    please contact their authors for more information.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef FX25_H_
#define FX25_H_

#include <stdbool.h>
#include <stdint.h>

#define FX25_MAX_BLOCK_SIZE 255

typedef struct fx25_mode_s {
    uint64_t correlation_tag;
    uint16_t data_size;
    uint8_t parity_check_size;
} fx25_mode_t;

extern const fx25_mode_t fx25_mode_list[11];

/**
 * @brief Get FX.25 mode for given correlation tag
 * @param tag FX.25 correlation tag
 * @return FX.25 mode structure pointer or NULL if not a FX.25 tag
 */
const fx25_mode_t *fx25_get_mode_for_tag(uint64_t tag);

/**
 * @brief Get FX.25 mode for given payload size
 * @param size Payload size including flags and CRC
 * @return FX.25 mode structure pointer or NULL if standard AX.25 must be used
 */
const fx25_mode_t *fx25_get_mode_for_size(uint16_t size);

/**
 * @brief Encode AX.25 message in FX.25
 * @param *buffer AX.25 message (bit-stuffed, with CRC and padding)
 * @param *mode FX.25 mode
 */
void fx25_encode(uint8_t *buffer, const fx25_mode_t *mode);

/**
 * @brief Decode/fix FX.25 packet
 * @param *buffer Input buffer
 * @param *mode FX.25 mode
 * @param *fixed Number of bytes fixed
 * @return True if message is valid, false if uncorrectable
 */
bool fx25_decode(uint8_t *buffer, const fx25_mode_t *mode, uint8_t *fixed);

/**
 * @brief Initialize FX.25 module
 */
void fx25_init(void);

#endif /* FX25_H_ */
