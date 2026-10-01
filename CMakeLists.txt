cmake_minimum_required(VERSION 3.25)
project(phlex_skeleton VERSION 0.1.0 LANGUAGES CXX)

# Phlex requires C++23.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Phlex.  Since Phlex 0.4.1 its CMake config finds its own dependencies
# (Boost, TBB, fmt, spdlog, Microsoft.GSL), so this one line is all we need.
find_package(phlex 0.4.1 REQUIRED)

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

# Install the plugins next to each other so one PHLEX_PLUGIN_PATH entry finds
# them all, plus the example configuration.
include(GNUInstallDirs)
install(TARGETS counter squarer chirp LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES config/skeleton.jsonnet
        DESTINATION "${CMAKE_INSTALL_DATADIR}/phlex_skeleton/config")

# A test that runs the example exactly as the README does: `phlex -c
# config/skeleton.jsonnet` with PHLEX_PLUGIN_PATH holding our plugins and the
# directory with Phlex's own plugins (generate_layers).  Run it with `ctest`.
include(CTest)
if(BUILD_TESTING)
  find_program(PHLEX_EXECUTABLE phlex HINTS "${phlex_BIN_DIR}" REQUIRED)
  add_test(NAME skeleton_run
           COMMAND "${PHLEX_EXECUTABLE}" -c "${CMAKE_CURRENT_SOURCE_DIR}/config/skeleton.jsonnet")
  set_tests_properties(skeleton_run PROPERTIES
    ENVIRONMENT "PHLEX_PLUGIN_PATH=${CMAKE_CURRENT_BINARY_DIR}:${phlex_LIBRARY_DIR}"
    # 5 events, numbered 1..5, so the last square is 25 (lines may come in any order).
    PASS_REGULAR_EXPRESSION "chirp: 25"
    FAIL_REGULAR_EXPRESSION "[Ee]rror|[Ee]xception")
endif()
