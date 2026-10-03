cmake_minimum_required(VERSION 3.16)
project(DSP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(Dependencies)

add_library(DSP SHARED src/library.cpp)
target_include_directories(DSP PUBLIC src)

#target_link_libraries(DSP PRIVATE matplotlib-cpp)

enable_testing()
add_subdirectory(tests)
