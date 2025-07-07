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

#ifndef DRIVERS_MODEM_H_
#define DRIVERS_MODEM_H_

#include <stdbool.h>
#include <stdint.h>

// number of maximum parallel demodulators
// each demodulator must be explicitly configured in code
// currently used only for 1200 Bd modem
#define MODEM_MAX_DEMODULATOR_COUNT 2

enum ModemType {
    MODEM_1200 = 0,
    MODEM_1200_V23,
    MODEM_300,
    MODEM_9600,
};

typedef enum ModemTxTestMode_e {
    TEST_DISABLED,
    TEST_MARK,
    TEST_SPACE,
    TEST_ALTERNATING,
} modemtxtestmode_t;

typedef struct ModemDemodConfig_s {
    enum ModemType modem;
    uint8_t usePWM : 1;      // 0 - use R2R, 1 - use PWM
    uint8_t flatAudioIn : 1; // 0 - normal (deemphasized) audio input, 1 - flat audio (unfiltered) input
} modem_demod_config_t;

extern modem_demod_config_t ModemConfig;

typedef enum ModemPrefilter_e {
    PREFILTER_NONE = 0,
    PREFILTER_PREEMPHASIS,
    PREFILTER_DEEMPHASIS,
    PREFILTER_FLAT,
} modemprefilter_t;

/**
 * @brief Get measured signal level
 * @param modem Modem number
 * @param *peak Output signal positive peak in %
 * @param *valley Output signal negative peak in %
 * @param *level Output signal level in %
 */
void ModemGetSignalLevel(uint8_t modem, int8_t *peak, int8_t *valley, uint8_t *level);

/**
 * @brief Get current modem baudrate
 * @return Baudrate
 */
float ModemGetBaudrate(void);

/**
 * @brief Get count of demodulators running in parallel
 * @return Count of demodulators
 */
uint8_t ModemGetDemodulatorCount(void);

/**
 * @brief Get prefilter type (preemphasis, deemphasis etc.) for given modem
 * @param modem Modem number
 * @return Filter type
 */
modemprefilter_t ModemGetFilterType(uint8_t modem);

/**
 * @brief Get current DCD state
 * @return 1 if channel busy, 0 if free
 */
uint8_t ModemDcdState(void);

/**
 * @brief Check if there is a TX test mode enabled
 * @return 1 if in TX test mode, 0 otherwise
 */
uint8_t ModemIsTxTestOngoing(void);

/**
 * @brief Clear modem RMS counter
 * @param number Modem number
 */
void ModemClearRMS(uint8_t number);

/**
 * @brief Get RMS value for modem
 * @param number Modem number
 * @return RMS value
 */
uint16_t ModemGetRMS(uint8_t number);

/**
 * @brief Start or restart TX test mode
 * @param type TX test type: TEST_MARK, TEST_SPACE or TEST_ALTERNATING
 */
void ModemTxTestStart(modemtxtestmode_t type);

/**
 * @brief Stop TX test mode
 */
void ModemTxTestStop(void);

/**
 * @brief Configure and start TX
 * @info This function is used internally by protocol module.
 * @warning Use Ax25TransmitStart() to initialize transmission
 */
void ModemTransmitStart(void);

/**
 * @brief Stop TX and go back to RX
 */
void ModemTransmitStop(void);

/**
 * @brief Initialize modem module
 */
void ModemInit(void);

void MODEM_DECODE(int16_t sample, uint16_t mVrms);
uint8_t MODEM_BAUDRATE_TIMER_HANDLER(void);

#endif
