#include "sine_filter.h"

void SineFilter_Init(SineFilter* sineFilter)
{
    for (int i = 0; i < SINE_FILTER_SIZE; i++)
    {
        sineFilter->real[i] = 0;
        sineFilter->imag[i] = 0;
    }

    for (int h = 0; h < SINE_FILTER_SIZE; h++)
    {
        for (int n = 0; n < SINE_FILTER_SIZE; n++)
        {
            sineFilter->sin[h * SINE_FILTER_SIZE + n] = sinf((2 * M_PI * h * n) / SINE_FILTER_SIZE);
        }
    }
}

void SineFilter_Evaluate(SineFilter* sineFilter, float* input, uint32_t inputSize)
{
    if (inputSize > SINE_FILTER_SIZE)
    {
        inputSize = SINE_FILTER_SIZE;
    }

    for (int h = 0; h < SINE_FILTER_SIZE; h++)
    {
        float real = 0;
        float imag = 0;

        for (int n = 0; n < SINE_FILTER_SIZE; n++)
        {
            real += input[n + SINE_FILTER_SIZE / 4] * sineFilter->sin[h * SINE_FILTER_SIZE + n];
            imag += input[n] * sineFilter->sin[h * SINE_FILTER_SIZE + n];
        }

        sineFilter->real[h] = -2 * real / SINE_FILTER_SIZE;
        sineFilter->imag[h] = -2 * imag / SINE_FILTER_SIZE;
    }
}

float SineFilter_GetMag(SineFilter* sineFilter, uint32_t index)
{
    return sqrtf(powf(sineFilter->real[index], 2) + powf(sineFilter->imag[index], 2));
}

float SineFilter_GetPhase(SineFilter* sineFilter, uint32_t index)
{
    return atanf(sineFilter->imag[index] / sineFilter->real[index]);
}