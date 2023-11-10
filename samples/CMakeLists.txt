
function(add_sample NAME)
  add_executable(argparse_sample_${NAME} ${NAME}.cpp)
  target_link_libraries(argparse_sample_${NAME} PRIVATE argparse::argparse)
  if(ARGPARSE_PEDANTIC_COMPILE_FLAGS)
    target_compile_options(argparse_sample_${NAME} PRIVATE ${ARGPARSE_PEDANTIC_COMPILE_FLAGS})
  endif()

  # Disable deprecation for windows
  target_compile_definitions(argparse_sample_${NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

  set_target_properties(argparse_sample_${NAME} PROPERTIES OUTPUT_NAME ${NAME})
endfunction()

add_sample(positional_argument)
add_sample(optional_flag_argument)
add_sample(required_optional_argument)
add_sample(is_used)
add_sample(joining_repeated_optional_arguments)
add_sample(repeating_argument_to_increase_value)
add_sample(negative_numbers)
add_sample(description_epilog_metavar)
add_sample(list_of_arguments)
add_sample(compound_arguments)
add_sample(gathering_remaining_arguments)
add_sample(subcommands)
add_sample(parse_known_args)
add_sample(custom_prefix_characters)
add_sample(custom_assignment_characters)
