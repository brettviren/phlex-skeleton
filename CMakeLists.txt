cmake_minimum_required(VERSION 3.25)
project(phlex_skeleton VERSION 0.1.0 LANGUAGES CXX)

# Phlex requires C++23.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# phlex's exported CMake config references these imported targets but does not
# find them for us, so we must find_package() them BEFORE find_package(phlex)
# or phlexTargets.cmake errors with "target not found".  (This workaround may
# become unnecessary in a future Phlex release.)
find_package(Boost REQUIRED COMPONENTS json)
find_package(TBB REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(phlex REQUIRED)

# Each Phlex plugin is a dlopen-only MODULE library named lib<name>.so.  The
# <name> is exactly what you write as `cpp: '<name>'` in the Jsonnet config.

# A source (provider) node links phlex::source.
add_library(counter MODULE src/counter.cpp)
target_link_libraries(counter PRIVATE phlex::source)

# Module nodes (transform, observe) link phlex::module.
add_library(squarer MODULE src/squarer.cpp)
target_link_libraries(squarer PRIVATE phlex::module)

add_library(chirp MODULE src/chirp.cpp)
target_link_libraries(chirp PRIVATE phlex::module)
