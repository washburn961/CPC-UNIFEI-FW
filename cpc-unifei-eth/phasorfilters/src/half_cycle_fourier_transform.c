#include "half_cycle_fourier_transform.h"

void HalfCycleFourierTransform_Init(HalfCycleFourierTransform* halfCycleFourierTransform)
{
    for (int i = 0; i < HCDFT_SIZE; i++)
    {
        halfCycleFourierTransform->real[i] = 0;
        halfCycleFourierTransform->imag[i] = 0;
    }

    for (int h = 0; h < HCDFT_SIZE; h++)
    {
        for (int n = 0; n < HCDFT_SIZE / 2; n++)
        {
            halfCycleFourierTransform->sin[h * HCDFT_SIZE / 2 + n] = sinf((2 * M_PI * h * n) / HCDFT_SIZE);
            halfCycleFourierTransform->cos[h * HCDFT_SIZE / 2 + n] = cosf((2 * M_PI * h * n) / HCDFT_SIZE);
        }
    }
}
void HalfCycleFourierTransform_Evaluate(HalfCycleFourierTransform* halfCycleFourierTransform, float* input, uint32_t inputSize)
{
    if (inputSize > HCDFT_SIZE)
    {
        inputSize = HCDFT_SIZE;
    }

    for (int h = 0; h < HCDFT_SIZE; h++)
    {
        float real = 0;
        float imag = 0;

        for (int n = 0; n < HCDFT_SIZE / 2; n++)
        {
            real += input[n] * halfCycleFourierTransform->cos[h * HCDFT_SIZE / 2 + n];
            imag += input[n] * halfCycleFourierTransform->sin[h * HCDFT_SIZE / 2 + n];
        }

        halfCycleFourierTransform->real[h] = 4 * real / HCDFT_SIZE;
        halfCycleFourierTransform->imag[h] = -4 * imag / HCDFT_SIZE;
    }
}

float HalfCycleFourierTransform_GetMag(HalfCycleFourierTransform* halfCycleFourierTransform, uint32_t index)
{
    return sqrtf(powf(halfCycleFourierTransform->real[index], 2) + powf(halfCycleFourierTransform->imag[index], 2));
}

float HalfCycleFourierTransform_GetPhase(HalfCycleFourierTransform* halfCycleFourierTransform, uint32_t index)
{
    return atanf(halfCycleFourierTransform->imag[index] / halfCycleFourierTransform->real[index]);
}

void HalfCycleFourierTransform_GetMagBin(HalfCycleFourierTransform* halfCycleFourierTransform, float* out_magBin)
{
    for (uint32_t i = 0; i < HCDFT_SIZE; i++)
    {
        out_magBin[i] = HalfCycleFourierTransform_GetMag(halfCycleFourierTransform, i);
    }
}