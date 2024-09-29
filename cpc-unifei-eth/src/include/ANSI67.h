#ifndef ANSI_67_H
#define ANSI_67_H

/**
 * @file ANSI67.h
 * @brief ANSI 67 Directional Overcurrent Protection Element Header
 *
 * This file provides the structure definition and function declarations for
 * implementing a directional overcurrent protection element (ANSI 67).
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <cpc_complex.h>

 /**
  * @brief Structure representing the system and line parameters for ANSI 67.
  */
typedef struct {
    complex_t Zlt1; /*< Positive sequence impedance */
    complex_t Zlt0; /*< Zero sequence impedance */
    float RTP; /**< Potential transformer ratio */
    float RTC; /**< Current transformer ratio */
    float iNom; /**< Nominal current (placa) */
    float i_pickup; /**< Pickup current */
    bool dir_ph; /**< if true, activates the phase directional element */
    bool dir_seq; /**< if true, activates the sequence directional element */
    bool dir_imp; /**< if true, activates the impedance directional element */
} ANSI67_Config;

/**
 * @brief Structure representing phase and sequence currents.
 */
typedef struct {
    complex_t current_ph[5]; /**< Phase currents measurements */
    complex_t current_seq[4]; /**< Sequence currents */
} Currents;

/**
 * @brief Structure representing phase and sequence voltages.
 */
typedef struct {
    complex_t voltage_ph[6]; /**< Phase voltages measurements */
    complex_t voltage_seq[4]; /**< Sequence voltages */
} Voltages;

/**
 * @brief Structure representing the results of the ANSI 67 directional element.
 */
typedef struct {
    float T_ph[4]; /* Phase torque */
    float T_0; /* Zero sequence torque */
    float T_1; /* Positive sequence torque */
    float T_2; /* Negative sequence torque */
    float Z2; /* Negative sequence impedance */
    float Z0; /* Zero sequence impedance */
    float Z2_f; /* Forward negative sequence impedance */
    float Z2_r; /* Reverse negative sequence impedance */
    float Z0_f; /* Forward zero sequence impedance */
    float Z0_r; /* Reverse zero sequence impedance */

    unsigned int pk_dir_ph[4]; /*< Phase directional detection flag */
    unsigned int pk_dir_seq; /*< Sequence directional detection flag */
    unsigned int pk_dir_imp; /*< Impedance directional detection flag */
    unsigned int pk_dir_Z2; /*< Detection flag for directional Z2 (negative sequence impedance) */
    unsigned int pk_dir_Z0; /*< Detection flag for directional Z0 (zero sequence impedance) */
    unsigned int is_pickup; /*< General detection flag for directional element*/
} ANSI67;


/**
 * @brief Initializes the system and line parameters for ANSI 67.
 *
 * @param element Pointer to the ANSI67_Config structure to be initialized.
 * @param Z1 Positive sequence impedance.
 * @param Z0 Zero sequence impedance.
 * @param iNom Nominal current.
 * @param dir_ph Activation of the phase directional element.
 * @param dir_seq Activation of the sequence directional element.
 * @param dir_imp Activation of the impedance directional element.
 */
void ANSI67_Init(ANSI67_Config* general, complex_t Zlt1, complex_t Zlt0, float RTC, float RTP, float iNom, float i_pickup, bool dir_ph, bool dir_seq, bool dir_imp);


/**
 * @brief Initializes system currents.
 *
 * @param element Pointer to the Current structure to be initialized.
 * @param current_phA Phase A current measurement.
 * @param current_phB Phase B current measurement.
 * @param current_phC Phase C current measurement.
 */
void Set_currents(Currents* element, complex_t current_phA, complex_t current_phB, complex_t current_phC);

/**
 * @brief Initializes system voltages.
 *
 * @param element Pointer to the Voltage structure to be initialized.
 * @param voltage_phA Phase A voltage measurement.
 * @param voltage_phB Phase B voltage measurement.
 * @param voltage_phC Phase C voltage measurement.
 */
void Set_voltages(Voltages* element, complex_t voltage_phA, complex_t voltage_phB, complex_t voltage_phC);


/**
 * @brief Advances the simulation step for the ANSI 67 directional element.
 *
 * @param element Pointer to the ANSI67 structure.
 * @param general Pointer to the ANSI67_Config structure.
 * @param element_i Pointer to the Current structure.
 * @param element_v Pointer to the Voltage structure.
 */
void ANSI67_Step(ANSI67* element, ANSI67_Config* general, Currents* element_i, Voltages* element_v);

/**
 * @brief Resets the pickup condition of the ANSI 67 directional protection element.
 *
 * @param element Pointer to the ANSI67 structure.
 */
void ANSI67_Reset(ANSI67* element);

#endif // ANSI_67_H