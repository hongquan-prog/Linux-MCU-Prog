include(${CMAKE_SOURCE_DIR}/cmake/utility.cmake)
add_library(Platform STATIC
			"Source/imx_gpio.c"
            "Source/fsl_rgpio.c"
)

target_include_directories(Platform PUBLIC Include)
redefine_file_name(Platform)
