cmake_minimum_required(VERSION 3.22)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
  include(${CMAKE_TOOLCHAIN_FILE})
endif()

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16 -O3)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
# Set the project name
set(CMAKE_PROJECT_NAME SUPERCAP)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# collect core and user sources and add to target
file(GLOB_RECURSE CORE_SRCS
    "${CMAKE_SOURCE_DIR}/Core/Src/*.c"
)

list(REMOVE_ITEM CORE_SRCS "${CMAKE_SOURCE_DIR}/Core/Src/system_stm32g4xx.c")

file(GLOB_RECURSE USER_SRCS
    "${CMAKE_SOURCE_DIR}/User/*.c"
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CORE_SRCS}
    ${USER_SRCS}
)

# add include paths (ensure headers like CAN_communicate.h 被找到)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/User
    ${CMAKE_SOURCE_DIR}/User/interface
    ${CMAKE_SOURCE_DIR}/User/data
    ${CMAKE_SOURCE_DIR}/User/math_tools
    ${CMAKE_SOURCE_DIR}/User/v1/src
)

# Add include directories for ARM toolchain
include_directories(/usr/lib/arm-none-eabi/include)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
