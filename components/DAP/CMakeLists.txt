include(${CMAKE_SOURCE_DIR}/cmake/utility.cmake)
add_library(DAP STATIC
			"Source/DAP.c"
			"Source/DAP_vendor.c"
			"Source/SW_DP.c"
)

target_include_directories(DAP PUBLIC "Include" "Config")
target_link_libraries(DAP PRIVATE Platform)
redefine_file_name(DAP)
