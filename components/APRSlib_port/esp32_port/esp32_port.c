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

#include <stdint.h>

#include <driver/gptimer.h>
#include <driver/sdm.h>
#include <dsps_fir.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_continuous.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/misc.h>
#include <soc/syscon_struct.h>

#include "esp32_port.h"

void port_delay(long msec) {
    vTaskDelay(msec / portTICK_PERIOD_MS);
}

uint64_t port_millis(void) {
    return esp_timer_get_time() / 1000;
}

long port_random(uint32_t min, uint32_t max) {
    long res = random();
    if (res < min)
        res = min;
    if (res > max)
        res = max;

    return res;
}

void port_pin_mode(uint8_t pin, pin_mode_t mode) {
}

bool port_digital_read(uint8_t pin) {
    return true;
}

void port_digital_write(uint8_t pin, pin_value_t mod) {
}

void port_queue_init(uint32_t size) {
}

void port_queue_flush(void) {
}

uint32_t port_queue_getcount(void) {
    return 0;
}

bool port_queue_pop(int16_t *value) {
    return true;
}

void port_led_status(uint8_t red, uint8_t green, uint8_t blue) {
}

void port_timer_enable(bool enable) {
}

void port_dac_timer_enable(bool enable) {
}
///////////////////////////////////////////////////////////////////////

void port_adc_continue_init(void) {
}

uint8_t port_adc_cali_raw_to_voltage(int raw, int *voltage) {
    return 1;
}

void port_sigmadelta_init(void) {
}

uint8_t port_sigmadelta_set_duty(uint8_t channel, int8_t duty) {
    return 1;
}
