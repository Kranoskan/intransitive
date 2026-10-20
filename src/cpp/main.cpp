#include "application.h"

// Punto de entrada: delega el flujo completo a la capa de aplicación.
int main(int argc, char* argv[]) {
    return intransitive::run_application(argc, argv);
}
