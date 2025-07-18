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

#ifndef AX25_H_
#define AX25_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define AX25_NOT_FX25 255

// for AX.25 329 bytes is the theoretical max size assuming 2-byte Control, 1-byte PID, 256-byte info field and 8 digi address fields
#define AX25_FRAME_MAX_SIZE (329) // single frame max length
#define AX25_CTRL_UI        0x03
#define AX25_PID_NOLAYER3   0xF0
#define AX25_CALL(str, id)                                                                                                                                     \
    { .call = (str), .ssid = (id) }
#define AX25_MAX_RPT          8
#define AX25_REPEATED(msg, n) ((msg)->rpt_flags & BV(n))
#define CALL_OVERSPACE        1

typedef enum Ax25RxStage_e {
    RX_STAGE_IDLE = 0,
    RX_STAGE_FLAG,
    RX_STAGE_FRAME,
#ifdef ENABLE_FX25
    RX_STAGE_FX25_FRAME,
#endif
} ax25_rxstage_t;

typedef struct Ax25ProtoConfig_s {
    uint16_t txDelayLength;   // TXDelay length in ms
    uint16_t txTailLength;    // TXTail length in ms
    uint16_t quietTime;       // Quiet time in ms
    uint8_t allowNonAprs : 1; // allow non-APRS packets
    bool fx25 : 1;            // enable FX.25 (AX.25 + FEC)
    bool fx25Tx : 1;          // enable TX in FX.25
} ax25_protoconfig_t;

struct AX25Ctx; // Forward declarations
struct AX25Msg;

typedef void (*ax25_callback_t)(struct AX25Msg *msg);
typedef struct Hdlc_s {
    uint8_t demodulatedBits;
    uint8_t bitIndex;
    uint8_t currentByte;
    bool receiving;
} hdlc_t;

typedef struct AX25Ctx_s {
    uint8_t buf[AX25_FRAME_MAX_SIZE];
    size_t frame_len;
    uint16_t crc_in;
    uint16_t crc_out;
    ax25_callback_t hook;
    bool sync;
    bool escape;
} ax25ctx_t;

typedef struct ax25header_s {
    char addr[7];
    char ssid;
} ax25header_t;

typedef struct ax25frame_s {
    ax25header_t header[10];
    char data[AX25_FRAME_MAX_SIZE];
} ax25frame_t;

typedef struct AX25Call_s {
    char call[6 + CALL_OVERSPACE];
    uint8_t ssid;
} ax25call_t;

typedef struct AX25Msg_s {
    ax25call_t src;
    ax25call_t dst;
    ax25call_t rpt_list[AX25_MAX_RPT];
    uint8_t rpt_count;
    uint8_t rpt_flags;
    uint16_t ctrl;
    uint8_t pid;
    uint8_t info[AX25_FRAME_MAX_SIZE];
    size_t len;
    uint16_t mVrms;
} ax25msg_t;

extern ax25_protoconfig_t Ax25Config;
extern bool ax25_stateTx;
extern int transmissionState;

/**
 * @brief Write frame to transmit buffer
 * @param *data Data to transmit
 * @param size Data size
 * @return Pointer to internal frame handle or NULL on failure
 * @attention This function will block if transmission is already in progress
 */
void *ax25_write_tx_frame(uint8_t *data, uint16_t size);

/**
 * @brief Get bitmap of "frame received" flags for each decoder. A non-zero value means that a frame was received
 * @return Bitmap of decoder that received the frame
 */
uint8_t ax25_get_received_frame_bitmap(void);

/**
 * @brief Clear bitmap of "frame received" flags
 */
void ax25_clear_received_frame_bitmap(void);

/**
 * @brief Get next received frame (if available)
 * @param **dst Pointer to internal buffer
 * @param *size Actual frame size
 * @param *peak Signak positive peak value in %
 * @param *valley Signal negative peak value in %
 * @param *level Signal level in %
 * @param *corrected Number of bytes corrected in FX.25 mode. 255 is returned if not a FX.25 packet.
 * @return True if frame was read, false if no more frames to read
 */
bool ax25_read_next_rx_frame(uint8_t **dst, uint16_t *size, int8_t *peak, int8_t *valley, uint8_t *level, uint8_t *corrected, uint16_t *mV);

/**
 * @brief Get current RX stage
 * @param[in] modemNo Modem/decoder number (0 or 1)
 * @return RX_STATE_IDLE, RX_STATE_FLAG or RX_STATE_FRAME
 * @warning Only for internal use
 */
ax25_rxstage_t ax25_get_rx_stage(uint8_t modemNo);

/**
 * @brief Parse incoming bit (not symbol!)
 * @details Handles bit-stuffing, header and CRC checking, stores received frame and sets "frame received flag", multiplexes both decoders
 * @param[in] bit Incoming bit
 * @param[in] *dem Modem state pointer
 * @warning Only for internal use
 */
void ax25_bit_parse(uint8_t bit, uint8_t modem, uint16_t mV);

/**
 * @brief Get next bit to be transmitted
 * @return Bit to be transmitted
 * @warning Only for internal use
 */
uint8_t ax25_get_tx_bit(void);

/**
 * @brief Initialize transmission and start when possible
 */
void ax25_transmit_buffer(void);

/**
 * @brief Start transmitting when possible
 * @attention Must be continuously polled in main loop
 */
void ax25_transmit_check(void);

/**
 * @brief Initialize AX25 module
 */
void ax25_init(uint8_t fx25Mode);

void ax25_decode(uint8_t *buf, size_t len, uint16_t mVrms, ax25msg_t *msg);
char ax25_encode(ax25frame_t *frame, char *txt, int size);
int ax25_hdlc_frame(uint8_t *outbuf, size_t outbuf_len, ax25ctx_t *ctx, ax25frame_t *pkg);
void ax25_tx_delay(uint16_t delay_ms);
void ax25_time_slot(uint16_t ts);
bool ax25_new_rx_frames(void);

#endif /* AX25_H_ */
