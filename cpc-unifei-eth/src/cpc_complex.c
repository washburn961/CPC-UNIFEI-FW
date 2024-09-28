#include "cpc_complex.h"
#include <math.h>

// Transform int to complex
complex_t float_transform(float a, float b) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    result.real = a;
    result.imag = b;
    return result;
}

// Basic operations
complex_t complex_add(complex_t a, complex_t b) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

complex_t complex_subtract(complex_t a, complex_t b) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    result.real = a.real - b.real;
    result.imag = a.imag - b.imag;
    return result;
}

complex_t complex_multiply(complex_t a, complex_t b) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

complex_t complex_divide(complex_t a, complex_t b) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    float denominator = b.real * b.real + b.imag * b.imag;
    result.real = (a.real * b.real + a.imag * b.imag) / denominator;
    result.imag = (a.imag * b.real - a.real * b.imag) / denominator;
    return result;
}

// Accessors
float complex_get_real(complex_t a) {
    return a.real;
}

float complex_get_imag(complex_t a) {
    return a.imag;
}

// Properties
float complex_get_magnitude(complex_t a) {
    return sqrtf(a.real * a.real + a.imag * a.imag);
}

float complex_get_angle(complex_t a) {
    return atan2f(a.imag, a.real);
}

complex_t complex_exponential(complex_t a) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    float exp_real = expf(a.real);
    result.real = exp_real * cosf(a.imag);
    result.imag = exp_real * sinf(a.imag);
    return result;
}

complex_t complex_exponential_zlt(complex_t a) {
    complex_t result;
    float angle = atan2f(a.imag, a.real);
    result.real = cosf(angle);
    result.imag = sinf(angle);
    return result;
}

complex_t complex_conjugate(complex_t a) {
    complex_t result = { 0.0f, 0.0f };  // Inicializa com 0.0f para segurança
    result.real = a.real;
    result.imag = -a.imag;
    return result;
}
