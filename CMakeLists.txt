cmake_minimum_required(VERSION 3.5)

# Set Version Number
set(meshtool_VERSION_MAJOR 0)
set(meshtool_VERSION_MINOR 1)
set(CMAKE_CC_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER $ENV{CXX})

project (VoxelArt)

# Check for system type, Darwin (MACOS) or Linux
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    message("Found a MACOS environment...")
    set(DARWIN true)
endif()

# Set CXX flags
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17 -g -lGL")

# Set package variables
message("CMAKE_PREFIX_PATH= " ${CMAKE_PREFIX_PATH})
message("CMAKE_SYSTEM_NAME= " ${CMAKE_SYSTEM_NAME})

# Find Packages
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} src/include/meshtool/glm/glm ${FREETYPE_INCLUDE_DIRS})
message("GLEW_INCLUDE_DIR= " ${GLEW_INCLUDE_DIR})
message("GLEW_LIBRARIES= " ${GLEW_LIBRARIES})
message("FREETYPE_INCLUDE_DIRS= " ${FREETYPE_INCLUDE_DIRS})
message("FREETYPE_LIBRARIES= " ${FREETYPE_LIBRARIES})

# Source file lists
set(MAIN_SOURCE src/all.cpp) #Unitybuild

# Library Section

# Main Executable Section
add_executable(meshtool ${MAIN_SOURCE})
target_include_directories(meshtool PUBLIC src src/include/meshtool)
target_link_libraries(meshtool ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} /usr/lib/x86_64-linux-gnu/mesa/libGL.so ${FREETYPE_LIBRARIES})

# Utilities Executable Section

# Unit Tests Section

# Library Test Section

# Ogre Testing section

message("Completed parse of CMakeList.txt for meshtool project...")
message("Compiler chosen= " ${CMAKE_CXX_COMPILER})
