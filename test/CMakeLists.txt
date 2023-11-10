

# ARGPARSE test executable
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
