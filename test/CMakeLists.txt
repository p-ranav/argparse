
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # Update if necessary
  set(ARGPARSE_PEDANTIC_COMPILE_FLAGS -Wall -Wno-long-long -Wpedantic -Wsign-conversion -Wshadow -Wconversion -Werror -Wextra)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Update if necessary
  set(ARGPARSE_PEDANTIC_COMPILE_FLAGS -Wall -Wno-long-long -Wpedantic -Wsign-conversion -Wshadow -Wconversion -Werror -Wextra)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Disable deprecation for windows
if (WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# ARGPARSE executable
file(GLOB ARGPARSE_TEST_SOURCES
    main.cpp
    test_actions.cpp
    test_append.cpp
    test_as_container.cpp
    test_bool_operator.cpp
    test_choices.cpp
    test_compound_arguments.cpp
    test_container_arguments.cpp
    test_default_args.cpp
    test_default_value.cpp
    test_error_reporting.cpp
    test_get.cpp
    test_help.cpp
    test_invalid_arguments.cpp
    test_is_used.cpp
    test_issue_37.cpp
    test_mutually_exclusive_group.cpp
    test_negative_numbers.cpp
    test_optional_arguments.cpp
    test_parent_parsers.cpp
    test_parse_args.cpp
    test_positional_arguments.cpp
    test_repr.cpp
    test_required_arguments.cpp
    test_scan.cpp
    test_stringstream.cpp
    test_version.cpp
    test_subparsers.cpp
    test_parse_known_args.cpp
    test_equals_form.cpp
    test_prefix_chars.cpp
)

add_executable(argparse_tests ${ARGPARSE_TEST_SOURCES})
if(ARGPARSE_PEDANTIC_COMPILE_FLAGS)
  target_compile_options(argparse_tests PRIVATE ${ARGPARSE_PEDANTIC_COMPILE_FLAGS})
endif()
target_link_libraries(argparse_tests PRIVATE argparse::argparse)

add_test(NAME argparse_tests COMMAND argparse_tests)
