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

#include "kiss.h"
#include "ax25.h"

extern ax25_protoconfig_t Ax25Config;

static uint8_t serialBuffer[AX25_MAX_FRAME_LEN]; // Buffer for holding incoming serial data

size_t ctxbufflen;
size_t frame_len = 0;
uint8_t *ctxbuffer;
ax25ctx_t testkiss;
bool IN_FRAME;
bool ESCAPE;
uint8_t command = CMD_UNKNOWN;
uint8_t p = 63;

int kiss_wrapper(uint8_t *pkg, uint8_t *buf, size_t len) {
    uint8_t *ptr = pkg;
    int size = 0;
    *ptr++ = FEND;
    *ptr++ = 0x00;
    for (unsigned i = 0; i < len; i++) {
        uint8_t b = buf[i];
        if (b == FEND) {
            *ptr++ = FESC;
            *ptr++ = TFEND;
        } else if (b == FESC) {
            *ptr++ = FESC;
            *ptr++ = TFESC;
        } else {
            *ptr++ = b;
        }
    }
    *ptr++ = FEND;
    size = ptr - pkg;
    return size;
}

void kiss_serial(uint8_t sbyte) {

    if (IN_FRAME && sbyte == FEND && command == CMD_DATA) {
        IN_FRAME = false;
        ax25_write_tx_frame(serialBuffer, frame_len);
    } else if (sbyte == FEND) {
        IN_FRAME = true;
        command = CMD_UNKNOWN;
        frame_len = 0;
    } else if (IN_FRAME && frame_len < AX25_MAX_FRAME_LEN) {
        // Have a look at the command byte first
        if (frame_len == 0 && command == CMD_UNKNOWN) {
            // MicroModem supports only one HDLC port, so we
            // strip off the port nibble of the command byte
            sbyte = sbyte & 0x0F;
            command = sbyte;
        } else if (command == CMD_DATA) {
            if (sbyte == FESC) {
                ESCAPE = true;
            } else {
                if (ESCAPE) {
                    if (sbyte == TFEND)
                        sbyte = FEND;
                    if (sbyte == TFESC)
                        sbyte = FESC;
                    ESCAPE = false;
                }
                serialBuffer[frame_len++] = sbyte;
            }
        } else if (command == CMD_TXDELAY) {
            Ax25Config.txDelayLength = sbyte * 10UL;
        } else if (command == CMD_TXTAIL) {
            Ax25Config.txTailLength = sbyte * 10;
        } else if (command == CMD_SLOTTIME) {
            Ax25Config.quietTime = sbyte * 10;
        } else if (command == CMD_P) {
            p = sbyte;
        }
    }
}

size_t kiss_parse(uint8_t *buf, uint8_t *raw, size_t len) {
    uint8_t sbyte = 0;
    size_t frame_len = 0;
    IN_FRAME = false;
    for (int i = 0; i < len; i++) {
        sbyte = raw[i];
        if (IN_FRAME && sbyte == FEND && command == CMD_DATA) {
            IN_FRAME = false;
            return frame_len;
        } else if (sbyte == FEND) {
            IN_FRAME = true;
            command = CMD_UNKNOWN;
            frame_len = 0;
        } else if (IN_FRAME && frame_len < AX25_MAX_FRAME_LEN) {
            // Have a look at the command byte first
            if (frame_len == 0 && command == CMD_UNKNOWN) {
                // MicroModem supports only one HDLC port, so we
                // strip off the port nibble of the command byte
                sbyte = sbyte & 0x0F;
                command = sbyte;
            } else if (command == CMD_DATA) {
                if (sbyte == FESC) {
                    ESCAPE = true;
                } else {
                    if (ESCAPE) {
                        if (sbyte == TFEND)
                            sbyte = FEND;
                        if (sbyte == TFESC)
                            sbyte = FESC;
                        ESCAPE = false;
                    }
                    serialBuffer[frame_len++] = sbyte;
                }
            } else if (command == CMD_TXDELAY) {
                Ax25Config.txDelayLength = sbyte * 10UL;
            } else if (command == CMD_TXTAIL) {
                Ax25Config.txTailLength = sbyte * 10;
            } else if (command == CMD_SLOTTIME) {
                Ax25Config.quietTime = sbyte * 10;
            } else if (command == CMD_P) {
                p = sbyte;
            }
        }
    }
    return frame_len;
}
