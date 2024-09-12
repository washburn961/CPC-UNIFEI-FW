#ifndef FOURIER_TRANSFORM_H
#define FOURIER_TRANSFORM_H

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdint.h>

typedef struct
{
    float* real;
    float* imag;
    float* sin;
    float* cos;
    uint32_t size;
} fourier_transform;

uint32_t dft_init(fourier_transform* transform);
uint32_t dft_step(fourier_transform* transform, float* input);
uint32_t dft_get_magnitude(fourier_transform* transform, float* out_magnitude, uint32_t index);
uint32_t dft_get_phase(fourier_transform* transform, float* out_phase, uint32_t index);

#endif