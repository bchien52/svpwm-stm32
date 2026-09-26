#ifndef SVPWM_H
#define SVPWM_H

#include <stdint.h>

#define SQRT3 1.7320508075688772f
#ifndef M_PI
#define M_PI  3.14159265358979323846f
#endif

typedef struct {
	float Vdc;
	float Ts;
	float Vq;
	float Vd;
	float Valpha;
	float Vbeta;
	float Vref1;
	float Vref2;
	float Vref3;
	float TX;
	float TY;
	float TZ;
	float T1;
	float T2;
	float T0;
	uint8_t sector;
	float theta;
	float Taon;
	float Tbon;
	float Tcon;
	uint32_t CCR1;
	uint32_t CCR2;
	uint32_t CCR3;
} SVPWM_t;

void SVPWM_Init(SVPWM_t *svpwm, float Vdc, float Ts);
void SVPWM_Calculate(SVPWM_t *svpwm, float Ud, float Uq, float theta, uint32_t TIM_ARR);
void SVPWM_InversePark(SVPWM_t *svpwm, float sin_t, float cos_t);
void SVPWM_ModifiedInverseClarke(SVPWM_t *svpwm);
uint8_t SVPWM_IdentifySector(SVPWM_t *svpwm);
void SVPWM_CalculateTXYZ(SVPWM_t *svpwm);
void SVPWM_DetermineT1T2(SVPWM_t *svpwm);
void SVPWM_Saturation(SVPWM_t *svpwm);
void SVPWM_CalculateSwitchingTimes (SVPWM_t *svpwm);
void SVPWM_PhaseAssignment(SVPWM_t *svpwm);
void SVPWM_ConvertToCCR(SVPWM_t *svpwm, uint32_t TIM_ARR);

#endif
