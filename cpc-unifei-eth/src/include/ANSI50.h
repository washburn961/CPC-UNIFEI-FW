#ifndef ANSI_50_H
#define ANSI_50_H

#define PHASE_COUNT 3

/**
 * @file ANSI50.h
 * @brief ANSI 50 Instantaneous Overcurrent Protection Element Header
 *
 * This file provides the structure definition and function declarations for implementing an ANSI 50 instantaneous overcurrent protection element.
 *
 * ### Usage
 *
 * 1. **Initialization**:
 *    - Initialize the ANSI 50 element using the `ANSI50_Init` function with the `pick_up_current` setting as defined by the user.
 *
 * 2. **Execution**:
 *    - Use a task in a scheduler such as FreeRTOS or a timer interrupt to update the `current` field in the struct.
 *    - Execute the `ANSI50_Step` function periodically.
 *
 * 3. **Post-Execution**:
 *    - After execution, check if the relay has tripped by inspecting the `is_tripped` field.
 *    - If necessary, use the `ANSI50_Reset` function to reset the trip condition.
 *
 * Example:
 * @code
 * ANSI50 overcurrentElement;
 * ANSI50_Init(&overcurrentElement, 5.0);
 * while (1) {
 *     overcurrentElement.current = get_current_reading(); // Update current
 *     ANSI50_Step(&overcurrentElement);
 *     if (overcurrentElement.is_tripped) {
 *         handle_trip();
 *         ANSI50_Reset(&overcurrentElement);
 *     }
 *     vTaskDelay(pdMS_TO_TICKS(10)); // Delay for FreeRTOS task
 * }
 * @endcode
 */

#include <cpc_complex.h>

/**
  * @brief Structure representing an ANSI 50 instantaneous overcurrent protection element.
  */
typedef struct {
    complex_t current[4]; /**< Phase current measurement */
    float pick_up_current; /**< Pickup current setting */
    unsigned int is_tripped[4]; /**< Trip status flag */
} ANSI50;

/**
 * @brief Initializes an ANSI 50 instantaneous overcurrent protection element.
 *
 * @param element Pointer to the ANSI50 structure to be initialized.
 * @param pick_up_current Pickup current setting.
 */
void ANSI50_Init(ANSI50* element, float pick_up_current);

/**
 * @brief Advances the simulation step for the ANSI 50 instantaneous overcurrent protection element.
 *
 * @param element Pointer to the ANSI50 structure.
 */
void ANSI50_Step(ANSI50* element);

/**
 * @brief Resets the trip condition of the ANSI 50 instantaneous overcurrent protection element.
 *
 * @param element Pointer to the ANSI50 structure.
 */
void ANSI50_Reset(ANSI50* element);

#endif // ANSI_50_H