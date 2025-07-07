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

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "afsk.h"
#include "ax25.h"
#include "fx25.h"
#include "modem.h"
#include "APRSlib_port.h"

static const char *TAG = "afsk";

#define DEBUG_TNC
#define FILTER_TAPS 8 // Number of taps in the FIR filter

// Resampling configuration
#define INPUT_RATE  38400
#define OUTPUT_RATE 9600

// AGC configuration
#define AGC_TARGET_RMS 0.2f   // Target RMS level (-10dBFS)
#define AGC_ATTACK     0.02f  // Fast attack rate
#define AGC_RELEASE    0.001f // Slow release rate
#define AGC_MAX_GAIN   10.0f
#define AGC_MIN_GAIN   0.1f

#define AX25_FLAG         0x7e
#define AX25_MASK         0xfc // bit mask of MSb six bits
#define AX25_EOP          0xfc // end of packet, 7e << 1
#define AX25_STUFF_BIT    0x7c // bit stuffing bit, five of continuous one bits
#define AX25_FLAG_BITS    6
#define AX25_ADDR_LEN     7
#define AX25_MIN_PKT_SIZE (AX25_ADDR_LEN * 2 + 1 + 2) // src addr + dst addr + Control + FCS
#define AX25_SSID_MASK    0x0f

#define DEFAULT_SEMAPHORE_TIMEOUT 10

typedef struct interrupt_config_s {
    void *fn;
    void *arg;
} interrupt_config_t;

extern float markFreq;  // mark frequency
extern float spaceFreq; // space frequency
extern float baudRate;  // baudrate
extern unsigned long custom_preamble;
extern unsigned long custom_tail;
extern ax25ctx_t AX25;
extern int mVrms;
extern float dBV;

int adcVal;
int8_t _sql_pin, _ptt_pin, _pwr_pin, _dac_pin, _adc_pin;
bool _sql_active, _ptt_active, _pwr_active;
uint8_t adc_atten;
bool hw_afsk_dac_isr = false;
uint16_t SAMPLERATE = 38400;
uint16_t RESAMPLE_RATIO = (38400 / OUTPUT_RATE); // 38400/9600 = 3
uint16_t BLOCK_SIZE = (38400 / 50);              // Must be multiple of resample ratio
float *audio_buffer = NULL;
int8_t _led_rx_pin = 2;
int8_t _led_tx_pin = 4;
int8_t _led_strip_pin = -1;
uint8_t r_old = 0, g_old = 0, b_old = 0;
unsigned long rgbTimeout = 0;
float agc_gain = 1.0f;
tcb_t tcb;
volatile bool new_samples = false;
uint8_t modem_config = 0;
port_SemaphoreHandle_t xI2CSemaphore;
int offset = 0;
int dc_offset = 620;
bool sqlActive = false;
uint8_t sinwave = 127;
bool adcq_lock = false;
uint16_t phaseAcc = 0;
uint32_t ret_num;
uint8_t *resultADC;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
long mVsum = 0;
int mVsumCount = 0;
uint8_t dcd_cnt = 0;

// Filter coefficients (designed for 38400→9600 resampling)
// cutoff = 4800  # Nyquist for 9600Hz
static const float resample_coeffs[FILTER_TAPS] = { 0.003560, 0.038084, 0.161032, 0.297324, 0.297324, 0.161032, 0.038084, 0.003560 };

static void resample_audio(float *input_buffer) {
    // Apply anti-aliasing filter and decimate
    for (int i = 0; i < BLOCK_SIZE / RESAMPLE_RATIO; i++) {
        float sum = 0;
        for (int j = 0; j < FILTER_TAPS; j++) {
            int index = i * RESAMPLE_RATIO + j;
            if (index < BLOCK_SIZE) {
                sum += input_buffer[index] * resample_coeffs[j];
            }
        }
        input_buffer[i] = sum;
    }
}

static float update_agc(float *input_buffer, size_t len) {
    // Calculate RMS of current block
    float sum_sq = 0;
    for (int i = 0; i < len; i++) {
        sum_sq += input_buffer[i] * input_buffer[i];
    }
    float rms = sqrtf(sum_sq / len);

    // Adjust gain based on RMS level
    float error = AGC_TARGET_RMS / (rms + 1e-6f);
    float rate = (error < 1.0f) ? AGC_RELEASE : AGC_ATTACK;
    agc_gain = agc_gain * (1.0f - rate) + (agc_gain * error) * rate;
    agc_gain = fmaxf(fminf(agc_gain, AGC_MAX_GAIN), AGC_MIN_GAIN);
    return agc_gain;
}

static void hw_init(void) {
    // Set up ADC
    if (_sql_pin > -1) {
        port_pin_mode(_sql_pin, INPUT_PULLUP);
    }

    if (_pwr_pin > -1) {
        port_pin_mode(_pwr_pin, OUTPUT);
    }

    if (_led_tx_pin > -1) {
        port_pin_mode(_led_tx_pin, OUTPUT);
    }

    if (_led_rx_pin > -1) {
        port_pin_mode(_led_rx_pin, OUTPUT);
    }

    if (_pwr_pin > -1) {
        port_digital_write(_pwr_pin, !_pwr_active);
    }

    port_adc_continue_init();
    port_sigmadelta_init();

    afsk_set_transmit(false);
}

////////////////////////////////////////////////

bool afsk_get_transmit() {
    bool ret = false;

    if ((port_digital_read(_ptt_pin) ^ _ptt_active) == 0) // signal active with ptt_active
        ret = true;
    else
        ret = false;
    if (hw_afsk_dac_isr)
        ret = true;

    return ret;
}

void afsk_set_transmit(bool val) {
    hw_afsk_dac_isr = val;
}

bool afsk_get_receive(void) {
    bool ret = false;

    if ((port_digital_read(_ptt_pin) ^ _ptt_active) == 0) // signal active with ptt_active
        return false;                               // PTT Protection receive
    if (port_digital_read(LED_RX_PIN))                    // Check RX LED receiving.
        ret = true;

    return ret;
}

/**
 * @brief Controls PTT output
 * @param state False - PTT off, true - PTT on
 */
void afsk_set_ptt(bool state) {

    if (state) {
        afsk_set_transmit(true);
        if (_ptt_pin > 34)
            _ptt_pin = 32;
        if (_ptt_active) {
            port_pin_mode(_ptt_pin, OUTPUT);
            port_digital_write(_ptt_pin, HIGH);
        } else { // Open Collector to LOW
            port_pin_mode(_ptt_pin, OUTPUT_OPEN_DRAIN);
            port_digital_write(_ptt_pin, LOW);
        }
        port_led_status(255, 0, 0);
    } else {
        afsk_set_transmit(false);
        port_queue_flush();
        if (_ptt_active) {
            port_pin_mode(_ptt_pin, OUTPUT);
            port_digital_write(_ptt_pin, LOW);
        } else { // Open Collector to HIGH
            port_pin_mode(_ptt_pin, OUTPUT_OPEN_DRAIN);
            port_digital_write(_ptt_pin, HIGH);
        }
        port_led_status(0, 0, 0);
    }
}

void afsk_set_modem(uint8_t val, bool bpf, uint16_t timeSlot, uint16_t preamble, uint8_t fx25Mode) {
    if (bpf)
        ModemConfig.flatAudioIn = 1;
    else
        ModemConfig.flatAudioIn = 0;

    if (val == 0) {
        ModemConfig.modem = MODEM_300;
        SAMPLERATE = 28800;
        BLOCK_SIZE = (SAMPLERATE / 50); // Must be multiple of resample ratio
        RESAMPLE_RATIO = (SAMPLERATE / 9600);
    } else if (val == 1) {
        ModemConfig.modem = MODEM_1200;
        SAMPLERATE = 19200;
        BLOCK_SIZE = (SAMPLERATE / 50); // Must be multiple of resample ratio
        RESAMPLE_RATIO = (SAMPLERATE / 9600);
    } else if (val == 2) {
        ModemConfig.modem = MODEM_1200_V23;
        SAMPLERATE = 19200;
        BLOCK_SIZE = (SAMPLERATE / 50); // Must be multiple of resample ratio
        RESAMPLE_RATIO = (SAMPLERATE / 9600);
    } else if (val == 3) {
        ModemConfig.modem = MODEM_9600;
        SAMPLERATE = 38400;
        BLOCK_SIZE = (SAMPLERATE / 100); // Must be multiple of resample ratio
        RESAMPLE_RATIO = (SAMPLERATE / 38400);
    }
    if (audio_buffer != NULL) {
        free(audio_buffer);
        audio_buffer = NULL;
    }
    audio_buffer = (float *)calloc(BLOCK_SIZE, sizeof(float));
    if (audio_buffer == NULL) {
        log_i(TAG, "Error allocating memory for audio buffer");
        return;
    }
    log_i(TAG, "Modem: %d, SampleRate: %d, BlockSize: %d", ModemConfig.modem, SAMPLERATE, BLOCK_SIZE);
    ModemConfig.usePWM = 1;
    modem_init();
    ax25_init(fx25Mode);
    if (fx25Mode > 0)
        fx25_init();
    ax25_time_slot(timeSlot);
    ax25_tx_delay(preamble);
}

void afsk_set_ptt_value(int8_t val, bool act) {
    _ptt_pin = val;
    _ptt_active = act;
}

void afsk_init(int8_t adc_pin, int8_t dac_pin, int8_t ptt_pin, int8_t sql_pin, int8_t pwr_pin, int8_t led_tx_pin, int8_t led_rx_pin, int8_t led_strip_pin,
               bool ptt_act, bool sql_act, bool pwr_act) {

    port_queue_init(768 * 2); // Instantiate queue

    _adc_pin = adc_pin;
    _dac_pin = dac_pin;
    _ptt_pin = ptt_pin;
    _sql_pin = sql_pin;
    _pwr_pin = pwr_pin;
    _led_tx_pin = led_tx_pin;
    _led_rx_pin = led_rx_pin;
    _led_strip_pin = led_strip_pin;

    _ptt_active = ptt_act;
    _sql_active = sql_act;
    _pwr_active = pwr_act;

    giveSemaphore();

    tcb_t *tp = &tcb;
    int i = 0;
    tp->port = i;
    tp->kiss_type = i << 4;
    tp->avg = 2048;
    tp->cdt = false;
    tp->cdt_lvl = 0;
    tp->cdt_led_pin = 2;
    tp->cdt_led_on = 2;
    log_i(TAG, "cdt_led_pin = %d, cdt_led_on = %d, port = %d", tp->cdt_led_pin, tp->cdt_led_on, tp->port);

#ifdef FX25TNCR2
    tp->sta_led_pin = STA_LED_PIN[i];
#endif

#ifdef FX25_ENABLE
    tp->fx25_parity = FX25_PARITY[i]; // FX.25 parity
#endif
    tp->cdt_sem = xSemaphoreCreateBinary();
    assert(tp->cdt_sem);
    assert(xSemaphoreGive(tp->cdt_sem) == pdTRUE); // initialize

    tp->fullDuplex = true;  // full duplex
    tp->SlotTime = 10;      // 100ms
    tp->TXDELAY = 50;       // 500ms
    tp->persistence_P = 63; // P = 0.25
    hw_init();
}

void afsk_poll(void) {
    int mV;
    int x = 0;
    int16_t adc = 0;

    if (hw_afsk_dac_isr) {
    } else {
        if (audio_buffer != NULL) {
            tcb_t *tp = &tcb;
            while (port_queue_getcount() >= BLOCK_SIZE) {
                mVsum = 0;
                mVsumCount = 0;
                for (x = 0; x < BLOCK_SIZE; x++) {
                    if (!port_queue_pop(&adc)) // Pull queue buffer
                        break;

                    tp->avg_sum += adc - tp->avg_buf[tp->avg_idx];
                    tp->avg_buf[tp->avg_idx++] = adc;
                    if (tp->avg_idx >= TCB_AVG_N)
                        tp->avg_idx -= TCB_AVG_N;
                    tp->avg = tp->avg_sum / TCB_AVG_N;

                    // carrier detect
                    adcVal = (int)adc - tp->avg;
                    int m = 1;
                    if ((RESAMPLE_RATIO > 1) || (ModemConfig.modem == MODEM_9600)) {
                        m = 4;
                    }

                    if (x % m == 0) {
                        port_adc_cali_raw_to_voltage(adc, &mV);
                        mV -= offset;
                        mVsum += mV * mV; // Accumulate squared voltage values
                        mVsumCount++;
                    }

                    float sample = adcVal / 2048.0f * agc_gain;
                    audio_buffer[x] = sample;
                }
                // Update AGC gain
                update_agc(audio_buffer, BLOCK_SIZE);
                port_adc_cali_raw_to_voltage(tp->avg, &offset);

                if (mVsumCount > 0) {
                    tp->cdt_lvl = mVrms = sqrtl(mVsum / mVsumCount); // RMS voltage  VRMS = √(1/mVsumCount)(mVsum)
                    mVsum = 0;
                    mVsumCount = 0;
                    if (mVrms > 10) // >-40dBm
                    {
                        if (dcd_cnt < 100)
                            dcd_cnt++;
                    } else if (mVrms < 5) // <-46dBm
                    {
                        if (dcd_cnt > 0)
                            dcd_cnt--;
                    }
                }

                if ((dcd_cnt > 3) || (ModemConfig.modem == MODEM_9600)) {
                    tp->cdt = true;
                    //  Process audio block
                    if (RESAMPLE_RATIO > 1)
                        resample_audio(audio_buffer);

                    // Process audio block
                    int16_t sample;

                    for (int i = 0; i < BLOCK_SIZE / RESAMPLE_RATIO; i++) {
                        // Convert back to 12-bit audio (0-4095)
                        sample = audio_buffer[i] * 2048;
                        modem_decode(sample, mVrms);
                    }
                } else {
                    tp->cdt = false;
                }
            }
        }
    }
}
