#ifndef COSINE_FILTER_H
#define COSINE_FILTER_H

#define _USE_MATH_DEFINES

#include "phasor_filters_conf.h"
#include <complex.h>
#include <math.h>
#include <stdint.h>

typedef struct
{
    float real[COSINE_FILTER_SIZE];
    float imag[COSINE_FILTER_SIZE];
    float cos[COSINE_FILTER_SIZE * COSINE_FILTER_SIZE];
} CosineFilter;

void CosineFilter_Init(CosineFilter* cosineFilter);
void CosineFilter_Evaluate(CosineFilter* cosineFilter, float* input, uint32_t inputSize);
float CosineFilter_GetMag(CosineFilter* cosineFilter, uint32_t index);
float CosineFilter_GetPhase(CosineFilter* cosineFilter, uint32_t index);

#endif