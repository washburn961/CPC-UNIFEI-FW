#ifndef HALF_CYCLE_FOURIER_TRANSFORM_H
#define HALF_CYCLE_FOURIER_TRANSFORM_H

#define _USE_MATH_DEFINES

#include "phasor_filters_conf.h"
#include <complex.h>
#include <math.h>
#include <stdint.h>

typedef struct
{
    float real[HCDFT_SIZE];
    float imag[HCDFT_SIZE];
    float sin[(HCDFT_SIZE) * (HCDFT_SIZE / 2)];
    float cos[(HCDFT_SIZE) * (HCDFT_SIZE / 2)];
} HalfCycleFourierTransform;

void HalfCycleFourierTransform_Init(HalfCycleFourierTransform* fourierTransform);
void HalfCycleFourierTransform_Evaluate(HalfCycleFourierTransform* fourierTransform, float* input, uint32_t inputSize);
float HalfCycleFourierTransform_GetMag(HalfCycleFourierTransform* fourierTransform, uint32_t index);
float HalfCycleFourierTransform_GetPhase(HalfCycleFourierTransform* fourierTransform, uint32_t index);
void HalfCycleFourierTransform_GetMagBin(HalfCycleFourierTransform* fourierTransform, float* out_magBin);

#endif