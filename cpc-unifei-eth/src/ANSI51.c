#include "ANSI51.h"
#include "cpc_complex.h"
#include <math.h>
#include <stdio.h>

float sum_trip;

// Fun��o para definir as correntes de fase no rel� ANSI 51
 void SetCurrent(ANSI51* element, complex_t iphA, complex_t iphB, complex_t iphC) {
    element->current[0] = iphA;
    element->current[1] = iphB;
    element->current[2] = iphC;
}

// Fun��o para inicializar o rel� ANSI 51
void ANSI51_Init(ANSI51* element, float td, float pick_up_current, float time_step, ANSI51_Standard standard, int curve) {
    element->td = td;
    element->pick_up_current = pick_up_current;
    element->time_step = time_step;
    element->standard = standard;
    element->curve = curve;
    element->is_tripped[PHASE_COUNT] = 0;
    element->time[PHASE_COUNT] = 0;
}

// Fun��o para verificar a condi��o de sobrecorrente em uma fase espec�fica
static int CheckOvercurrentCondition(ANSI51* element, int phase) {
    element->is_pickupped[phase] = complex_get_magnitude(element->current[phase]) > element->pick_up_current;

    return element->is_pickupped[phase];
}

// Fun��o para calcular o tempo necess�rio para que o rel� dispare, com base na curva selecionada
static float CalculateTimeToTrip(ANSI51* element, int phase) {
    float magnitude = complex_get_magnitude(element->current[phase]);
    float pick_up_multiple = magnitude / element->pick_up_current;

    if (element->standard == STANDARD_1) {
        switch (element->curve) {
        case 1:
            element->time_to_trip[phase] = (element->td / 7.0f) * ((0.0515f / (powf(pick_up_multiple, 0.02f) - 1.0f)) + 0.114f);
            break;
        case 2:
            element->time_to_trip[phase] = (element->td / 7.0f) * ((19.61f / (powf(pick_up_multiple, 2.0f) - 1.0f)) + 0.491f);
            break;
        case 3:
            element->time_to_trip[phase] = (element->td / 7.0f) * ((28.2f / (powf(pick_up_multiple, 2.0f) - 1.0f)) + 0.1217f);
            break;
        case 4:
            element->time_to_trip[phase] = (element->td / 7.0f) * ((5.95f / (powf(pick_up_multiple, 2.0f) - 1.0f)) + 0.18f);
            break;
        case 5:
            element->time_to_trip[phase] = (element->td / 7.0f) * ((0.02394f / (powf(pick_up_multiple, 0.02f) - 1.0f)) + 0.01694f);
            break;
        }
    }
    else if (element->standard == STANDARD_2) {
        switch (element->curve) {
        case 1:
            element->time_to_trip[phase] = (element->td * (0.14f / (powf(pick_up_multiple, 0.02f) - 1.0f)));
            break;
        case 2:
            element->time_to_trip[phase] = (element->td * (13.5f / (pick_up_multiple - 1.0f)));
            break;
        case 3:
            element->time_to_trip[phase] = (element->td * (80.0f / (powf(pick_up_multiple, 2.0f) - 1.0f)));
            break;
        case 4:
            element->time_to_trip[phase] = (element->td * (120.0f / (pick_up_multiple - 1.0f)));
            break;
        case 5:
            element->time_to_trip[phase] = (element->td * (0.05f / (powf(pick_up_multiple, 0.04f) - 1.0f)));
            break;
        }
    }
    return element->time_to_trip[phase];
}

// Fun��o para avan�ar a simula��o do rel� ANSI 51 e calcular o tempo de opera��o por fase
void ANSI51_Step(ANSI51* element) {
    for (int phase = 0; phase < PHASE_COUNT; phase++) {
        if (!CheckOvercurrentCondition(element, phase)) {
            if (element->time[phase] > 0) {
                element->time[phase] = 0.0f; // Reseta o tempo acumulado se a corrente voltar ao normal
            }
            continue;
        }

        element->is_tripped[phase] = element->time[phase] >= CalculateTimeToTrip(element, phase);

        if (!element->is_tripped[phase]) {
            element->time[phase] += element->time_step;
        }         
    }
}

// Fun��o para resetar o estado de disparo do rel� ANSI 51
void ANSI51_Reset(ANSI51* element) {
    for (int phase = 0; phase < PHASE_COUNT; phase++) {
        if (element->is_tripped[phase]) {
            element->is_tripped[phase] = 0;
            element->time[phase] = 0.0f;
        }
    }
}
