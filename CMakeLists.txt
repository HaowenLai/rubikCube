cmake_minimum_required(VERSION 3.2)

project(RubikCube)

find_package(OpenCV REQUIRED)
#find_package(PythonLibs REQUIRED)

#use c++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#debug mode, use gdb to debug
#SET(CMAKE_BUILD_TYPE "Debug") 
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")


#SET(PYTHON_INCLUDE_PATH "/usr/include/python3.5")
#SET(PYTHON_LIBRARIES "/usr/lib/x86_64-linux-gnu/libpython3.5m.so")

include_directories(
 include
 /usr/local/include
 ${OpenCV_INCLUDE_DIRS}
# ${PYTHON_INCLUDE_PATH}
)

link_directories(data)

#all source put here
set(stimulateSrcList 
    exec/stimulate.cpp
    src/RubikCube.cpp)



add_executable(stimulate ${stimulateSrcList})

target_link_libraries(stimulate ${OpenCV_LIBS} pthread kociemba)
#target_link_libraries(startIt pthread usbcan)

