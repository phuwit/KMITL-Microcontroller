cmake_minimum_required(VERSION 3.22)

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

# Set the project name
set(CMAKE_PROJECT_NAME Lab8)
set(LIB_ILI9341_DIR Core/Inc)
set(LIB_ILI9341_H Core/Inc/5x5_font.h Core/Inc/ILI9341_GFX.h Core/Inc/ILI9341_STM32_Driver.h Core/Inc/ILI9341_Touchscreen.h)
set(LIB_ILI9341_C Core/Src/ILI9341_GFX.c Core/Src/ILI9341_STM32_Driver.c Core/Src/ILI9341_Touchscreen.c)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME} ${LIB_ILI9341_H})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${LIB_ILI9341_DIR}
)

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
    ${LIB_ILI9341_H}
)

file(GLOB_RECURSE ALL_HEADERS "${LIB_ILI9341_DIR}/*.h")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${LIB_ILI9341_DIR})
add_library(LIB_ILI9341 ${LIB_ILI9341_C} ${LIB_ILI9341_H})
target_include_directories(LIB_ILI9341 PRIVATE ${LIB_ILI9341_DIR} ${CMAKE_PROJECT_NAME} Drivers/ Core/Inc Drivers/CMSIS/Device/ST/STM32F7xx/Include Drivers/STM32F7xx_HAL_Driver/Inc cmake/stm32cubemx)