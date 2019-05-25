cmake_minimum_required(VERSION 3.8)
project(argparse VERSION 1.0.0 LANGUAGES CXX)
option(ARGPARSE_BUILD_TESTS OFF)

include(GNUInstallDirs)

add_library(argparse INTERFACE)
add_library(argparse::argparse ALIAS argparse)

target_compile_features(argparse INTERFACE cxx_std_17)
target_include_directories(argparse INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

if(ARGPARSE_BUILD_TESTS)
  add_subdirectory(test)
endif()

install(TARGETS argparse EXPORT argparseConfig)
install(EXPORT argparseConfig
        NAMESPACE argparse::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/argparse)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/include/argparse.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/argparse)
