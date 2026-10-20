#ifndef INTRANSITIVE_PLATFORM_PARALLEL_H
#define INTRANSITIVE_PLATFORM_PARALLEL_H

#include <cstddef>

namespace intransitive {

typedef void (*ParallelTask)(std::size_t index, void* context);

// Ejecuta cada índice una vez usando los núcleos disponibles en Windows.
// En otras plataformas, ejecuta el trabajo secuencialmente.
void parallel_for(
    std::size_t item_count,
    ParallelTask task,
    void* context
);

}

#endif
