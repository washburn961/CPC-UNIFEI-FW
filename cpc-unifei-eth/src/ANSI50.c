#include "ansi50.h"
#include <cpc_complex.h>

int sum_trip;

complex_t SetCurrent(ANSI50* element, complex_t iphA, complex_t iphB, complex_t iphC) {
	element->current[0]= iphA;
	element->current[1]= iphB;
	element->current[2]= iphC;
}


void ANSI50_Init(ANSI50* element, float pick_up_current) {
	element->pick_up_current = pick_up_current; 
	element->is_tripped[PHASE_COUNT] = 0; 
}

void ANSI50_Step(ANSI50* element) {
	for (int i = 0; i < PHASE_COUNT; i++) {
		if (complex_get_magnitude(element->current[i]) > element->pick_up_current) {
			element->is_tripped[i] = 1;
		}
		else {
			element->is_tripped[i] = 0;

		}
		/*sum_trip = sum_trip + element->is_tripped[i];*/
	}
	//if (sum_trip == 0) return 0;
	//return element->is_tripped;
}

void ANSI50_Reset(ANSI50* element) {
	for (int i = 0; i < PHASE_COUNT; i++) {
		element->is_tripped[i] = 0;
	}
}