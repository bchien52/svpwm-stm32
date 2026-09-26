#include "stm32f10x.h"
#include "gpio_driver.h"
#include "svpwm.h"
#include <math.h>

#define TIM1_ARR 3599u
#define TIM1_CLK_HZ 72000000.0f
#define DEAD_TIME 72u

#define TS (2.0f * (float)(TIM1_ARR + 1u) / TIM1_CLK_HZ)   // 100 µs

#define SIN_FREQ 50.0f
#define V_PHASE 6.0f
#define VDC 12.0f
#define UQ 6.0f
#define UD 0.0f

SVPWM_t svpwm;


float theta = 0.0f;
volatile uint8_t svpwm_flag = 0;
volatile uint32_t ccr1 = TIM1_ARR / 2u;
volatile uint32_t ccr2 = TIM1_ARR / 2u;
volatile uint32_t ccr3 = TIM1_ARR / 2u;

void GPIO_Config(void)
{
	gpio_enable_clock(GPIOA);
	gpio_enable_clock(GPIOB);
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	AFIO->MAPR &= ~AFIO_MAPR_TIM1_REMAP;
	AFIO->MAPR |=  AFIO_MAPR_TIM1_REMAP_0;

  // High side
	gpio_init(GPIOA, 8, GPIO_MODE_OUTPUT_50M, GPIO_CNF_AF_PP);
	gpio_init(GPIOA, 9, GPIO_MODE_OUTPUT_50M, GPIO_CNF_AF_PP);
	gpio_init(GPIOA, 10, GPIO_MODE_OUTPUT_50M, GPIO_CNF_AF_PP);
	
	// Low side
	gpio_init(GPIOA, 7, GPIO_MODE_OUTPUT_50M, GPIO_CNF_AF_PP);
	gpio_init(GPIOB, 0, GPIO_MODE_OUTPUT_50M, GPIO_CNF_AF_PP);
	gpio_init(GPIOB, 1, GPIO_MODE_OUTPUT_50M, GPIO_CNF_AF_PP);
}

void TIM1_Config(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM1->PSC = 0;
	TIM1->ARR = TIM1_ARR;
	TIM1->RCR = 1;

	// Center-aligned mode 1
	TIM1->CR1 |= TIM_CR1_CMS_0;
	TIM1->CR1 |= TIM_CR1_ARPE;

	// PWM mode 1 + preload CH1, CH2, CH3
	TIM1->CCMR1 = (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE);
	TIM1->CCMR2 = (TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE);

	// Bat output chinh + complementary
	TIM1->CCER = (TIM_CCER_CC1E  | TIM_CCER_CC1NE | TIM_CCER_CC2E  | TIM_CCER_CC2NE | TIM_CCER_CC3E  | TIM_CCER_CC3NE);

	// AOE
	TIM1->BDTR &= ~(0xFFu);
	TIM1->BDTR = (DEAD_TIME & 0xFFu) | TIM_BDTR_AOE;

	// Khoi dong 50% duty cycle
	TIM1->CCR1 = ccr1;
	TIM1->CCR2 = ccr2;
	TIM1->CCR3 = ccr3;

	TIM1->EGR |= TIM_EGR_UG;
	TIM1->SR  &= ~TIM_SR_UIF;

	NVIC_SetPriority(TIM1_UP_IRQn, 0);
	NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void TIM1_UP_IRQHandler(void)
{
	if (!(TIM1->SR & TIM_SR_UIF)) return;
	TIM1->SR &= ~TIM_SR_UIF;
	
	TIM1->CCR1 = ccr1;
	TIM1->CCR2 = ccr2;
	TIM1->CCR3 = ccr3;
	
	svpwm_flag = 1;
}

int main(void)
{
	SystemInit();
	GPIO_Config();
	TIM1_Config();
	
	SVPWM_Init(&svpwm, VDC, TS);

	svpwm.Vd = UD;
	svpwm.Vq = UQ;
	
	for (volatile uint32_t i = 0; i < 640000u; i++){};

	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->DIER |= TIM_DIER_UIE;
	TIM1->CR1  |= TIM_CR1_CEN;

	while (1) {
		if (svpwm_flag == 0) continue;
		svpwm_flag = 0;
		
		theta += (2.0f * (float)M_PI * SIN_FREQ * TS);
		
		if (theta >= 2.0f * (float)M_PI) {
			theta -= 2.0f * (float)M_PI;
		}
	
		SVPWM_Calculate(&svpwm, UD, UQ, theta, TIM1_ARR);
		
		ccr1 = svpwm.CCR1;
		ccr2 = svpwm.CCR2;
		ccr3 = svpwm.CCR3;
		
	}
}
