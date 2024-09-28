#include "ANSI67.h"

#define _USE_MATH_DEFINES
#include <stdbool.h>
#include <math.h>
#include <cpc_complex.h>

#define DEG2RAD(x) ((x) * M_PI/ 180.0f)
#define RAD2DEG(x) ((x) * 180.0f/ M_PI)

float Z2_set_f; /* Forward negative sequence impedance */
float Z2_set_r; /* Reverse negative sequence impedance */
float Z0_set_f; /* Forward zero sequence impedance */
float Z0_set_r; /* Reverse zero sequence impedance */
complex_t v_ph;/* Phase voltage calculated*/
complex_t exp_120; /* e^j120° */
complex_t exp_240; /* e^j240° */


void Set_currents(Currents* element_i, complex_t current_phA, complex_t current_phB, complex_t current_phC) {
	element_i->current_ph[1] = current_phA;
	element_i->current_ph[2] = current_phB;
	element_i->current_ph[3] = current_phC;
	element_i->current_ph[4] = current_phA;
}

void Set_voltages(Voltages* element_v, complex_t voltage_phA, complex_t voltage_phB, complex_t voltage_phC) {
	element_v->voltage_ph[1] = voltage_phA;
	element_v->voltage_ph[2] = voltage_phB;
	element_v->voltage_ph[3] = voltage_phC;
	element_v->voltage_ph[4] = voltage_phA;
	element_v->voltage_ph[5] = voltage_phB;
}

void ANSI67_Init(ANSI67_Config* general, complex_t Zlt1, complex_t Zlt0, float RTC, float RTP, float iNom, float i_pickup, bool dir_ph, bool dir_seq, bool dir_imp) {
	general->RTP = RTP;
	general->RTC = RTC;
	complex_t ratio = { RTC / RTP, 0.0f };
	general->Zlt1 = complex_multiply(Zlt1, ratio);
	general->Zlt0 = complex_multiply(Zlt0, ratio);
	general->iNom = iNom;
	general->i_pickup = i_pickup;
	general->dir_ph = dir_ph;
	general->dir_seq = dir_seq;
	general->dir_imp = dir_imp;
}


static complex_t CalculatesSequenceValues(Currents* element_i, Voltages* element_v) {
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


unsigned int CalculatePhaseDirectional(ANSI67_Config* general, ANSI67* element, Currents* element_i, Voltages* element_v) {
	for (int f = 1; f < 4; f++) {
		v_ph = complex_subtract(element_v->voltage_ph[f + 1], element_v->voltage_ph[f + 2]);

		// abs(Vb - Vc) * abs(Ia) * cos ( ang(Vb-Bc) - ang(Ia) )
		element->T_ph[f] = complex_get_magnitude(v_ph) * complex_get_magnitude(element_i->current_ph[f]) * cos(complex_get_angle(v_ph) - complex_get_angle(element_i->current_ph[f]));
	}
}

unsigned int CalculateSequenceDirectional(ANSI67_Config* general, ANSI67* element, Currents* element_i, Voltages* element_v) {

	if (complex_get_magnitude(element_i->current_seq[1]) > 0.001) {
		// abs(3*V0) * abs(3*I0) * cos ( ang(-3*V0) - ( ang(3*I0) + ang(zlt0) ) )
		element->T_0 = complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_v->voltage_seq[1])) * complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[1])) * cos(complex_get_angle(complex_multiply(float_transform(-3.0, 0.0), element_v->voltage_seq[1])) - (complex_get_angle(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[1])) + complex_get_angle(general->Zlt0)));
	}
	else if ((complex_get_magnitude(element_i->current_ph[1]) > general->i_pickup && complex_get_magnitude(element_i->current_ph[2]) > general->i_pickup) || (complex_get_magnitude(element_i->current_ph[2]) > general->i_pickup && complex_get_magnitude(element_i->current_ph[3]) > general->i_pickup) || (complex_get_magnitude(element_i->current_ph[1]) > general->i_pickup && complex_get_magnitude(element_i->current_ph[3]) > general->i_pickup)) {
		// abs(3*V2) * abs(3*I2) * cos ( ang(-3*V2) - ( ang(3*I2) + ang(zlt1) ) )
		element->T_2 = complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_v->voltage_seq[3])) * complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[3])) * cos(complex_get_angle(complex_multiply(float_transform(-3.0, 0.0), element_v->voltage_seq[3])) - (complex_get_angle(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[3])) + complex_get_angle(general->Zlt1)));
	}
	else if (complex_get_magnitude(element_i->current_ph[1]) > general->i_pickup && complex_get_magnitude(element_i->current_ph[2]) > general->i_pickup && complex_get_magnitude(element_i->current_ph[3]) > general->i_pickup) {
		// abs(3*V1) * abs(3*I1) * cos ( ang(3*V1) - ( ang(3*I1) + ang(zlt1) ) )
		element->T_1 = complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_v->voltage_seq[2])) * complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[2])) * cos(complex_get_angle(complex_multiply(float_transform(3.0, 0.0), element_v->voltage_seq[2])) - (complex_get_angle(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[2])) + complex_get_angle(general->Zlt1)));
	}
}

unsigned int CalculateImpedanceDirectional(ANSI67_Config* general, ANSI67* element, Currents* element_i, Voltages* element_v) {
	Z2_set_f = 0.5 * complex_get_magnitude(general->Zlt1);
	Z2_set_r = Z2_set_f + (0.5 / general->iNom);

	element->Z2 = complex_get_real(complex_multiply(element_v->voltage_seq[3], complex_conjugate(complex_multiply(element_i->current_seq[3], complex_exponential_zlt(general->Zlt1))))) / (complex_get_magnitude(element_i->current_seq[3]) * complex_get_magnitude(element_i->current_seq[3]));

	if (Z2_set_f <= 0) {
		element->Z2_f = 0.75 * Z2_set_f - (0.25 * complex_get_magnitude(complex_divide(element_v->voltage_seq[3], element_i->current_seq[3])));
	}
	else if (Z2_set_f > 0) {
		element->Z2_f = 1.25 * Z2_set_f - (0.25 * complex_get_magnitude(complex_divide(element_v->voltage_seq[3], element_i->current_seq[3])));
	}

	if (Z2_set_r >= 0) {
		element->Z2_r = 0.75 * Z2_set_r + (0.25 * complex_get_magnitude(complex_divide(element_v->voltage_seq[3], element_i->current_seq[3])));
	}
	else if (Z2_set_r < 0) {
		element->Z2_r = 1.25 * Z2_set_r + (0.25 * complex_get_magnitude(complex_divide(element_v->voltage_seq[3], element_i->current_seq[3])));
	}



	Z0_set_f = 0.5 * complex_get_magnitude(general->Zlt0);
	Z0_set_r = Z0_set_f + (0.5 / general->iNom);


	element->Z0 = complex_get_real(complex_multiply(complex_multiply(float_transform(3.0, 0.0), element_v->voltage_seq[1]), complex_conjugate(complex_multiply(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[1]), complex_exponential_zlt(general->Zlt1))))) / (complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[1])) * complex_get_magnitude(complex_multiply(float_transform(3.0, 0.0), element_i->current_seq[1])));


	complex_t v03_i03 = complex_divide(complex_multiply(element_v->voltage_seq[1], float_transform(3.0, 0.0)), complex_multiply(element_i->current_seq[1], float_transform(3.0, 0.0)));

	if (Z0_set_f <= 0) {
		element->Z0_f = 0.75 * Z0_set_f - (0.25 * complex_get_magnitude(v03_i03));
	}
	else if (Z0_set_f > 0) {
		element->Z0_f = 1.25 * Z0_set_f - (0.25 * complex_get_magnitude(v03_i03));
	}

	if (Z0_set_r >= 0) {
		element->Z0_r = 0.75 * Z0_set_r + (0.25 * complex_get_magnitude(v03_i03));
	}
	else if (Z0_set_r < 0) {
		element->Z0_r = 1.25 * Z0_set_r + (0.25 * complex_get_magnitude(v03_i03));
	}

}

void ANSI67_Step(ANSI67* element, ANSI67_Config* general, Currents* element_i, Voltages* element_v) {
	CalculatesSequenceValues(element_i, element_v);
	if (general->dir_ph == true) {
		CalculatePhaseDirectional(general, element, element_i, element_v);
		for (int f = 1; f < 4; f++) {
			if (element->T_ph[f] > 0) {
				element->pk_dir_ph[f] = 1;
			}
		}
	}
	if (general->dir_seq == true) {
		CalculateSequenceDirectional(general, element, element_i, element_v);
		if (element->T_0 > 0 || element->T_2 > 0 || element->T_1 > 0) {
			element->pk_dir_seq = 1;
		}
	}
	if (general->dir_imp == true) {
		CalculateImpedanceDirectional(general, element, element_i, element_v);
		if (element->Z2 < Z2_set_f) {
			element->pk_dir_Z2 = 1;
		}
		else if (element->Z2 > Z2_set_r) {
			element->pk_dir_Z2 = 0;
		}

		if (!element->pk_dir_Z2) {
			if (element->Z0 < Z0_set_f) {
				element->pk_dir_Z0 = 1;
			}
			else if (element->Z0 > Z0_set_r) {
				element->pk_dir_Z0 = 0;
			}
		}


		if (element->pk_dir_Z2 || element->pk_dir_Z0) {
			element->pk_dir_imp = 1;
		}
	}
}


void ANSI67_Reset(ANSI67* element) {
	if (!element->is_pickup) return;
	element->is_pickup = 0;
}

