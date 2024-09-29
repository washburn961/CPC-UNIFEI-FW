#include "ansi21.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cpc_complex.h>

#define DEG2RAD(x) ((x) * M_PI/ 180.0f)

// 
complex_t exp_120;
complex_t exp_240;
complex_t angle_Zlt1;
complex_t K0_mult_3;
complex_t i_pol;
float numerator;
float denominator;
float numerator_ft;
float numerator_ff;
float denominator_ft;
float denominator_ff;

/*initialization functions*/

void ANSI21_Init_param(ANSI21_Config* element, complex_t Zlt1, complex_t Zlt0, float iNom, float RTP, float RTC, complex_t K0) {
	element->RTP = RTP;
	element->RTC = RTC;
	complex_t ratio = { RTC / RTP, 0.0f };
	element->Zlt1 = complex_multiply(Zlt1, ratio);
	element->Zlt0 = complex_multiply(Zlt0, ratio);
	element->iNom = iNom;
	complex_t three = { 3.0f,0.0f };
	element->K0 = complex_divide(complex_subtract(element->Zlt0, element->Zlt1), complex_multiply(three, element->Zlt1));
}

void Set_current(Current* element, complex_t current_phA, complex_t current_phB, complex_t current_phC) {
	element->current_ph[1] = current_phA;
	element->current_ph[2] = current_phB;
	element->current_ph[3] = current_phC;
	element->current_ph[4] = current_phA;
}

void Set_voltage(Voltage* element, complex_t voltage_phA, complex_t voltage_phB, complex_t voltage_phC) {
	element->voltage_ph[1] = voltage_phA;
	element->voltage_ph[2] = voltage_phB;
	element->voltage_ph[3] = voltage_phC;
	element->voltage_ph[4] = voltage_phA;
}

void ANSI21_Init_Mp(ANSI21_Mp* element, float Zr_Mp_1, float Zr_Mp_2, float Tr_Mp_2, float time_step) {
	element->Zr_1 = Zr_Mp_1;
	element->Zr_2 = Zr_Mp_2;
	element->Tr_2 = Tr_Mp_2;
	element->m[3] = 0;
	element->time_step = time_step;
	element->time = 0;
	element->pk_zone1 = 0;
	element->pk_zone2 = 0;
	element->is_tripped = 0;
}

void ANSI21_Init_Mg(ANSI21_Mg* element, float Zr_Mg_1, float Zr_Mg_2, float Tr_Mg_2, float time_step) {
	element->Zr_1 = Zr_Mg_1;
	element->Zr_2 = Zr_Mg_2;
	element->Tr_2 = Tr_Mg_2;
	element->m[3] = 0;
	element->time_step = time_step;
	element->time = 0;
	element->pk_zone1 = 0;
	element->pk_zone2 = 0;
	element->is_tripped = 0;
}

void ANSI21_Init_Qg(ANSI21_Qg* element, float Zr_Qg_1, float Zr_Qg_2, float Br_Qg_1, float Br_Qg_2, float Tr_Qg_2, float time_step) {
	element->Zr_1 = Zr_Qg_1;
	element->Zr_2 = Zr_Qg_2;
	element->Br_1 = Br_Qg_1;
	element->Br_2 = Br_Qg_2;
	element->Tr_2 = Tr_Qg_2;
	element->m[3] = 0;
	element->resis_ft[3] = 0;
	element->resis_ff[3] = 0;
	element->time_step = time_step;
	element->time = 0;
	element->pk_zone1 = 0;
	element->pk_zone2 = 0;
	element->is_tripped = 0;
}



/*internal calculation functions*/

static complex_t CalculatesSequenceValues(Current* element_i, Voltage* element_v) {
	// Calculating the terms exp(j*120°) and exp(j*240°)
	exp_120 = complex_exponential(float_transform(0.0f, DEG2RAD(120.0f)));
	exp_240 = complex_exponential(float_transform(0.0f, DEG2RAD(240.0f)));

	// Calculating sequence phasors with the synthesis matrix
	element_i->current_seq[1] = complex_multiply(float_transform(1.0f / 3.0f, 0.0f), complex_add(complex_add(element_i->current_ph[1], element_i->current_ph[2]), element_i->current_ph[3]));
	element_i->current_seq[2] = complex_multiply(float_transform(1.0f / 3.0f, 0.0f), complex_add(element_i->current_ph[1], complex_add(complex_multiply(exp_120, element_i->current_ph[2]), complex_multiply(exp_240, element_i->current_ph[3]))));
	element_i->current_seq[3] = complex_multiply(float_transform(1.0f / 3.0f, 0.0f), complex_add(element_i->current_ph[1], complex_add(complex_multiply(exp_240, element_i->current_ph[2]), complex_multiply(exp_120, element_i->current_ph[3]))));

	element_v->voltage_seq[1] = complex_multiply(float_transform(1.0f / 3.0f, 0.0f), complex_add(complex_add(element_v->voltage_ph[1], element_v->voltage_ph[2]), element_v->voltage_ph[3]));
	element_v->voltage_seq[2] = complex_multiply(float_transform(1.0f / 3.0f, 0.0f), complex_add(element_v->voltage_ph[1], complex_add(complex_multiply(exp_120, element_v->voltage_ph[2]), complex_multiply(exp_240, element_v->voltage_ph[3]))));
	element_v->voltage_seq[3] = complex_multiply(float_transform(1.0f / 3.0f, 0.0f), complex_add(element_v->voltage_ph[1], complex_add(complex_multiply(exp_240, element_v->voltage_ph[2]), complex_multiply(exp_120, element_v->voltage_ph[3]))));
}

static float CalculateCharacteristicMapping_Mp(ANSI21_Mp* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {
	angle_Zlt1 = float_transform(0.0, complex_get_angle(general->Zlt1));
	for (int f = 1; f < 3; f++) {

		// real( (Va-Vb) * conj(Va-Vb) )
		numerator = complex_get_real(complex_multiply(complex_subtract(element_v->voltage_ph[f], element_v->voltage_ph[f + 1]), complex_conjugate(complex_subtract(element_v->voltage_ph[f], element_v->voltage_ph[f + 1]))));

		// real( e^(j * ang(Zlt1)) * (Ia-Ib) * conj(Va-Vb) )
		denominator = complex_get_real(complex_multiply(complex_multiply(complex_exponential(angle_Zlt1), complex_subtract(element_i->current_ph[f], element_i->current_ph[f + 1])), complex_conjugate(complex_subtract(element_v->voltage_ph[f], element_v->voltage_ph[f + 1]))));

		element->m[f] = (numerator / denominator) / complex_get_magnitude(general->Zlt1);

		if (element->m[f] < element->Zr_1 && element->m[f] > 0) {
			element->pk_zone1 = 1;
		}
		else if (element->m[f] < element->Zr_2 && element->m[f] > element->Zr_1) {
			element->pk_zone2 = 1;
		}
	}
}

static float CalculateCharacteristicMapping_Mg(ANSI21_Mg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {
	angle_Zlt1 = float_transform(0.0, complex_get_angle(general->Zlt1));
	K0_mult_3 = complex_multiply(general->K0, float_transform(3.0, 0.0));

	for (int f = 1; f < 3; f++) {

		// real( (Va) * conj(Va) )
		numerator = complex_get_real(complex_multiply(element_v->voltage_ph[f], complex_conjugate(element_v->voltage_ph[f])));

		// real( e^(j * ang(Zlt1)) * (Ia+ K0*3*I0) * conj(Va) )
		denominator = complex_get_real(complex_multiply(complex_multiply(complex_exponential(angle_Zlt1), complex_add(element_i->current_ph[f], complex_multiply(K0_mult_3, element_i->current_seq[1]))), complex_conjugate(element_v->voltage_ph[f])));

		element->m[f] = (numerator / denominator) / complex_get_magnitude(general->Zlt1);

		if (element->m[f] < element->Zr_1 && element->m[f] > 0) {
			element->pk_zone1 = 1;
		}
		else if (element->m[f] < element->Zr_2 && element->m[f] > element->Zr_1) {
			element->pk_zone2 = 1;
		}
	}
}

static float CalculateCharacteristicMapping_Qg(ANSI21_Qg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {
	angle_Zlt1 = float_transform(0.0, complex_get_angle(general->Zlt1));
	K0_mult_3 = complex_multiply(general->K0, float_transform(3.0, 0.0));

	for (int f = 1; f < 3; f++) {

		if (complex_get_magnitude(element_i->current_seq[1]) > 0.0001) {
			i_pol = complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[1]); /*zero sequence polarization (Ig=3*I0)*/
		}
		else if (complex_get_magnitude(element_i->current_seq[3]) > 0.0001) {
			i_pol = element_i->current_seq[3]; /*negative sequence polarization*/
		}
		else {
			i_pol = element_i->current_seq[2]; /*positive sequence polarization*/
		}

		// imag( (Va) * conj(i_pol) )
		numerator = complex_get_imag(complex_multiply(element_v->voltage_ph[f], complex_conjugate(i_pol)));

		// imag( e^(j * ang(Zlt1)) * (Ia + K0*3*I0) * conj(i_pol) )
		denominator = complex_get_imag(complex_multiply(complex_multiply(complex_exponential(angle_Zlt1), complex_add(element_i->current_ph[f], complex_multiply(K0_mult_3, element_i->current_seq[1]))), complex_conjugate(i_pol)));

		element->m[f] = (numerator / denominator) / complex_get_magnitude(general->Zlt1);

		if (element->m[f] < element->Zr_1 && element->m[f] > 0) {
			return 1.0;
		}
		else if (element->m[f] < element->Zr_2 && element->m[f] > element->Zr_1) {
			return 2.0;
		}
	}
}

static float CalculateResistance_Qg(ANSI21_Qg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {
	angle_Zlt1 = float_transform(0.0, complex_get_angle(general->Zlt1));
	K0_mult_3 = complex_multiply(general->K0, float_transform(3.0, 0.0));

	for (int f = 1; f < 3; f++) {

		// conj( e^(j * ang(Zlt1)) * (Ia +  3*K0*I0) ) 
		complex_t conjugate_ft = complex_conjugate(complex_multiply(complex_exponential(angle_Zlt1), complex_add(element_i->current_ph[f], complex_multiply(K0_mult_3, element_i->current_seq[1]))));

		// conj( e^(j * ang(Zlt1)) * (Ia - Ib)
		complex_t conjugate_ff = complex_conjugate(complex_multiply(complex_exponential(angle_Zlt1), complex_subtract(element_i->current_ph[f], element_i->current_ph[f + 1])));


		// imag( (Va) * conj( e^(j * ang(Zlt1)) * (Ia +  3*K0*I0) ) )
		numerator_ft = complex_get_imag(complex_multiply(element_v->voltage_ph[f], conjugate_ft));

		// imag( (Va-vb) * conj( e^(j * ang(Zlt1)) * (Ia - Ib) ) )
		numerator_ff = complex_get_imag(complex_multiply(complex_subtract(element_v->voltage_ph[f], element_v->voltage_ph[f + 1]), conjugate_ff));


		// imag( 1.5 * (I2 + I0) * conj( e^(j * ang(Zlt1)) * (Ia +  3*K0*I0) ) )
		denominator_ft = complex_get_imag(complex_multiply(complex_multiply(float_transform(1.5f, 0.0f), complex_add(element_i->current_seq[3], element_i->current_seq[1])), conjugate_ft));

		// imag(1.5 * (I2 + I0) * conj(e ^ (j * ang(Zlt1)) * (Ia + 3 * K0 * I0)))
		denominator_ff = complex_get_imag(complex_multiply(complex_multiply(float_transform(1.5f, 0.0f), complex_add(element_i->current_seq[3], element_i->current_seq[1])), conjugate_ff));


		element->resis_ft[f] = (numerator_ft / denominator_ft);
		element->resis_ff[f] = (numerator_ff / denominator_ff);

		if (element->resis_ft[f] <= element->Br_1 && element->resis_ft[f] >= -element->Br_1) {
			return 1.0;
		}
		else if (element->resis_ft[f] <= element->Br_2 && element->resis_ft[f] >= -element->Br_2) {
			return 2.0;
		}
	}
}



/*processing functions of each characteristc*/

void ANSI21_Step_Mp(ANSI21_Mp* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {

}

void ANSI21_Step_Mg(ANSI21_Mg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {

}

void ANSI21_Step_Qg(ANSI21_Qg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general) {

}



/*reset functions of each characteristc*/

void ANSI21_Reset_Mp(ANSI21_Mp* element) {
	if (!element->is_tripped) return;
	element->is_tripped = 0;
}

void ANSI21_Reset_Mg(ANSI21_Mg* element) {
	if (!element->is_tripped) return;
	element->is_tripped = 0;
}

void ANSI21_Reset_Qg(ANSI21_Qg* element) {
	if (!element->is_tripped) return;
	element->is_tripped = 0;
}