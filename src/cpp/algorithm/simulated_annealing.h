#ifndef INTRANSITIVE_ALGORITHM_SIMULATED_ANNEALING_H
#define INTRANSITIVE_ALGORITHM_SIMULATED_ANNEALING_H

#include "../config/parameters.h"
#include "../model/dice.h"

namespace intransitive {

// Busca reinicios en paralelo y devuelve el mejor ciclo de forma determinista.
DicePairs find_best_cycle(const Parameters& parameters);

}

#endif
