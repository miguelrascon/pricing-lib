cmake_minimum_required(VERSION 3.14)

project(PricingLib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Desactiva extensiones del compilador (p. ej. gnu++17) para escribir C++ estándar y portable.
set(CMAKE_CXX_EXTENSIONS OFF)

# Genera compile_commands.json, que usan clangd/IntelliSense para entender el proyecto.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Dependencias: Catch2 (framework de tests), descargado en la configuración.
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# ---------------------------------------------------------------------------
# Warnings del compilador: se aplican SOLO a nuestros targets (no a Catch2).
# Las expresiones generadoras $<...> eligen las flags según el compilador:
#   - MSVC:      /W4 (nivel alto de avisos) y /permissive- (conformidad estricta)
#   - GCC/Clang: -Wall -Wextra -Wpedantic, más -Wshadow y -Wconversion, que
#                detectan variables ocultadas y conversiones implícitas con pérdida.
# ---------------------------------------------------------------------------
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion>
)

# Opción para tratar los warnings como errores (se activa en CI).
option(PRICING_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
if(PRICING_WARNINGS_AS_ERRORS)
    target_compile_options(project_warnings INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
    )
endif()

# ---------------------------------------------------------------------------
# Librería principal
# ---------------------------------------------------------------------------
add_library(pricing_core
    src/math.cpp
    src/pricing.cpp
)

# PUBLIC: cualquier target que enlace con pricing_core encuentra
# automáticamente las cabeceras de la carpeta include/.
target_include_directories(pricing_core PUBLIC include)

# PRIVATE: los warnings se usan al compilar pricing_core, pero no se
# propagan a quien la consuma.
target_link_libraries(pricing_core PRIVATE project_warnings)

# ---------------------------------------------------------------------------
# Ejecutable de ejemplo
# ---------------------------------------------------------------------------
add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE pricing_core project_warnings)

# ---------------------------------------------------------------------------
# Tests (Catch2 aporta la función main mediante Catch2WithMain)
# ---------------------------------------------------------------------------
enable_testing()
add_executable(unit_tests
    tests/test_math.cpp
    tests/test_black_scholes.cpp
)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain pricing_core project_warnings)

# Registra cada TEST_CASE como un test independiente de CTest.
include(Catch)
catch_discover_tests(unit_tests)
