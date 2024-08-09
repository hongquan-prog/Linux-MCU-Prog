include(${CMAKE_SOURCE_DIR}/cmake/utility.cmake)
add_library(Program STATIC
    "src/swd_iface.cpp"
    "src/target_flash.cpp"
    "src/flash_accessor.cpp"
    "src/bin_program.cpp"
    "src/hex_parser.c"
    "src/hex_program.cpp"
    "src/algorithm.cpp"
    "src/file_programmer.cpp"
    "src/stream_programmer.cpp"
)

target_include_directories(Program PUBLIC "inc")
redefine_file_name(Program)

# add_library(Program STATIC IMPORTED GLOBAL)  
# set_target_properties(Program PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libProgram.a)
# target_include_directories(Program INTERFACE "inc")