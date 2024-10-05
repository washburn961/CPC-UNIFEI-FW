#include "ANSI87B.h"

void ANSI87B_Init(ANSI87B* element, float ipk, float maxpu, float x1, float m1, float m2) {
	element->ipk = ipk;
	element->maxpu = maxpu;
	element->x1 = x1;
	element->m1 = m1;
	element->m2 = m2;
	element->trip = 0;

	memset(element->current, 0, sizeof(element->current));
}

void SumOfPhasors(ANSI87B* element, float* sum) {

	for (int j = 0; j < 6; j++) {
		for (int i = 0; i < terminals; i++) {
			sum[j] += element->current[i][j];
		}
	}
}

void PhasorAbs(float* phasors, float* abs) {
	for (int i = 0; i < 6; i = i + 2) {
		abs[i / 2] = sqrt(pow(phasors[i], 2) + pow(phasors[i + 1], 2));
	}
}

void GetOperatingCurrent(ANSI87B* element, float* iop) {
	float sum[6] = { 0 };

	SumOfPhasors(element, sum);
	PhasorAbs(sum, iop);
}

void GetRestraintCurrent(ANSI87B* element, float* irst) {
	float abs[terminals][3];
	float aux[6];

	for (int i = 0; i < terminals; i++) {
		PhasorAbs(element->current[i], aux);

		for (int j = 0; j < 3; j++) {
			abs[i][j] = aux[j];
		}
	}

	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < terminals; i++) {
			irst[j] += abs[i][j];
		}
	}
}

void CheckDiffCondition(ANSI87B* element) {
	float iop[3] = { 0 };
	float irst[3] = { 0 };

	GetOperatingCurrent(element, iop);
	GetRestraintCurrent(element, irst);

	float b = element->x1 * (element->m1 - element->m2);

	for (int i = 0; i < 3; i++) {
		if((iop[i] > (element->m1 * irst[i]) && iop[i] > element->ipk && iop[i] > (element->m2 * irst[i] + b)) || (iop[i] >= element->maxpu)) {
			element->trip = 1;
		}
	}
}

void ANSI87B_Reset(ANSI87B* element) {
	if (!element->trip) return;
	element->trip = 0;
}