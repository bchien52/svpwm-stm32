/*
 * delay.c
 *
 * Thu vien Delay
 * Created on: Sep 20, 2025
 * Author: Dang Bao Chien
 *
 */

#include "stm32f10x.h"
#include "delay.h"

void delay_ms(int ms)
{
	SysTick->LOAD = 72000 - 1;    // 1 ms @ 72 MHz
	SysTick->VAL  = 0;            // reset counter
	
	// Bat SysTick timer:
  	// Bit 0 = ENABLE    : 1 = Bat timer
  	// Bit 1 = TICKINT   : 0 = Khong dung ngat
  	// Bit 2 = CLKSOURCE : 1 = Dung clock he thong HCLK (72MHz)
  	SysTick->CTRL = 0x5; // Enable
		
	// Lap cho du thoi gian yeu cau
  	for (int i = 0; i < ms; i++)
  	{	// Doi cho den khi co COUNTFLAG (bit 16) duoc bat len 1
		// Co nay se duoc set khi bo dem giam ve 0
    	while (!(SysTick->CTRL & (1 << 16)));
  	}

  	SysTick->CTRL = 0; // Tat SysTick sau khi xong
}

void delay_us(uint16_t us)
{
  	SysTick->LOAD = 72 - 1;       // 1 us @ 72 MHz (72 ticks = 1us)
  	SysTick->VAL  = 0;            // reset counter
	
  	// Bat SysTick timer (khong dung ngat, dung HCLK)
  	SysTick->CTRL = 0x5; // Enable
		
  	// Lap cho du thoi gian yeu cau
  	for (uint16_t i = 0; i < us; i++)
  	{
    	// Doi cho den khi co COUNTFLAG (bit 16) duoc bat len 1
    	while (!(SysTick->CTRL & (1 << 16)));
  	}
  	SysTick->CTRL = 0; // Tat SysTick sau khi xong
}

void delay(uint16_t ms)
{	
	TIM1->PSC = 7199; 				// Cau hinh Prescaler
	TIM1->ARR = ms*10 - 1; 			// 1 Tick = 1ms
	TIM1->CNT = 0; 					// Reset Counter
	TIM1->SR &= ~(1 << 0); 			// Xoa co UIF
	TIM1->CR1 |= (1 << 0); 			// Bat TIM1
	while (!(TIM1->SR & (1 << 0))); // Cho tran
	TIM1->CR1 &= ~(1 << 0); 		// Tat TIM1
} 
