#ifndef ANSI_51_H
#define ANSI_51_H

#include "cpc_complex.h"  // Inclui a sua biblioteca de números complexos

// Definição da enumeração ANSI51_Standard
typedef enum {
    STANDARD_1,
    STANDARD_2
} ANSI51_Standard;

/**
 * @brief Structure representing an ANSI 51 overcurrent protection element.
 */
typedef struct {
    float td; /**< Time dial setting */
    complex_t current[4]; /**< Current measurements for phases A, B, and C */
    float pick_up_current; /**< Pickup current setting */
    float time_step; /**< Simulation time step */
    float time[4]; /**< Accumulated time for each phase */
    float time_to_trip[4]; /**< Time to trip for each phase */
    unsigned int is_pickupped[4]; /**< Pick up status flags for each phase */
    unsigned int is_tripped[4]; /**< Trip status flags for each phase */
    ANSI51_Standard standard; /**< Norm selection: STANDARD_1 or STANDARD_2 */
    int curve; /**< Curve selection: 1 to 5 */
} ANSI51;

/**
 * @brief Initializes an ANSI 51 overcurrent protection element.
 *
 * @param element Pointer to the ANSI51 structure to be initialized.
 * @param td Time dial setting.
 * @param pick_up_current Pickup current setting.
 * @param time_step Simulation time step.
 * @param standard Norm selection: STANDARD_1 or STANDARD_2.
 * @param curve Curve selection: 1 to 5.
 */
void ANSI51_Init(ANSI51* element, float td, float pick_up_current, float time_step, ANSI51_Standard standard, int curve);

/**
 * @brief Advances the simulation step for the ANSI 51 overcurrent protection element.
 *
 * @param element Pointer to the ANSI51 structure.
 */
void ANSI51_Step(ANSI51* element);

/**
 * @brief Resets the trip condition of the ANSI 51 overcurrent protection element.
 *
 * @param element Pointer to the ANSI51 structure.
 */
void ANSI51_Reset(ANSI51* element);

/**
 * @brief Sets the current values for the ANSI 51 element (new declaration).
 *
 * @param element Pointer to the ANSI51 structure.
 * @param iphA Current for phase A.
 * @param iphB Current for phase B.
 * @param iphC Current for phase C.
 */
void SetCurrent(ANSI51* element, complex_t iphA, complex_t iphB, complex_t iphC);

#endif // ANSI_51_H
