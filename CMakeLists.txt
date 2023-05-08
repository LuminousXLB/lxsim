cmake_minimum_required(VERSION 3.20)

project(testbench)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Og -g -Wall -Wextra -Wpedantic)

set(VIVADO_INSTALLATION_ROOT
    "/tools/Xilinx/Vivado/2021.2"
    CACHE PATH "Vivado installation root")

set(XSIMK_DESIGN_LIB
    ""
    CACHE PATH "XSIMK design library")

if(NOT XSIMK_DESIGN_LIB)
  message(
    FATAL_ERROR
      "Create your design shared library by invoking xelab on the HDL design")
endif()

set(DIR_INCL ${VIVADO_INSTALLATION_ROOT}/data/xsim/include
             ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DIR_LINK ${VIVADO_INSTALLATION_ROOT}/lib/lnx64.o)
set(LIBRARIES rdi_simulator_kernel rdizlib tcl8.5)

include_directories(${DIR_INCL})
link_directories(${DIR_LINK})
link_libraries(${LIBRARIES})

add_library(xsi_kernel STATIC src/xsi_kernel.cpp)

add_executable(dut_generator src/generator.cpp)
target_link_libraries(dut_generator PRIVATE xsi_kernel)

add_custom_target(dut_include_dir COMMAND ${CMAKE_COMMAND} -E make_directory
                                          ${CMAKE_CURRENT_BINARY_DIR}/include)

add_custom_target(
  dut.hpp ALL
  COMMAND
    ${CMAKE_COMMAND} -E env
    LD_LIBRARY_PATH=${VIVADO_INSTALLATION_ROOT}/lib/lnx64.o
    $<TARGET_FILE:dut_generator> ${XSIMK_DESIGN_LIB} >
    ${CMAKE_CURRENT_BINARY_DIR}/include/dut.hpp
  DEPENDS dut_generator dut_include_dir
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/include/dut.hpp)

if(NOT ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}"))
  set(LXSIM_INCLUDE_DIR
      ${DIR_INCL} ${CMAKE_CURRENT_BINARY_DIR}/include
      PARENT_SCOPE)
  set(LXSIM_LINK_DIR
      ${DIR_LINK} ${CMAKE_CURRENT_BINARY_DIR}
      PARENT_SCOPE)
  set(LXSIM_LIBRARIES
      ${LIBRARIES} xsi_kernel
      PARENT_SCOPE)
endif()
