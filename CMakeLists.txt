cmake_minimum_required(VERSION 3.15)
project(cruckig VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(BUILD_TESTS "Build test programs" ON)
option(BUILD_BENCHMARK "Build benchmark program" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_WRAPPER_TESTS "Register Python/Rust README smoke tests with ctest" ON)

# ============================================================================
# Performance Optimization Level (0, 1, 2)
#
#   0 = Default   : -O2, portable, no special flags
#   1 = Optimized : -O3 + LTO (link-time optimization for cross-file inlining)
#   2 = Aggressive: -O3 + LTO + -march=native + -ffast-math
#
# Usage:
#   cmake .. -DCRUCKIG_OPT_LEVEL=2
# ============================================================================
set(CRUCKIG_OPT_LEVEL "0" CACHE STRING "Optimization level: 0=default, 1=LTO+O3, 2=aggressive")

# Library sources
set(CRUCKIG_SOURCES
    src/roots.c
    src/brake.c
    src/profile.c
    src/block.c
    src/position_first_step1.c
    src/position_first_step2.c
    src/position_second_step1.c
    src/position_second_step2.c
    src/position_third_step1.c
    src/position_third_step2.c
    src/velocity_second_step1.c
    src/velocity_second_step2.c
    src/velocity_third_step1.c
    src/velocity_third_step2.c
    src/trajectory.c
    src/calculator.c
    src/input_parameter.c
    src/output_parameter.c
    src/cruckig.c
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Static library
add_library(cruckig STATIC ${CRUCKIG_SOURCES})
target_include_directories(cruckig PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cruckig PRIVATE m)

# Shared library
add_library(cruckig_shared SHARED ${CRUCKIG_SOURCES})
set_target_properties(cruckig_shared PROPERTIES
    OUTPUT_NAME cruckig
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(cruckig_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cruckig_shared PRIVATE m)

# ============================================================================
# Compiler flags based on optimization level
# ============================================================================
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")

    # Base flags (all levels)
    set(CRUCKIG_BASE_FLAGS -Wall -Wextra -DNDEBUG)

    if(CRUCKIG_OPT_LEVEL STREQUAL "0")
        # Level 0: Default portable build
        set(CRUCKIG_OPT_FLAGS -O2)
        message(STATUS "cruckig optimization: Level 0 (default, -O2)")

    elseif(CRUCKIG_OPT_LEVEL STREQUAL "1")
        # Level 1: LTO + O3
        set(CRUCKIG_OPT_FLAGS -O3 -flto)
        message(STATUS "cruckig optimization: Level 1 (LTO + O3)")

    elseif(CRUCKIG_OPT_LEVEL STREQUAL "2")
        # Level 2: Aggressive - LTO + O3 + native + safe fast-math subset
        # Note: -ffast-math breaks isinf() which is used to detect infinite jerk/accel.
        # Instead we use individual safe flags that don't assume no-infinities.
        set(CRUCKIG_OPT_FLAGS -O3 -flto -march=native
            -fno-math-errno -funsafe-math-optimizations -fno-trapping-math
            -fassociative-math -freciprocal-math)
        message(STATUS "cruckig optimization: Level 2 (LTO + O3 + native + tuned-math)")

    else()
        message(FATAL_ERROR "Invalid CRUCKIG_OPT_LEVEL: ${CRUCKIG_OPT_LEVEL}. Must be 0, 1, or 2.")
    endif()

    # Apply to static library
    target_compile_options(cruckig PRIVATE ${CRUCKIG_BASE_FLAGS} ${CRUCKIG_OPT_FLAGS})

    # Apply to shared library (add -fPIC)
    target_compile_options(cruckig_shared PRIVATE ${CRUCKIG_BASE_FLAGS} ${CRUCKIG_OPT_FLAGS} -fPIC)

    # LTO requires linker flags too
    if(CRUCKIG_OPT_LEVEL GREATER 0)
        target_link_options(cruckig PRIVATE -flto)
        target_link_options(cruckig_shared PRIVATE -flto)
    endif()

endif()

# Pass optimization level as a compile definition
target_compile_definitions(cruckig PRIVATE CRUCKIG_OPT_LEVEL=${CRUCKIG_OPT_LEVEL})
target_compile_definitions(cruckig_shared PRIVATE CRUCKIG_OPT_LEVEL=${CRUCKIG_OPT_LEVEL})

# ============================================================================
# Tests
# ============================================================================
if(BUILD_TESTS)
    enable_testing()

    add_executable(cruckig_test test/test_main.c)
    target_link_libraries(cruckig_test PRIVATE cruckig m)

    add_executable(cruckig_test_random test/test_random.c)
    target_link_libraries(cruckig_test_random PRIVATE cruckig m)

    add_executable(cruckig_test_readme test/test_readme_example.c)
    target_link_libraries(cruckig_test_readme PRIVATE cruckig m)
    add_test(NAME readme_example COMMAND cruckig_test_readme)

    if(BUILD_WRAPPER_TESTS)
        find_program(Python3_EXECUTABLE NAMES python3)
        find_program(CARGO_EXECUTABLE NAMES cargo)

        if(Python3_EXECUTABLE)
            add_test(
                NAME readme_python
                COMMAND ${CMAKE_COMMAND} -E env
                    "CRUCKIG_LIB=$<TARGET_FILE:cruckig_shared>"
                    "PYTHONPATH=${CMAKE_SOURCE_DIR}/wrapper/python"
                    ${Python3_EXECUTABLE}
                    ${CMAKE_SOURCE_DIR}/wrapper/python/examples/test_readme_python.py
            )
            set_tests_properties(readme_python PROPERTIES LABELS "wrapper;python")
        else()
            message(STATUS "python3 not found — skipping readme_python ctest")
        endif()

        if(CARGO_EXECUTABLE)
            add_test(
                NAME readme_rust
                COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/wrapper/rust
                    ${CARGO_EXECUTABLE} test --test readme_example --quiet
            )
            set_tests_properties(readme_rust PROPERTIES LABELS "wrapper;rust")
        else()
            message(STATUS "cargo not found — skipping readme_rust ctest")
        endif()
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        if(CRUCKIG_OPT_LEVEL GREATER 0)
            target_compile_options(cruckig_test PRIVATE -O3 -flto)
            target_link_options(cruckig_test PRIVATE -flto)
            target_compile_options(cruckig_test_random PRIVATE -O3 -flto)
            target_link_options(cruckig_test_random PRIVATE -flto)
        endif()
    endif()

    # Cross-validation test: compare cruckig vs C++ Ruckig (optional)
    set(RUCKIG_DIR "" CACHE PATH "Path to Ruckig C++ source tree (for cross-validation test)")
    if(RUCKIG_DIR AND EXISTS "${RUCKIG_DIR}/include/ruckig/ruckig.hpp")
        enable_language(CXX)
        set(CMAKE_CXX_STANDARD 17)
        add_executable(cruckig_test_comparison test/test_comparison.cpp)
        target_include_directories(cruckig_test_comparison PRIVATE
            ${RUCKIG_DIR}/include
        )
        target_link_libraries(cruckig_test_comparison PRIVATE
            cruckig m
            ${RUCKIG_DIR}/build/libruckig.so
        )
        target_compile_options(cruckig_test_comparison PRIVATE -O2)
        message(STATUS "Cross-validation test enabled (Ruckig at ${RUCKIG_DIR})")
    endif()
endif()

# Benchmark
if(BUILD_BENCHMARK)
    add_executable(cruckig_benchmark test/benchmark.c)
    target_link_libraries(cruckig_benchmark PRIVATE cruckig m)

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        if(CRUCKIG_OPT_LEVEL GREATER 0)
            target_compile_options(cruckig_benchmark PRIVATE -O3 -flto)
            target_link_options(cruckig_benchmark PRIVATE -flto)
        endif()
    endif()
endif()

# Examples
if(BUILD_EXAMPLES)
    add_executable(cruckig_example_position examples/01_position.c)
    target_link_libraries(cruckig_example_position PRIVATE cruckig m)
endif()

# ============================================================================
# Install rules (with component separation: runtime vs development)
# ============================================================================

# --- Runtime component: shared library only ---
install(TARGETS cruckig_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT development
)

# --- Development component: static lib, headers, cmake config ---
install(TARGETS cruckig
    EXPORT cruckig-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT development
)

install(DIRECTORY include/cruckig
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT development
)

# CMake config files for find_package(cruckig)
set(CRUCKIG_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(EXPORT cruckig-targets
    DESTINATION ${CRUCKIG_INSTALL_CONFIGDIR}
    NAMESPACE cruckig::
    COMPONENT development
)

configure_package_config_file(
    cmake/cruckig-config.cmake.in
    cruckig-config.cmake
    INSTALL_DESTINATION ${CRUCKIG_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    cruckig-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cruckig-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cruckig-config-version.cmake"
    DESTINATION ${CRUCKIG_INSTALL_CONFIGDIR}
    COMPONENT development
)

# ============================================================================
# CPack / DEB packaging  (two packages: libcruckig + libcruckig-dev)
# ============================================================================
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "cruckig")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_PROCESSOR})

# Enable component-based packaging: one .deb per component
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime development)

# ---------- Component metadata ----------
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "cruckig runtime library")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION
    "Shared library for cruckig jerk-limited trajectory generation.")

set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "cruckig development files")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION
    "Headers, static library, and CMake config for cruckig.")
set(CPACK_COMPONENT_DEVELOPMENT_DEPENDS runtime)

# ---------- Common DEB fields ----------
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "cruckig maintainers")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# ---------- Per-component DEB fields ----------
# runtime  →  libcruckig_0.1.0_amd64.deb
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "libcruckig")
set(CPACK_DEBIAN_RUNTIME_FILE_NAME "libcruckig_${PROJECT_VERSION}_${CMAKE_HOST_SYSTEM_PROCESSOR}.deb")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_DEPENDS "libc6 (>= 2.17)")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_DESCRIPTION
"Pure C jerk-limited real-time trajectory generation (runtime)
 cruckig generates time-optimal, jerk-limited trajectories for robots
 and machines.  This package contains the shared library.")

# development  →  libcruckig-dev_0.1.0_amd64.deb
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_NAME "libcruckig-dev")
set(CPACK_DEBIAN_DEVELOPMENT_FILE_NAME "libcruckig-dev_${PROJECT_VERSION}_${CMAKE_HOST_SYSTEM_PROCESSOR}.deb")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_DEPENDS "libcruckig (= ${PROJECT_VERSION})")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_DESCRIPTION
"Pure C jerk-limited real-time trajectory generation (development)
 Headers, static library (.a), and CMake find_package() config files
 for building against cruckig.")

include(CPack)

# Print summary
message(STATUS "cruckig v${PROJECT_VERSION} | OPT_LEVEL=${CRUCKIG_OPT_LEVEL}")
