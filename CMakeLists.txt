cmake_minimum_required(VERSION 3.14)
project(algebraic_parsers VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options(-O2 -g)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(Threads REQUIRED)

# Optional: Find Google Test for testing
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # If GTest is not found, we'll download it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Create header-only interface library for the main parsers
add_library(algebraic_parsers INTERFACE)
target_include_directories(algebraic_parsers INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(algebraic_parsers INTERFACE cxx_std_20)

# Porter2 stemmer implementation (has .cpp file)
add_library(porter2stemmer 
    include/parsers/porter2stemmer.cpp
    include/parsers/porter2stemmer.h
    include/parsers/porter2stemmer.hpp
)
target_include_directories(porter2stemmer PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(porter2stemmer PUBLIC cxx_std_20)

# Link the porter2stemmer to the main library
target_link_libraries(algebraic_parsers INTERFACE porter2stemmer)

# ============================================================================
# Executables for testing individual components
# ============================================================================

# Test executables for the existing test files
add_executable(lc_alpha_test test/lc_alpha_test.cpp)
target_link_libraries(lc_alpha_test algebraic_parsers)

add_executable(porter2stemmer_test test/porter2stemmer_test.cpp)
target_link_libraries(porter2stemmer_test algebraic_parsers)

add_executable(fsm_string_rewriter_driver test/fsm_string_rewriter_driver.cpp)
target_link_libraries(fsm_string_rewriter_driver algebraic_parsers)

# ============================================================================
# Test Suite with Google Test
# ============================================================================

if(GTest_FOUND OR TARGET gtest)
    enable_testing()
    
    # Main composition test suite
    add_executable(composition_tests test/composition_test.cpp)
    target_link_libraries(composition_tests 
        algebraic_parsers 
        gtest 
        gtest_main 
        Threads::Threads
    )
    
    # Individual unit tests
    add_executable(unit_tests
        test/unit_tests.cpp  # We'll create this
    )
    target_link_libraries(unit_tests 
        algebraic_parsers 
        gtest 
        gtest_main 
        Threads::Threads
    )
    
    # Integration tests
    add_executable(integration_tests
        test/integration_tests.cpp  # We'll create this
    )
    target_link_libraries(integration_tests 
        algebraic_parsers 
        gtest 
        gtest_main 
        Threads::Threads
    )
    
    # Register tests with CTest
    add_test(NAME CompositionTests COMMAND composition_tests)
    add_test(NAME UnitTests COMMAND unit_tests)
    add_test(NAME IntegrationTests COMMAND integration_tests)
    
    # Test coverage target (if gcov/lcov available)
    if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage
                # Clean previous coverage data
                COMMAND ${LCOV_PATH} --directory . --zerocounters
                
                # Run tests
                COMMAND ctest
                
                # Capture coverage data
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                
                # Remove system headers and test files from coverage
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/test/*' '*/gtest/*' --output-file coverage_filtered.info
                
                # Generate HTML report
                COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage_html
                
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating test coverage report"
            )
        endif()
    endif()
endif()

# ============================================================================
# Documentation with Doxygen (optional)
# ============================================================================

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS algebraic_parsers porter2stemmer
    EXPORT algebraic_parsers_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

install(EXPORT algebraic_parsers_targets
    FILE algebraic_parsers_targets.cmake
    NAMESPACE algebraic_parsers::
    DESTINATION lib/cmake/algebraic_parsers
)

# Generate package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    algebraic_parsers_config_version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/algebraic_parsers_config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/algebraic_parsers_config.cmake
    INSTALL_DESTINATION lib/cmake/algebraic_parsers
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/algebraic_parsers_config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/algebraic_parsers_config_version.cmake
    DESTINATION lib/cmake/algebraic_parsers
)

# ============================================================================
# Custom Targets
# ============================================================================

# Format code using clang-format (if available)
find_program(CLANG_FORMAT_EXE NAMES "clang-format")
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/include/*.[ch]pp
        ${CMAKE_SOURCE_DIR}/include/*.[ch]
        ${CMAKE_SOURCE_DIR}/test/*.[ch]pp
        ${CMAKE_SOURCE_DIR}/examples/*.[ch]pp
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_CXX_SOURCE_FILES}
        COMMENT "Formatting source code"
    )
endif()

# Static analysis with cppcheck (if available)
find_program(CPPCHECK_EXE NAMES "cppcheck")
if(CPPCHECK_EXE)
    add_custom_target(static_analysis
        COMMAND ${CPPCHECK_EXE} 
            --enable=all 
            --std=c++20 
            --inconclusive 
            --force
            --inline-suppr
            --suppress=missingIncludeSystem
            -I ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/test
            ${CMAKE_SOURCE_DIR}/examples
        COMMENT "Running static analysis"
    )
endif()

# Print build configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "GTest found: ${GTest_FOUND}")
message(STATUS "Doxygen found: ${DOXYGEN_FOUND}")

if(CLANG_FORMAT_EXE)
    message(STATUS "clang-format available: use 'make format' to format code")
endif()

if(CPPCHECK_EXE)
    message(STATUS "cppcheck available: use 'make static_analysis' for static analysis")
endif()