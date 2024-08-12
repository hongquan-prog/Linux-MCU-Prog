cmake_minimum_required(VERSION 3.16)

project(algorithm)

# target
add_executable(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp ${CMAKE_SOURCE_DIR}/../Program/src/algorithm.cpp)
# include path
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../Program/inc)
