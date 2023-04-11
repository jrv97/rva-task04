cmake_minimum_required(VERSION 3.0)
project(ARBockProject)

set(CMAKE_CXX_FLAGS "-std=c++11")

find_package(OpenCV 4.5.4 REQUIRED)
LINK_LIBRARIES(${OpenCV_LIBS})

add_executable(basicar basicar.cpp rva.cpp)
