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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fx25.h"
#include "rs.h"

#define FX25_RS_FCR 1
#define FX25_PREGENERATE_POLYS
#define FX25_MAX_DISTANCE 10 // maximum Hamming distance when comparing tags

const fx25_mode_t fx25_mode_list[11] = {
    { .correlation_tag = 0xB74DB7DF8A532F3E, .data_size = 239, .parity_check_size = 16 }, //
    { .correlation_tag = 0x26FF60A600CC8FDE, .data_size = 128, .parity_check_size = 16 }, //
    { .correlation_tag = 0xC7DC0508F3D9B09E, .data_size = 64, .parity_check_size = 16 },  //
    { .correlation_tag = 0x8F056EB4369660EE, .data_size = 32, .parity_check_size = 16 },  //
    { .correlation_tag = 0x6E260B1AC5835FAE, .data_size = 223, .parity_check_size = 32 }, //
    { .correlation_tag = 0xFF94DC634F1CFF4E, .data_size = 128, .parity_check_size = 32 }, //
    { .correlation_tag = 0x1EB7B9CDBC09C00E, .data_size = 64, .parity_check_size = 32 },  //
    { .correlation_tag = 0xDBF869BD2DBB1776, .data_size = 32, .parity_check_size = 32 },  //
    { .correlation_tag = 0x3ADB0C13DEAE2836, .data_size = 191, .parity_check_size = 64 }, //
    { .correlation_tag = 0xAB69DB6A543188D6, .data_size = 128, .parity_check_size = 64 }, //
    { .correlation_tag = 0x4A4ABEC4A724B796, .data_size = 64, .parity_check_size = 64 }   //
};

const fx25_mode_t *fx25_get_mode_for_tag(uint64_t tag) {
    for (uint8_t i = 0; i < sizeof(fx25_mode_list) / sizeof(*fx25_mode_list); i++) {
        if (__builtin_popcountll(tag ^ fx25_mode_list[i].correlation_tag) <= FX25_MAX_DISTANCE)
            return &fx25_mode_list[i];
    }
    return NULL;
}

const fx25_mode_t *fx25_get_mode_for_size(uint16_t size) {
    // use "UZ7HO Soundmodem standard" for choosing FX.25 mode
    if (size <= 32)
        return &fx25_mode_list[3];
    else if (size <= 64)
        return &fx25_mode_list[2];
    else if (size <= 128)
        return &fx25_mode_list[5];
    else if (size <= 191)
        return &fx25_mode_list[8];
    else if (size <= 223)
        return &fx25_mode_list[4];
    else if (size <= 239)
        return &fx25_mode_list[0];
    else
        return NULL; // frame too big, do not use FX.25
}

#ifdef FX25_PREGENERATE_POLYS
static lwfecrs_t rs16, rs32, rs64;
#else
static struct LwFecRS rs;
#endif

void fx25_encode(uint8_t *buffer, const fx25_mode_t *mode) {
#ifdef FX25_PREGENERATE_POLYS
    lwfecrs_t *rs = NULL;
    switch (mode->parity_check_size) {
        case 16:
            rs = &rs16;
            break;
        case 32:
            rs = &rs32;
            break;
        case 64:
            rs = &rs64;
            break;
        default:
            rs = &rs16;
            break;
    }
    RsEncode(rs, buffer, mode->data_size);
#else
    RsInit(&rs, mode->T, FX25_RS_FCR);
    RsEncode(&rs, buffer, mode->K);
#endif
}

bool fx25_decode(uint8_t *buffer, const fx25_mode_t *mode, uint8_t *fixed) {
#ifdef FX25_PREGENERATE_POLYS
    lwfecrs_t *rs = NULL;
    switch (mode->parity_check_size) {
        case 16:
            rs = &rs16;
            break;
        case 32:
            rs = &rs32;
            break;
        case 64:
            rs = &rs64;
            break;
        default:
            rs = &rs16;
            break;
    }
    return RsDecode(rs, buffer, mode->data_size, fixed);
#else
    RsInit(&rs, mode->T, FX25_RS_FCR);
    return RsDecode(&rs, buffer, mode->K, fixed);
#endif
}

void fx25_init(void) {
#ifdef FX25_PREGENERATE_POLYS
    RsInit(&rs16, 16, FX25_RS_FCR);
    RsInit(&rs32, 32, FX25_RS_FCR);
    RsInit(&rs64, 64, FX25_RS_FCR);
#else
#endif
}
