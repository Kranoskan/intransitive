#ifndef INTRANSITIVE_PLATFORM_PATHS_H
#define INTRANSITIVE_PLATFORM_PATHS_H

#include <cstdio>
#include <string>

namespace intransitive {

// Convierte un argumento recibido del sistema a UTF-8 para procesar rutas.
std::string argument_to_utf8(const char* argument);

// Obtiene la carpeta de trabajo actual codificada en UTF-8.
std::string current_directory();

// Obtiene la carpeta que contiene el ejecutable.
std::string executable_directory(const char* executable_argument);

// Resuelve una ruta relativa contra la carpeta base indicada.
std::string absolute_path(
    const std::string& path,
    const std::string& base_directory
);

// Devuelve la carpeta padre de una ruta.
std::string parent_path(const std::string& path);

// Abre una ruta UTF-8 con el modo de archivo especificado.
FILE* open_path(const std::string& path, const char* mode);

// Crea recursivamente una carpeta y devuelve el motivo de error si falla.
bool create_directories(const std::string& path, std::string& error_message);

}

#endif
