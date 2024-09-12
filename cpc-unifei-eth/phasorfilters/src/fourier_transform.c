#include "fourier_transform.h"

uint32_t dft_init(fourier_transform* transform) {
	uint32_t ret = 0;

	for (uint32_t i = 0; i < transform->size; i++)
	{
		transform->real[i] = 0;
		transform->imag[i] = 0;
	}

	for (uint32_t h = 0; h < transform->size; h++)
	{
		for (uint32_t n = 0; n < transform->size; n++)
		{
			transform->sin[h * transform->size + n] = sinf((2 * M_PI * h * n) / transform->size);
			transform->cos[h * transform->size + n] = cosf((2 * M_PI * h * n) / transform->size);
		}
	}

	return ret;
}

uint32_t dft_step(fourier_transform* transform, float* input) {
	uint32_t ret = 0;

	for (uint32_t h = 0; h < transform->size; h++)
	{
		float real = 0;
		float imag = 0;

		for (uint32_t n = 0; n < transform->size; n++)
		{
			real += input[n] * transform->cos[h * transform->size + n];
			imag += input[n] * transform->sin[h * transform->size + n];
		}

		transform->real[h] = 2 * real / transform->size;
		transform->imag[h] = -2 * imag / transform->size;
	}

	return ret;
}

uint32_t dft_get_magnitude(fourier_transform* transform, float* out_magnitude, uint32_t index) {
	uint32_t ret = 0;

	if (index >= transform->size) return 1;

	*out_magnitude = sqrtf(powf(transform->real[index], 2) + powf(transform->imag[index], 2));

	return ret;
}

uint32_t dft_get_phase(fourier_transform* transform, float* out_phase, uint32_t index) {
	uint32_t ret = 0;

	if (index >= transform->size) return 1; // Check bounds

	*out_phase = atan2f(transform->imag[index], transform->real[index]);

	return ret;
}