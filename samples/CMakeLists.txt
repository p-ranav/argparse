cmake_minimum_required(VERSION 3.6)
project(argparse_samples)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wsign-conversion -Wshadow -Wconversion")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Disable deprecation for windows
if (WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

function(add_sample NAME)
  ADD_EXECUTABLE(ARGPARSE_SAMPLE_${NAME} ${NAME}.cpp)
  INCLUDE_DIRECTORIES("../include" ".")
  TARGET_LINK_LIBRARIES(ARGPARSE_SAMPLE_${NAME} PRIVATE argparse::argparse)
  set_target_properties(ARGPARSE_SAMPLE_${NAME} PROPERTIES OUTPUT_NAME ${NAME})
endfunction()

add_sample(git_subcommands)
add_sample(square_a_number)