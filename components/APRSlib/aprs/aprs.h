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

#ifndef APRS_H_
#define APRS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
 
void aprs_init();
void aprs_poll(void);

void aprs_set_callsign(char *call, int ssid);
void aprs_set_destination(char *call, int ssid);
void aprs_set_message_destination(char *call, int ssid);
void aprs_set_path1(char *call, int ssid);
void aprs_set_path2(char *call, int ssid);

void aprs_set_preamble(unsigned long pre);
void aprs_set_tail(unsigned long tail);
void aprs_use_alternate_symbol_table(bool use);
void aprs_set_symbol(char sym);

void aprs_set_latitude(char *lat);
void aprs_set_longitude(char *lon);
void aprs_set_power(int s);
void aprs_set_height(int s);
void aprs_set_gain(int s);
void aprs_set_directivity(int s);

void aprs_send_packet(void *_buffer, size_t length);
void aprs_send_location(void *_buffer, size_t length);
void aprs_send_message(void *_buffer, size_t length);
void aprs_message_retry(void);

void aprs_print_settings(void);
void aprs_send_tnc2_packet(const uint8_t *raw, size_t length);

#endif /* APRS_H_ */
