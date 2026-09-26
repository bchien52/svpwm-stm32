/**
 * @file    delay.h
 * @brief   Thu vien Delay
 * @author  Dang Bao Chien
 * @date    2025-09-20
 */

#ifndef DELAY_H
#define DELAY_H

#include "stdint.h"

/**
 * @brief Tao thoi gian tre tinh bang SysTick
 *
 * Dung de tao tre trong qua trinh thuc thi chuong trinh.
 *
 * @param ms: Thoi gian tre mong muon (ms).
 * @retval None
 */
void delay_ms(int ms);

/**
 * @brief Tao thoi gian tre tinh bang Timer
 *
 * @param ms: So mili giay can tre.
 * @retval None
 */
void delay(uint16_t ms);
void delay_us(uint16_t us);

#endif	// DELAY_H
