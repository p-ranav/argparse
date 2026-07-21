cmake_minimum_required(VERSION 3.28)

add_library(argparse_modules)

add_library(argparse::modules ALIAS argparse_modules)

target_sources(
  argparse_modules PUBLIC FILE_SET CXX_MODULES FILES
                          argparse.cppm)

target_link_libraries(argparse_modules PRIVATE argparse::argparse)

if(ARGPARSE_MODULE_USE_STD_MODULE)
  target_compile_definitions(argparse_modules
                             PRIVATE -DARGPARSE_MODULE_USE_STD_MODULE=1)
  
  target_compile_features(argparse_modules PUBLIC cxx_std_23)
else()
  target_compile_features(argparse_modules PUBLIC cxx_std_20)
endif()
