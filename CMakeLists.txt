cmake_minimum_required(VERSION 3.2)
project(RubikCube)

find_package(OpenCV REQUIRED)
#find_package(PythonLibs REQUIRED)

# use c++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# debug mode, use gdb to debug
#SET(CMAKE_BUILD_TYPE "Debug") 
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

# set path for package that cannot be found
#SET(PYTHON_INCLUDE_PATH "/usr/include/python3.5")
#SET(PYTHON_LIBRARIES "/usr/lib/x86_64-linux-gnu/libpython3.5m.so")

include_directories(
 include
 /usr/local/include
 ${OpenCV_INCLUDE_DIRS}
# ${PYTHON_INCLUDE_PATH}
)

# specify link library directory
#link_directories(data)

# Add block directories
add_subdirectory(kociemba_genLib)

# all source put here
set(simulateSrcList 
    exec/simulate.cpp
    src/RubikCube.cpp)

set(solveCubeManuallySrcList 
    exec/solveCubeManually.cpp
    src/RubikCube.cpp
    src/Motor.cpp)

set(disorganizeSrcList
    exec/disorganize.cpp
    src/RubikCube.cpp
    src/Motor.cpp)

set(colorToolSrcList
    exec/colorTool.cpp
    src/RubikCube.cpp
    src/colorRecog.cpp)

set(readColorSrcList
    exec/readColor.cpp
    src/RubikCube.cpp
    src/colorRecog.cpp)

add_executable(simulate ${simulateSrcList})
#add_executable(solveCubeManually ${solveCubeManuallySrcList})
#add_executable(disorganize ${disorganizeSrcList})
add_executable(colorTool ${colorToolSrcList})
add_executable(readColor ${readColorSrcList})


target_link_libraries(simulate ${OpenCV_LIBS} pthread kociemba)
#target_link_libraries(solveCubeManually ${OpenCV_LIBS} wiringPi kociemba)
#target_link_libraries(disorganize ${OpenCV_LIBS} wiringPi)
target_link_libraries(colorTool ${OpenCV_LIBS})
target_link_libraries(readColor ${OpenCV_LIBS})
