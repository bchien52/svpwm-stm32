#include "svpwm.h"
#include <math.h>

void SVPWM_Init(SVPWM_t *svpwm, float Vdc, float Ts)
{
	svpwm->Vdc = Vdc;
	svpwm->Ts = Ts;
	svpwm->Vref1 = 0.0f;
	svpwm->Vref2 = 0.0f;
	svpwm->Vref3 = 0.0f;
	svpwm->TX = 0.0f;
	svpwm->TY = 0.0f;
	svpwm->TZ = 0.0f;
	svpwm->T1 = 0.0f;
	svpwm->T2 = 0.0f;
	svpwm->T0 = 0.0f;
	svpwm->sector = 0u;
	svpwm->Taon = 0.0f;
	svpwm->Tbon = 0.0f;
	svpwm->Tcon = 0.0f;
	svpwm->CCR1 = 0u;
	svpwm->CCR2 = 0u;
	svpwm->CCR3 = 0u;
}

void SVPWM_Calculate(SVPWM_t *svpwm, float Ud, float Uq, float theta, uint32_t TIM_ARR)
{
	svpwm->Vd = Ud;
	svpwm->Vq = Uq;

	float sin_t = sinf(theta);
	float cos_t = cosf(theta);
	
	SVPWM_InversePark(svpwm, sin_t, cos_t);
	SVPWM_ModifiedInverseClarke(svpwm);
	SVPWM_IdentifySector(svpwm);
	SVPWM_CalculateTXYZ(svpwm);
	SVPWM_DetermineT1T2(svpwm);
	SVPWM_Saturation(svpwm);
	SVPWM_CalculateSwitchingTimes(svpwm);
	SVPWM_PhaseAssignment(svpwm);
	SVPWM_ConvertToCCR(svpwm, TIM_ARR);
}

void SVPWM_InversePark(SVPWM_t *svpwm, float sin_t, float cos_t)
{
	svpwm->Valpha = svpwm->Vd * cos_t - svpwm->Vq * sin_t;
	svpwm->Vbeta  = svpwm->Vd * sin_t + svpwm->Vq * cos_t;
}

void SVPWM_ModifiedInverseClarke(SVPWM_t *svpwm)
{
	svpwm->Vref1 = svpwm->Vbeta;
	svpwm->Vref2 = -0.5f * svpwm->Vbeta + (SQRT3 / 2.0f) * svpwm->Valpha;
	svpwm->Vref3 = -0.5f * svpwm->Vbeta - (SQRT3 / 2.0f) * svpwm->Valpha;	
}
uint8_t SVPWM_IdentifySector(SVPWM_t *svpwm)
{
	uint8_t a = (svpwm->Vref1 > 0.0f) ? 1u : 0u;
	uint8_t b = (svpwm->Vref2 > 0.0f) ? 1u : 0u;
	uint8_t c = (svpwm->Vref3 > 0.0f) ? 1u : 0u;
	uint8_t _sector = a + 2u*b + 4u*c;

	switch(_sector) {
		case 1u: svpwm->sector = 1u; break;  // S3
		case 3u: svpwm->sector = 2u; break;  // S1
		case 2u: svpwm->sector = 3u; break;  // S5
		case 6u: svpwm->sector = 4u; break;  // S4
		case 4u: svpwm->sector = 5u; break;  // S6
		case 5u: svpwm->sector = 6u; break;  // S2
		default: svpwm->sector = 1u; break;
	}
	return svpwm->sector;
}

void SVPWM_CalculateTXYZ(SVPWM_t *svpwm)
{
	if (svpwm->Vdc < 1e-6f) {
    svpwm->TX = 0.0f;
    svpwm->TY = 0.0f;
    svpwm->TZ = 0.0f;
    return;
  }
  
	float K = SQRT3 / svpwm->Vdc;

	svpwm->TX =  svpwm->Vref1 * K;
	svpwm->TY = -svpwm->Vref3 * K;
	svpwm->TZ = -svpwm->Vref2 * K;
}

void SVPWM_DetermineT1T2(SVPWM_t *svpwm)
{

	switch (svpwm->sector) {
		case 1u:  // S3
			svpwm->T1 = -svpwm->TZ;
			svpwm->T2 = svpwm->TX;
			break;
		case 2u:  // S1
			svpwm->T1 = svpwm->TZ;
			svpwm->T2 = svpwm->TY;
			break;
		case 3u:  // S5
			svpwm->T1 = svpwm->TX;
			svpwm->T2 = -svpwm->TY;
			break;
		case 4u:  // S4
			svpwm->T1 = -svpwm->TX;
			svpwm->T2 = svpwm->TZ;
			break;
		case 5u:  // S6
			svpwm->T1 = -svpwm->TY;
			svpwm->T2 = -svpwm->TZ;
			break;
		case 6u:  // S2
			svpwm->T1 =  svpwm->TY;
			svpwm->T2 = -svpwm->TX  ;
			break;
		default:
			svpwm->T1 = 0.0f;
			svpwm->T2 = 0.0f;
			break;
	}
}

void SVPWM_Saturation(SVPWM_t *svpwm)
{
	float sum = svpwm->T1 + svpwm->T2;

	if (sum > 1.0f) {
		svpwm->T1 = svpwm->T1 / sum;
		svpwm->T2 = svpwm->T2 / sum;
		svpwm->T0 = 0.0f;
	} else {
		svpwm->T0 = 1.0f - svpwm->T1 - svpwm->T2;
	}
}

void SVPWM_CalculateSwitchingTimes(SVPWM_t *svpwm)
{
	svpwm->Taon = svpwm->T0 * 0.5f;
	svpwm->Tbon = svpwm->Taon + svpwm->T1;
	svpwm->Tcon = svpwm->Tbon + svpwm->T2;
}

void SVPWM_PhaseAssignment(SVPWM_t *svpwm)
{
	float C1, C2, C3;

	switch (svpwm->sector) {
		case 1u:  // S3
			C1 = svpwm->Taon; C2 = svpwm->Tbon; C3 = svpwm->Tcon; break;
		case 2u:  // S1
			C1 = svpwm->Tbon; C2 = svpwm->Taon; C3 = svpwm->Tcon; break;
		case 3u:  // S5
			C1 = svpwm->Tcon; C2 = svpwm->Taon; C3 = svpwm->Tbon; break;
		case 4u:  // S4
			C1 = svpwm->Tcon; C2 = svpwm->Tbon; C3 = svpwm->Taon; break;
		case 5u:  // S6
			C1 = svpwm->Tbon; C2 = svpwm->Tcon; C3 = svpwm->Taon; break;
		case 6u:  // S2
			C1 = svpwm->Taon; C2 = svpwm->Tcon; C3 = svpwm->Tbon; break;
		default:
			C1 = C2 = C3 = svpwm->Taon; break;
	}

	svpwm->Taon = C1;
	svpwm->Tbon = C2;
	svpwm->Tcon = C3;
}

void SVPWM_ConvertToCCR(SVPWM_t *svpwm, uint32_t TIM_ARR)
{
	svpwm->CCR1 = (uint32_t)(svpwm->Taon * (float)TIM_ARR);
	svpwm->CCR2 = (uint32_t)(svpwm->Tbon * (float)TIM_ARR);
	svpwm->CCR3 = (uint32_t)(svpwm->Tcon * (float)TIM_ARR);

	if (svpwm->CCR1 > TIM_ARR) svpwm->CCR1 = TIM_ARR;
	if (svpwm->CCR2 > TIM_ARR) svpwm->CCR2 = TIM_ARR;
	if (svpwm->CCR3 > TIM_ARR) svpwm->CCR3 = TIM_ARR;
}
