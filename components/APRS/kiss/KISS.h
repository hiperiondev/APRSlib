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

#ifndef _PROTOCOL_KISS
#define _PROTOCOL_KISS 0x02

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define FEND  0xC0
#define FESC  0xDB
#define TFEND 0xDC
#define TFESC 0xDD

#define CMD_UNKNOWN     0xFE
#define CMD_DATA        0x00
#define CMD_TXDELAY     0x01
#define CMD_P           0x02
#define CMD_SLOTTIME    0x03
#define CMD_TXTAIL      0x04
#define CMD_FULLDUPLEX  0x05
#define CMD_SETHARDWARE 0x06
#define CMD_RETURN      0xFF

#define AX25_MAX_FRAME_LEN 329

int kiss_wrapper(uint8_t *pkg, uint8_t *buf, size_t len);
void kiss_serial(uint8_t sbyte);
size_t kiss_parse(uint8_t *buf, uint8_t *raw, size_t len);

#endif
