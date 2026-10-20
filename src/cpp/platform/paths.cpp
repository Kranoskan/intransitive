#include "paths.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace intransitive {
namespace {

#ifdef _WIN32
// Convierte una ruta UTF-8 a UTF-16 para las API de Windows.
std::wstring utf8_to_wide(const std::string& value) {
    if (value.empty()) {
        return L"";
    }
    const int length = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(), -1, NULL, 0
    );
    if (length == 0) {
        throw std::runtime_error("No se pudo convertir una ruta UTF-8.");
    }
    std::vector<wchar_t> buffer(static_cast<std::size_t>(length));
    if (MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(), -1,
            buffer.data(), length
        ) == 0) {
        throw std::runtime_error("No se pudo convertir una ruta UTF-8.");
    }
    return std::wstring(buffer.data());
}

// Convierte una ruta UTF-16 al formato UTF-8 de los mensajes de error.
std::string wide_to_utf8(const std::wstring& value) {
    if (value.empty()) {
        return "";
    }
    const int length = WideCharToMultiByte(
        CP_UTF8, 0, value.c_str(), -1, NULL, 0, NULL, NULL
    );
    if (length == 0) {
        throw std::runtime_error("No se pudo convertir una ruta a UTF-8.");
    }
    std::vector<char> buffer(static_cast<std::size_t>(length));
    if (WideCharToMultiByte(
            CP_UTF8, 0, value.c_str(), -1,
            buffer.data(), length, NULL, NULL
        ) == 0) {
        throw std::runtime_error("No se pudo convertir una ruta a UTF-8.");
    }
    return std::string(buffer.data());
}

// Adapta una ruta a la codificación local que acepta fopen en este MinGW.
std::string wide_to_system_encoding(const std::wstring& value) {
    const int length = WideCharToMultiByte(
        CP_ACP, 0, value.c_str(), -1, NULL, 0, NULL, NULL
    );
    if (length == 0) {
        throw std::runtime_error(
            "La ruta no se puede representar en la codificación del sistema."
        );
    }
    std::vector<char> buffer(static_cast<std::size_t>(length));
    if (WideCharToMultiByte(
            CP_ACP, 0, value.c_str(), -1,
            buffer.data(), length, NULL, NULL
        ) == 0) {
        throw std::runtime_error(
            "La ruta no se puede representar en la codificación del sistema."
        );
    }
    return std::string(buffer.data());
}

// Obtiene la carpeta padre teniendo en cuenta separadores de Windows.
std::string parent_from_utf8_path(const std::string& path) {
    const std::size_t separator = path.find_last_of("/\\");
    if (separator == std::string::npos) {
        return "";
    }
    if (separator == 2 && path.size() >= 3 && path[1] == ':') {
        return path.substr(0, 3);
    }
    if (separator == 0) {
        return path.substr(0, 1);
    }
    return path.substr(0, separator);
}
#endif

// Reconoce rutas absolutas con las convenciones de la plataforma actual.
bool is_absolute(const std::string& path) {
    if (path.empty()) {
        return false;
    }
#ifdef _WIN32
    return path[0] == '/' || path[0] == '\\' ||
           (path.size() >= 3 &&
            ((path[0] >= 'A' && path[0] <= 'Z') ||
             (path[0] >= 'a' && path[0] <= 'z')) &&
            path[1] == ':' &&
            (path[2] == '/' || path[2] == '\\'));
#else
    return path[0] == '/';
#endif
}

// Combina una carpeta con una ruta relativa usando el separador del sistema.
std::string join_paths(
    const std::string& directory,
    const std::string& child
) {
    if (directory.empty()) {
        return child;
    }
    const char last = directory[directory.size() - 1];
    if (last == '/' || last == '\\') {
        return directory + child;
    }
#ifdef _WIN32
    return directory + "\\" + child;
#else
    return directory + "/" + child;
#endif
}

// Crea una sola carpeta o acepta que ya exista como directorio.
bool create_one_directory(const std::string& path, std::string& error_message) {
#ifdef _WIN32
    const std::wstring wide_path = utf8_to_wide(path);
    if (CreateDirectoryW(wide_path.c_str(), NULL)) {
        return true;
    }
    const DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS) {
        const DWORD attributes = GetFileAttributesW(wide_path.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            return true;
        }
    }
    error_message = "código de error " + std::to_string(error);
    return false;
#else
    if (mkdir(path.c_str(), 0777) == 0) {
        return true;
    }
    if (errno == EEXIST) {
        struct stat status;
        if (stat(path.c_str(), &status) == 0 && S_ISDIR(status.st_mode)) {
            return true;
        }
    }
    error_message = std::strerror(errno);
    return false;
#endif
}

}

// Decodifica el argumento de la consola a UTF-8 cuando se ejecuta en Windows.
std::string argument_to_utf8(const char* argument) {
    if (argument == NULL) {
        return "";
    }
#ifdef _WIN32
    const int wide_length = MultiByteToWideChar(
        CP_ACP, 0, argument, -1, NULL, 0
    );
    if (wide_length == 0) {
        return argument;
    }
    std::vector<wchar_t> wide_buffer(
        static_cast<std::size_t>(wide_length)
    );
    if (MultiByteToWideChar(
            CP_ACP, 0, argument, -1, wide_buffer.data(), wide_length
        ) == 0) {
        return argument;
    }
    return wide_to_utf8(std::wstring(wide_buffer.data()));
#else
    return argument;
#endif
}

// Recupera la carpeta de trabajo actual en UTF-8.
std::string current_directory() {
#ifdef _WIN32
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetCurrentDirectoryW(
        static_cast<DWORD>(buffer.size()), buffer.data()
    );
    if (length == 0 || length >= buffer.size()) {
        throw std::runtime_error("No se pudo obtener la carpeta actual.");
    }
    return wide_to_utf8(std::wstring(buffer.data(), length));
#else
    std::vector<char> buffer(4096);
    if (getcwd(buffer.data(), buffer.size()) == NULL) {
        throw std::runtime_error("No se pudo obtener la carpeta actual.");
    }
    return std::string(buffer.data());
#endif
}

// Localiza la carpeta del ejecutable, independientemente de la carpeta actual.
std::string executable_directory(const char* executable_argument) {
#ifdef _WIN32
    (void)executable_argument;
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(
        NULL, buffer.data(), static_cast<DWORD>(buffer.size())
    );
    if (length == 0 || length >= buffer.size()) {
        throw std::runtime_error("No se pudo obtener la ruta del ejecutable.");
    }
    return parent_from_utf8_path(
        wide_to_utf8(std::wstring(buffer.data(), length))
    );
#else
    const std::string executable = argument_to_utf8(executable_argument);
    return parent_path(absolute_path(executable, current_directory()));
#endif
}

// Convierte una ruta relativa en absoluta respecto a la carpeta base.
std::string absolute_path(
    const std::string& path,
    const std::string& base_directory
) {
    if (is_absolute(path)) {
        return path;
    }
    return join_paths(base_directory, path);
}

// Extrae la carpeta que contiene una ruta.
std::string parent_path(const std::string& path) {
#ifdef _WIN32
    return parent_from_utf8_path(path);
#else
    const std::size_t separator = path.find_last_of('/');
    if (separator == std::string::npos) {
        return "";
    }
    if (separator == 0) {
        return "/";
    }
    return path.substr(0, separator);
#endif
}

// Abre una ruta UTF-8 traduciendo la codificación necesaria en Windows.
FILE* open_path(const std::string& path, const char* mode) {
#ifdef _WIN32
    const std::wstring wide_path = utf8_to_wide(path);
    const std::string system_path = wide_to_system_encoding(wide_path);
    return std::fopen(system_path.c_str(), mode);
#else
    return std::fopen(path.c_str(), mode);
#endif
}

// Crea en orden todos los directorios de la ruta solicitada.
bool create_directories(
    const std::string& path,
    std::string& error_message
) {
    if (path.empty()) {
        return true;
    }
    if (create_one_directory(path, error_message)) {
        return true;
    }

    const std::string parent = parent_path(path);
    if (parent.empty() || parent == path) {
        return false;
    }
#ifdef _WIN32
    if (parent.size() == 2 && parent[1] == ':') {
        return false;
    }
#endif
    if (!create_directories(parent, error_message)) {
        return false;
    }
    return create_one_directory(path, error_message);
}

}
