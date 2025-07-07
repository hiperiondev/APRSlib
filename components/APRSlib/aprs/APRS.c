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
#include <stdint.h>
#include <string.h>

#include "APRSlib_port.h"
#include "ax25.h"

#define countof(a) sizeof(a) / sizeof(a[0])

static const char *TAG = "afsk";

extern void aprs_msg_callback(struct AX25Msg *msg);

unsigned long custom_preamble = 350UL;
unsigned long custom_tail = 50UL;
ax25ctx_t AX25;
ax25call_t src;
ax25call_t dst;
ax25call_t path1;
ax25call_t path2;
char CALL[7] = "NOCALL";
int CALL_SSID = 0;
char DST[7] = "APE32I";
int DST_SSID = 0;
char PATH1[7] = "WIDE1";
int PATH1_SSID = 1;
char PATH2[7] = "WIDE2";
int PATH2_SSID = 2;
ax25call_t path[8];
char latitude[9];
char longtitude[10];
char symbolTable = '/';
char symbol = 'n';
uint8_t power = 10;
uint8_t height = 10;
uint8_t gain = 10;
uint8_t directivity = 10;
char message_recip[7];
int message_recip_ssid = -1;
int message_seq = 0;
char lastMessage[67];
size_t lastMessageLen;
bool message_autoAck = false;

void APRS_init() {
    // AFSK_init();
    // Ax25Init(aprs_msg_callback);
    // ax25_init(&AX25, aprs_msg_callback);
}

void APRS_poll(void) {
    // ax25_poll(&AX25);
}

void APRS_setCallsign(char *call, int ssid) {
    memset(CALL, 0, 7);
    int i = 0;

    while (i < 6 && call[i] != 0) {
        CALL[i] = call[i];
        i++;
    }

    CALL_SSID = ssid;
}

void APRS_setDestination(char *call, int ssid) {
    memset(DST, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        DST[i] = call[i];
        i++;
    }
    DST_SSID = ssid;
}

void APRS_setPath1(char *call, int ssid) {
    memset(PATH1, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        PATH1[i] = call[i];
        i++;
    }
    PATH1_SSID = ssid;
}

void APRS_setPath2(char *call, int ssid) {
    memset(PATH2, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        PATH2[i] = call[i];
        i++;
    }
    PATH2_SSID = ssid;
}

void APRS_setMessageDestination(char *call, int ssid) {
    memset(message_recip, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        message_recip[i] = call[i];
        i++;
    }
    message_recip_ssid = ssid;
}

void APRS_setPreamble(unsigned long pre) {
    custom_preamble = pre;
}

void APRS_setTail(unsigned long tail) {
    custom_tail = tail;
}

void APRS_useAlternateSymbolTable(bool use) {
    if (use) {
        symbolTable = '\\';
    } else {
        symbolTable = '/';
    }
}

void APRS_setSymbol(char sym) {
    symbol = sym;
}

void APRS_setLat(char *lat) {
    memset(latitude, 0, 9);
    int i = 0;
    while (i < 8 && lat[i] != 0) {
        latitude[i] = lat[i];
        i++;
    }
}

void APRS_setLon(char *lon) {
    memset(longtitude, 0, 10);
    int i = 0;
    while (i < 9 && lon[i] != 0) {
        longtitude[i] = lon[i];
        i++;
    }
}

void APRS_setPower(int s) {
    if (s >= 0 && s < 10) {
        power = s;
    }
}

void APRS_setHeight(int s) {
    if (s >= 0 && s < 10) {
        height = s;
    }
}

void APRS_setGain(int s) {
    if (s >= 0 && s < 10) {
        gain = s;
    }
}

void APRS_setDirectivity(int s) {
    if (s >= 0 && s < 10) {
        directivity = s;
    }
}

void APRS_printSettings() {
    log_i(TAG, "LibAPRS Settings:\n");
    log_i(TAG, "Callsign:     ");
    log_i(TAG, "%s", CALL);
    log_i(TAG, "-");
    log_i(TAG, "%d\n", CALL_SSID);
    log_i(TAG, "Destination:  ");
    log_i(TAG, "%s\n", DST);
    log_i(TAG, "-");
    log_i(TAG, "%d\n", DST_SSID);
    log_i(TAG, "Path1:        ");
    log_i(TAG, "%s", PATH1);
    log_i(TAG, "-");
    log_i(TAG, "%d", PATH1_SSID);
    log_i(TAG, "Path2:        ");
    log_i(TAG, "%s", PATH2);
    log_i(TAG, "-");
    log_i(TAG, "%d\n", PATH2_SSID);
    log_i(TAG, "Message dst:  ");
    if (message_recip[0] == 0) {
        log_i(TAG, "N/A");
    } else {
        log_i(TAG, "%s", message_recip);
        log_i(TAG, "-");
        log_i(TAG, "%d", message_recip_ssid);
    }
    log_i(TAG, "TX Preamble:  ");
    log_i(TAG, "%lu", custom_preamble);
    log_i(TAG, "TX Tail:      ");
    log_i(TAG, "%lu", custom_tail);
    log_i(TAG, "%s", "Symbol table: ");
    if (symbolTable == '/') {
        log_i(TAG, "Normal");
    } else {
        log_i(TAG, "Alternate");
    }
    log_i(TAG, "Symbol:       ");
    log_i(TAG, "%c", symbol);
    log_i(TAG, "Power:        ");
    if (power < 10) {
        log_i(TAG, "%d", power);
    } else {
        log_i(TAG, "N/A");
    }
    log_i(TAG, "Height:       ");
    if (height < 10) {
        log_i(TAG, "%d", height);
    } else {
        log_i(TAG, "N/A");
    }
    log_i(TAG, "Gain:         ");
    if (gain < 10) {
        log_i(TAG, "%d", gain);
    } else {
        log_i(TAG, "N/A");
    }
    log_i(TAG, "Directivity:  ");
    if (directivity < 10) {
        log_i(TAG, "%d", directivity);
    } else {
        log_i(TAG, "N/A");
    }
    log_i(TAG, "Latitude:     ");
    if (latitude[0] != 0) {
        log_i(TAG, "%s", latitude);
    } else {
        log_i(TAG, "N/A");
    }
    log_i(TAG, "Longtitude:   ");
    if (longtitude[0] != 0) {
        log_i(TAG, "%s", longtitude);
    } else {
        log_i(TAG, "N/A");
    }
}

void APRS_sendPkt(void *_buffer, size_t length) {

    //uint8_t *buffer = (uint8_t *)_buffer;

    memcpy(dst.call, DST, 6);
    dst.ssid = DST_SSID;

    memcpy(src.call, CALL, 6);
    src.ssid = CALL_SSID;

    memcpy(path1.call, PATH1, 6);
    path1.ssid = PATH1_SSID;

    memcpy(path2.call, PATH2, 6);
    path2.ssid = PATH2_SSID;

    path[0] = dst;
    path[1] = src;
    path[2] = path1;
    path[3] = path2;

    // ax25_sendVia(&AX25, path, countof(path), buffer, length);
}

void APRS_sendTNC2Pkt(const uint8_t *raw, size_t length) {
    uint8_t data[300];
    int size = 0;
    ax25frame_t frame;
    ax25_encode(&frame, (char *)raw, length);
    size = hdlcFrame(data, 300, &AX25, &frame);
    log_i(TAG, "TX HDLC Fram size=%d", size);
    void *handle = NULL;
    if (size > 0) {
        if (NULL != (handle = Ax25WriteTxFrame(data, size))) {
            // Ax25TransmitCheck();
        }
    }
}

// Dynamic RAM usage of this function is 30 bytes
void APRS_sendLoc(void *_buffer, size_t length) {
    size_t payloadLength = 20 + length;
    bool usePHG = false;
    if (power < 10 && height < 10 && gain < 10 && directivity < 9) {
        usePHG = true;
        payloadLength += 7;
    }
    uint8_t *packet = (uint8_t *)malloc(payloadLength);
    uint8_t *ptr = packet;
    packet[0] = '=';
    packet[9] = symbolTable;
    packet[19] = symbol;
    ptr++;
    memcpy(ptr, latitude, 8);
    ptr += 9;
    memcpy(ptr, longtitude, 9);
    ptr += 10;
    if (usePHG) {
        packet[20] = 'P';
        packet[21] = 'H';
        packet[22] = 'G';
        packet[23] = power + 48;
        packet[24] = height + 48;
        packet[25] = gain + 48;
        packet[26] = directivity + 48;
        ptr += 7;
    }
    if (length > 0) {
        uint8_t *buffer = (uint8_t *)_buffer;
        memcpy(ptr, buffer, length);
    }

    APRS_sendPkt(packet, payloadLength);
    free(packet);
}

// Dynamic RAM usage of this function is 18 bytes
void APRS_sendMsg(void *_buffer, size_t length) {
    if (length > 67)
        length = 67;
    size_t payloadLength = 11 + length + 4;

    uint8_t *packet = (uint8_t *)malloc(payloadLength);
    uint8_t *ptr = packet;
    packet[0] = ':';
    int callSize = 6;
    int count = 0;
    while (callSize--) {
        if (message_recip[count] != 0) {
            packet[1 + count] = message_recip[count];
            count++;
        }
    }
    if (message_recip_ssid != -1) {
        packet[1 + count] = '-';
        count++;
        if (message_recip_ssid < 10) {
            packet[1 + count] = message_recip_ssid + 48;
            count++;
        } else {
            packet[1 + count] = 49;
            count++;
            packet[1 + count] = message_recip_ssid - 10 + 48;
            count++;
        }
    }
    while (count < 9) {
        packet[1 + count] = ' ';
        count++;
    }
    packet[1 + count] = ':';
    ptr += 11;
    if (length > 0) {
        uint8_t *buffer = (uint8_t *)_buffer;
        memcpy(ptr, buffer, length);
        memcpy(lastMessage, buffer, length);
        lastMessageLen = length;
    }

    message_seq++;
    if (message_seq > 999)
        message_seq = 0;

    packet[11 + length] = '{';
    int n = message_seq % 10;
    int d = ((message_seq % 100) - n) / 10;
    int h = (message_seq - d - n) / 100;

    packet[12 + length] = h + 48;
    packet[13 + length] = d + 48;
    packet[14 + length] = n + 48;

    APRS_sendPkt(packet, payloadLength);
    free(packet);
}

void APRS_msgRetry() {
    message_seq--;
    APRS_sendMsg(lastMessage, lastMessageLen);
}
