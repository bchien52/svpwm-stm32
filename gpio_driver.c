/*
 * gpio_driver.c
 *
 * Thu vien giao tiep GPIO
 * Created on: Sep 16, 2025
 * Author: Dang Bao Chien
 *
 */

#include "gpio_driver.h"

void gpio_init(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode, gpio_cnf_t cnf) 
{		
	uint32_t pos = pin * 4; // Moi pin chiem 4 bit
	volatile uint32_t *reg;	// Tro toi thanh ghi cau hinh (CRL/CRH)
	
	// Neu pin < 8 => CRL, nguoc lai CRH
	if (pin < 8) reg = &port->CRL; // Cau hinh cho pin 0-7
	else {
			reg = &port->CRH; // Cau hinh 8-15
			pos -=32; // Dich vi tri vi CRH bat dau tu pin 8
	}
	// Xoa 4 bit cau hinh cu
	*reg &= ~(0xF << pos); 
	// Ghi gia tri mode (2 bit thấp), cnf (2 bit cao) vao vi tri pin
	*reg |= ((mode & 0x03) | ((cnf & 0x03) << 2)) << pos; 
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t state)
{
	if (state)
	{
		port->BSRR = (1 << pin); // Ghi 1 vao BSRR -> set pin
	}
	else
	{
		port->BRR = (1 << pin);	// Ghi 1 vao BRR  -> reset pin
	}
}
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin)
{	
	// Doc bit trong thanh ghi IDR, neu khac 0 -> tra ve 1, nguoc lai 0
	return (port->IDR & (1 << pin)) ? 1 : 0;
}

void gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{	
	// XOR bit pin trong thanh ghi ODR de dao trang thai
	port->ODR ^= (1 << pin);
}

