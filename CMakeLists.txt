cmake_minimum_required(VERSION 3.14)

project(PricingLib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# download catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Core library
add_library(pricing_core src/pricing.cpp)

# PUBLIC means that any executable that links to 'pricing_core' 
# will automatically know where to find the headers in the 'include' folder.
target_include_directories(pricing_core PUBLIC include)

# Executables and tests

# Main app
add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE pricing_core)

# 3. Build the tests and link both Catch2 and the core library
enable_testing()
add_executable(unit_tests tests/test_main.cpp)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain pricing_core)

include(Catch)
catch_discover_tests(unit_tests)