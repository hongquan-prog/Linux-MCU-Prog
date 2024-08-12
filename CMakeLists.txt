cmake_minimum_required(VERSION 3.10)  

project(Prog VERSION 1.1.3)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS  "-Wall -pipe -fmerge-all-constants -fno-strict-aliasing -ffunction-sections -fdata-sections -Wno-builtin-macro-redefined")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fsanitize=address,leak")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fsanitize=address,leak")
set(ALGORITHM_BIN ${CMAKE_BINARY_DIR}/algorithm.bin)
set(ALGORITHM_SRC ${CMAKE_BINARY_DIR}/algorithm.cpp)
file(GLOB ALGORITHM_FLM "${CMAKE_SOURCE_DIR}/algorithm/**/*.FLM" "${CMAKE_SOURCE_DIR}/algorithm/**/**/*.FLM")

add_custom_command(  
    OUTPUT ${ALGORITHM_SRC}  
    COMMAND cmake -S . -B ${CMAKE_BINARY_DIR}/Algorithm -DCMAKE_PROJECT_NAME=Algorithm  
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/components/Tool  
    COMMAND cmake --build ${CMAKE_BINARY_DIR}/Algorithm  
    COMMAND ${CMAKE_BINARY_DIR}/Algorithm/Algorithm ${ALGORITHM_BIN} ${ALGORITHM_FLM}  
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/array.py ${ALGORITHM_BIN} ${ALGORITHM_SRC}   
    COMMENT "Generating ${ALGORITHM_SRC}"
)

add_custom_target(Algorithm  
    DEPENDS ${ALGORITHM_SRC}  
)

include(${CMAKE_SOURCE_DIR}/scripts/utility.cmake)
add_subdirectory(components/Program)
add_subdirectory(components/SWD)
add_executable(Prog ${CMAKE_SOURCE_DIR}/main/main.cpp ${ALGORITHM_SRC})
target_link_libraries(Prog PRIVATE Program SWD)
set_target_properties(Prog PROPERTIES SUFFIX .elf)
target_link_options(Prog PRIVATE
    -pipe 
    -fmerge-all-constants 
    -Wl,--as-needed,--gc-sections 
    -Wl,--hash-style=gnu
)
redefine_file_name(Prog)

add_dependencies(Prog Algorithm)