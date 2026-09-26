/**
 * @file    gpio_driver.h
 * @brief   Thu vien GPIO
 * @author  Dang Bao Chien
 * @date    2025-09-16
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32f10x.h"

/**
 * @brief  Cac che do lam viec cua chan GPIO
 */
typedef enum {
	GPIO_MODE_INPUT = 0x0,		  /**< Input Mode */
	GPIO_MODE_OUTPUT_10M = 0x1,	/**< Output 10MHz Mode */
	GPIO_MODE_OUTPUT_2M = 0x2,	/**< Output 2MHz Mode */
	GPIO_MODE_OUTPUT_50M = 0x3	/**< Output 50MHz Mode */
}	gpio_mode_t;

/**
 * @brief  Cau hinh chuc nang cho GPIO
 * @note 	 Chia thanh 2 nhom:
 * - Khi o che do Input: dung ANALOG, FLOATING, PU/PD
 * - Khi o che do Output/AF: dung GP_PP, GP_OD, AF_PP, AF_OD
 */
typedef enum {
	GPIO_CNF_ANALOG = 0x0, 	    /**< input analog */
	GPIO_CNF_FLOATING = 0x1,    /**< input floating */
	GPIO_CNF_PU_PD = 0x2,		    /**< input pull-up/pull-down */
	
	GPIO_CNF_GP_PP = 0x0,		    /**< output general purpose push-pull */
	GPIO_CNF_GP_OD = 0x1,		    /**< output general purpose open-drain */
	GPIO_CNF_AF_PP = 0x2,		    /**< output alternate function push-pull */
	GPIO_CNF_AF_OD = 0x3	 	    /**< output alternate function open-drain */
} gpio_cnf_t;

/**
 * @brief  Khoi tao 1 chan GPIO
 * @param  port: Port GPIO
 * @param  pin:  Pin can khoi tao
 * @param  mode: Che do hoat dong (input/output/alternate/analog)
 * @param  cnf:  Kieu cau hinh (pull-up, pull-down, open-drain, push-pull, ...)
 */
void gpio_init(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode, gpio_cnf_t cnf);

/**
 * @brief  Ghi trang thai cua 1 chan GPIO
 * @param  port: Port GPIO
 * @param  pin:  Pin can ghi trang thai
 * @param  state: Trang thai cua pin (0 hoac 1)
 */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t state);

/**
 * @brief  Doc trang thai 1 chan GPIO
 * @param  port: Port GPIO
 * @param  pin:  Pin can doc trang thai
 * @retval 0 = LOW, 1 = HIGH
 */
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief  Dao trang thai 1 chan GPIO
 * @param  port: Port GPIO
 * @param  pin:  Pin can dao trang thai
 */
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief  Kich hoat clock cho port GPIO tuong ung
 */
static inline void gpio_enable_clock(GPIO_TypeDef *port)
{
  if      (port == GPIOA) RCC->APB2ENR |= (1 << 2);
  else if (port == GPIOB) RCC->APB2ENR |= (1 << 3);
  else if (port == GPIOC) RCC->APB2ENR |= (1 << 4);
  else if (port == GPIOD) RCC->APB2ENR |= (1 << 5);
  else if (port == GPIOE) RCC->APB2ENR |= (1 << 6);
}

#endif	// GPIO_DRIVE_H_
