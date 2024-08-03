include(${CMAKE_SOURCE_DIR}/cmake/utility.cmake)
add_library(SWD STATIC
    "src/imx93_swd.cpp"
    "src/ah618_swd.cpp"
    "src/rk3588_swd.cpp"
    "src/imx93_gpio.c"
    "src/ah618_gpio.c"
    "src/rk3588_gpio.c"
)

target_include_directories(SWD PUBLIC "inc")
target_link_libraries(SWD PRIVATE Program)
redefine_file_name(SWD)
