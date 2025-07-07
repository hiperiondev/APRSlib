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

#ifndef ESP32_PORT_H_
#define ESP32_PORT_H_

#include <stdint.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#define port_enter_critical_isr(mux) portENTER_CRITICAL_ISR(mux)
#define port_exit_critical_isr(mux) portEXIT_CRITICAL_ISR(mux)

#define port_TaskHandle_t      TaskHandle_t
#define port_SemaphoreHandle_t SemaphoreHandle_t
#define digitalRead(pin)       1
#define pinMode(pin, mode)     // TODO: implement
#define digitalWrite(pin, val) // TODO: implement
#define log_i                  ESP_LOGI
#define __disable_irq()        // TODO: implement
#define __enable_irq()         // TODO: implement
#define giveSemaphore()                                                                                                                                        \
    if (xI2CSemaphore == NULL) {                                                                                                                               \
        xI2CSemaphore = xSemaphoreCreateMutex();                                                                                                               \
        if ((xI2CSemaphore) != NULL)                                                                                                                           \
            xSemaphoreGive((xI2CSemaphore));                                                                                                                   \
    }

void port_delay(long msec);
uint64_t port_millis(void);
long port_random(uint32_t min, uint32_t max);

void port_adc_continue_init(void);
uint8_t port_adc_cali_raw_to_voltage(int raw, int *voltage);
void port_sigmadelta_init(void);
uint8_t port_sigmadelta_set_duty(uint8_t channel, int8_t duty);

#endif /* ESP32_PORT_H_ */
