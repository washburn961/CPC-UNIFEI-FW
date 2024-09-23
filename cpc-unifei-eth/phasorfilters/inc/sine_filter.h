#ifndef SINE_FILTER_H
#define SINE_FILTER_H

#define _USE_MATH_DEFINES

#include "phasor_filters_conf.h"
#include <complex.h>
#include <math.h>
#include <stdint.h>

typedef struct
{
    float real[SINE_FILTER_SIZE];
    float imag[SINE_FILTER_SIZE];
    float sin[SINE_FILTER_SIZE * SINE_FILTER_SIZE];
} SineFilter;

void SineFilter_Init(SineFilter* sineFilter);
void SineFilter_Evaluate(SineFilter* sineFilter, float* input, uint32_t inputSize);
float SineFilter_GetMag(SineFilter* sineFilter, uint32_t index);
float SineFilter_GetPhase(SineFilter* sineFilter, uint32_t index);

#endif