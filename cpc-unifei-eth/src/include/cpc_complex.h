#ifndef CPC_COMPLEX_H
#define CPC_COMPLEX_H

typedef struct {
    float real;
    float imag;
} complex_t;

// Transform int to complex
complex_t float_transform(float a, float b);  

// Basic operations
complex_t complex_add(complex_t a, complex_t b);
complex_t complex_subtract(complex_t a, complex_t b);
complex_t complex_multiply(complex_t a, complex_t b);
complex_t complex_divide(complex_t a, complex_t b);

// Accessors
float complex_get_real(complex_t a);
float complex_get_imag(complex_t a);

// Properties
float complex_get_magnitude(complex_t a);
float complex_get_angle(complex_t a);
complex_t complex_exponential(complex_t a);
complex_t complex_exponential_zlt(complex_t a);
complex_t complex_conjugate(complex_t a);

#endif // CPC_COMPLEX_H
