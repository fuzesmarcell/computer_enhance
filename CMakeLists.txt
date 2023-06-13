cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(ComputerEnhance VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20) # We should probably downconvert to 17
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_executable(haversine_generator "haversine_generator.cpp" "haversine_reference.h")
add_library(ce_json "ce_json.h" "ce_json.cpp")
add_executable(haversine "haversine.cpp")
target_link_libraries(haversine ce_json)