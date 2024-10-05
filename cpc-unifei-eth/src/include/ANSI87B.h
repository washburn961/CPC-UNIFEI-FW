#ifndef ANSI_87B_H
#define ANSI_87B_H

#define _USE_MATH_DEFINES
#define terminals 8

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

//Defines a structure with the parameters used for differential protection.
typedef struct {
	float ipk; //Pick-up current.
	float maxpu; //Unrestrained operating current.
	float x1; //Start of slope 2 region.
	float m1; //Slope 1.
	float m2; //Slope 2.
	float current[terminals][6]; //Array containing the complex input current for each terminal connected to the busbar.
	unsigned int trip; //Trip signaling;
	float ctr[terminals]; //CT Ratios;
} ANSI87B;

/*
@brief Initializes the differential protection element, providing its parameters.
@param ipk Pick-up current.
@param maxpu Unrestrained operating current.
@param x1 Start of slope 2 region.
@param m1 Slope 1.
@param m2 Slope 2.
@param ctr CT Ratios
*/
void ANSI87B_Init(ANSI87B* element, float ipk, float maxpu, float x1, float m1, float m2, float ctr[terminals]);

//Checks the differential condition od the system.
void CheckDiffCondition(ANSI87B* element);

//Resets the protection element after trip signal is triggered.
void ANSI87B_Reset(ANSI87B* element);

#endif