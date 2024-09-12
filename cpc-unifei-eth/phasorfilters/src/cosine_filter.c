#include "cosine_filter.h"

void CosineFilter_Init(CosineFilter* cosineFilter)
{
    for (int i = 0; i < COSINE_FILTER_SIZE; i++)
    {
        cosineFilter->real[i] = 0;
        cosineFilter->imag[i] = 0;
    }

    for (int h = 0; h < COSINE_FILTER_SIZE; h++)
    {
        for (int n = 0; n < COSINE_FILTER_SIZE; n++)
        {
            cosineFilter->cos[h * COSINE_FILTER_SIZE + n] = cosf((2 * M_PI * h * n) / COSINE_FILTER_SIZE);
        }
    }
}

void CosineFilter_Evaluate(CosineFilter* cosineFilter, float* input, uint32_t inputSize)
{
    if (inputSize > COSINE_FILTER_SIZE)
    {
        inputSize = COSINE_FILTER_SIZE;
    }

    for (int h = 0; h < COSINE_FILTER_SIZE; h++)
    {
        float real = 0;
        float imag = 0;

        for (int n = 0; n < COSINE_FILTER_SIZE; n++)
        {
            real += input[n] * cosineFilter->cos[h * COSINE_FILTER_SIZE + n];
            imag += input[n + COSINE_FILTER_SIZE / 4] * cosineFilter->cos[h * COSINE_FILTER_SIZE + n];
        }

        cosineFilter->real[h] = 2 * real / COSINE_FILTER_SIZE;
        cosineFilter->imag[h] = -2 * imag / COSINE_FILTER_SIZE;
    }
}

float CosineFilter_GetMag(CosineFilter* cosineFilter, uint32_t index)
{
    return sqrtf(powf(cosineFilter->real[index], 2) + powf(cosineFilter->imag[index], 2));
}

float CosineFilter_GetPhase(CosineFilter* cosineFilter, uint32_t index)
{
    return atanf(cosineFilter->imag[index] / cosineFilter->real[index]);
}