include(${CMAKE_SOURCE_DIR}/cmake/utility.cmake)
add_library(Program STATIC
    "src/swd_host.c"
    "src/swd_iface.cpp"
    "src/target_swd.cpp"
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
target_link_libraries(Program PRIVATE DAP Platform)
redefine_file_name(Program)
