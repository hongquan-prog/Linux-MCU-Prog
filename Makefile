.PHONY: all clean update flash menuconfig rebuild  

# Build type and output directory
BUILD_TYPE := RELEASE
BUILD_OUTPUT_DIR := build/release
TOOLCHAIN_FILE = 
CROSS_COMPILER =

all: config
	cmake --build ${BUILD_OUTPUT_DIR}
	cd $(BUILD_OUTPUT_DIR)

config:
ifeq ($(TOOLCHAIN_FILE),)
	cmake -S . -B ${BUILD_OUTPUT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCROSS_COMPILER=$(CROSS_COMPILER)
else
	cmake -S . -B ${BUILD_OUTPUT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCROSS_COMPILER=$(CROSS_COMPILER) -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE)
endif

clean:
	rm -rf $(BUILD_OUTPUT_DIR)

rebuild: clean all

menuconfig:
	cmake-gui ${BUILD_OUTPUT_DIR}
