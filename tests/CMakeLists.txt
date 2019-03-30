cmake_minimum_required(VERSION 3.6)
project(ARGPARSE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Disable deprecation for windows
if (WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ARGPARSE executable
file(GLOB ARGPARSE_TEST_SOURCES
    "*.cpp"
    "*.hpp"
    "../src/argparse.hpp"
)
ADD_EXECUTABLE(ARGPARSE ${ARGPARSE_TEST_SOURCES})
INCLUDE_DIRECTORIES("../src" ".")
set_target_properties(ARGPARSE PROPERTIES OUTPUT_NAME tests)
set_property(TARGET ARGPARSE PROPERTY CXX_STANDARD 17)

# Set ${PROJECT_NAME} as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ARGPARSE)