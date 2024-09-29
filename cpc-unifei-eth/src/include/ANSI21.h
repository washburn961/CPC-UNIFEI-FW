#ifndef ANSI_21_H
#define ANSI_21_H

/**
 * @file ANSI21.h
 * @brief ANSI 21 Distance Protection Element Header
 *
 * This file provides the structure definition and function declarations for
 implementing an ANSI 21 with mho and quadrilateral characteristics.
 *
 *
*/

#include <stdio.h>
#include <math.h>
#include <cpc_complex.h>

/**
 * @brief Structure representing line and system parameters.
 */
typedef struct {
    complex_t Zlt1; /**< Positive sequence line impedance */
    complex_t Zlt0; /**< Zero sequence line impedance */
    float iNom; /**< Nominal current (placa) */
    float RTP; /**< Potential transformer ratio */
    float RTC; /**< Current transformer ratio */
    complex_t K0; /**< Ground compensation factor */
} ANSI21_Config;

/**
 * @brief Structure representing phase and sequence currents.
 */
typedef struct {
    complex_t current_ph[5]; /**< Phase currents measurements */
    complex_t current_seq[4]; /**< Sequence currents */
} Current;

/**
 * @brief Structure representing phase and sequence voltages.
 */
typedef struct {
    complex_t voltage_ph[5]; /**< Phase voltages measurements */
    complex_t voltage_seq[4]; /**< Sequence voltages */
} Voltage;

/**
 * @brief Structure representing an ANSI 21 element - mho phase characteristic.
 */
typedef struct {
    float Zr_1; /**< Zone 1 impedance range */
    float Zr_2; /**< Zone 2 impedance range */
    float Tr_2; /**< Zone 2 trip delay */
    float m[4]; /**< Calculated mho reach */
    float time_step; /**< Simulation time step */
    float time; /**< Accumulates time*/
    unsigned int pk_zone1; /**< Pickup zone 1 status */
    unsigned int pk_zone2; /**< Pickup zone 2 status */
    unsigned int is_tripped; /**< Trip status */
} ANSI21_Mp;

/**
 * @brief Structure representing an ANSI 21 element - mho ground characteristic.
 */
typedef struct {
    float Zr_1; /**< Zone 1 impedance range */
    float Zr_2; /**< Zone 2 impedance range */
    float Tr_2; /**< Zone 2 trip delay */
    float m[4]; /**< Calculated mho reach */
    float time_step; /**< Simulation time step */
    float time; /**< Accumulates time*/
    unsigned int pk_zone1; /**< Pickup zone 1 status */
    unsigned int pk_zone2; /**< Pickup zone 2 status */
    unsigned int is_tripped; /**< Trip status */
} ANSI21_Mg;

/**
 * @brief Structure representing an ANSI 21 element - quadrilateral ground characteristic.
 */
typedef struct {
    float Zr_1; /**< Zone 1 impedance range */
    float Zr_2; /**< Zone 2 impedance range */
    float Br_1; /**< Zone 1 resistance range */
    float Br_2; /**< Zone 2 resistance range */
    float Tr_2; /**< Zone 2 trip delay */
    float m[4]; /**< Calculated mho reach */
    float resis_ft[4]; /**< Estimated ground loop resistance */
    float resis_ff[4]; /**< Estimated phase loop resistance */
    float time_step; /**< Simulation time step */
    float time; /**< Accumulates time*/
    unsigned int pk_zone1; /**< Pickup zone 1 status */
    unsigned int pk_zone2; /**< Pickup zone 2 status */
    unsigned int is_tripped; /**< Trip status */
} ANSI21_Qg;


/**
 * @brief Initializes line and system parameters.
 *
 * @param element Pointer to the ANSI21_Config structure to be initialized.
 * @param Zlt1 Positive sequence line impedance
 * @param Zlt0 Zero sequence line impedance
 * @param iNom Nominal current (placa)
 * @param RTP Potential transformer ratio
 * @param RTC Current transformer ratio
 * @param K0 Ground compensation factor

 */
void ANSI21_Init_param(ANSI21_Config* element, complex_t Zlt1, complex_t Zlt0, float iNom, float RTP, float RTC, complex_t K0);

/**
 * @brief Initializes system currents.
 *
 * @param element Pointer to the Current structure to be initialized.
 * @param current_phA Phase A current measurement
 * @param current_phB Phase B current measurement
 * @param current_phC Phase C current measurement

 */
void Set_current(Current* element, complex_t current_phA, complex_t current_phB, complex_t current_phC);

/**
 * @brief Initializes system voltages.
 *
 * @param element Pointer to the Voltage structure to be initialized.
 * @param voltage_phA Phase A voltage measurement
 * @param voltage_phB Phase B voltage measurement
 * @param voltage_phC Phase C voltage measurement

 */
void Set_voltage(Voltage* element, complex_t voltage_phA, complex_t voltage_phB, complex_t voltage_phC);

/**
 * @brief Initializes an ANSI 21 distance protection element - mho phase characteristic.
 *
 * @param element Pointer to the ANSI21_Mp structure to be initialized.
 * @param Zr_Mp_1 Zone 1 impedance range
 * @param Zr_Mp_2 Zone 2 impedance range
 * @param Tr_Mp_2  Zone 2 trip delay
 * @param time_step Simulation time step
 */
void ANSI21_Init_Mp(ANSI21_Mp* element, float Zr_Mp_1, float Zr_Mp_2, float Tr_Mp_2, float time_step);

/**
 * @brief Initializes an ANSI 21 distance protection element - mho ground characteristic.
 *
 * @param element Pointer to the ANSI21_Mg structure to be initialized.
 * @param Zr_Mg_1 Zone 1 impedance range
 * @param Zr_Mg_2 Zone 2 impedance range
 * @param Tr_Mg_2  Zone 2 trip delay
 * @param time_step Simulation time step
 */
void ANSI21_Init_Mg(ANSI21_Mg* element, float Zr_Mg_1, float Zr_Mg_2, float Tr_Mg_2, float time_step);

/**
 * @brief Initializes an ANSI 21 distance protection element - quadrilateral ground characteristic.
 *
 * @param element Pointer to the ANSI21_Qg structure to be initialized.
 * @param Zr_Qg_1 Zone 1 impedance range
 * @param Zr_Qg_2 Zone 2 impedance range
 * @param Br_Qg_1 Zone 1 resistance range
 * @param Br_Qg_2 Zone 2 resistance range
 * @param Tr_Qg_2  Zone 2 trip delay
 * @param time_step Simulation time step
 */
void ANSI21_Init_Qg(ANSI21_Qg* element, float Zr_Qg_1, float Zr_Qg_2, float Br_Qg_1, float Br_Qg_2, float Tr_Qg_2, float time_step);



/**
 * @brief Advances the simulation step for the ANSI 21 distance protection element - mho phase characteristic.
 *
 * @param element Pointer to the ANSI21_Mp structure.
 * @param element Pointer to the Current structure.
 * @param element Pointer to the Voltage structure.
 * @param element Pointer to the ANSI21_Config structure.
 */
void ANSI21_Step_Mp(ANSI21_Mp* element, Current* element_i, Voltage* element_v, ANSI21_Config* general);

/**
 * @brief Advances the simulation step for the ANSI 21 distance protection element - mho ground characteristic.
 *
 * @param element Pointer to the ANSI21_Mg structure.
 * @param element Pointer to the Current structure.
 * @param element Pointer to the Voltage structure.
 * @param element Pointer to the ANSI21_Config structure.
 */
void ANSI21_Step_Mg(ANSI21_Mg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general);

/**
 * @brief Advances the simulation step for the ANSI 21 distance protection element - quadrilateral ground characteristic.
 *
 * @param element Pointer to the ANSI21_Qg structure.
 * @param element Pointer to the Current structure.
 * @param element Pointer to the Voltage structure.
 * @param element Pointer to the ANSI21_Config structure.
 */
void ANSI21_Step_Qg(ANSI21_Qg* element, Current* element_i, Voltage* element_v, ANSI21_Config* general);


/**
 * @brief Resets the trip condition of the ANSI 21 distance protection element - mho phase characteristic.
 *
 * @param element Pointer to the ANSI21_Mp structure.
 */
void ANSI21_Reset_Mp(ANSI21_Mp* element);

/**
 * @brief Resets the trip condition of the ANSI 21 distance protection element - mho ground characteristic.
 *
 * @param element Pointer to the ANSI21_Mg structure.
 */
void ANSI21_Reset_Mg(ANSI21_Mg* element);

/**
 * @brief Resets the trip condition of the ANSI 21 distance protection element - quadrilateral ground characteristic.
 *
 * @param element Pointer to the ANSI21_Qg structure.
 */
void ANSI21_Reset_Qg(ANSI21_Qg* element);


#endif // ANSI_21
