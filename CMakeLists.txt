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
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fsanitize=address,leak,thread")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fsanitize=address,leak,thread")

include(${CMAKE_SOURCE_DIR}/cmake/utility.cmake)
add_subdirectory(components/DAP)
add_subdirectory(components/Platform)
add_subdirectory(components/Program)
add_executable(Prog ${CMAKE_SOURCE_DIR}/main/main.cpp)

target_link_libraries(Prog DAP Platform Program)
set_target_properties(Prog PROPERTIES SUFFIX .elf)
target_link_options(Prog PRIVATE
    -pipe 
    -fmerge-all-constants 
    -Wl,--as-needed,--gc-sections 
    -Wl,--hash-style=gnu
)
redefine_file_name(Prog)
