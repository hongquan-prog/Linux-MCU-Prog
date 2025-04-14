.PHONY: all clean update flash menuconfig rebuild  

# Build type and output directory
BUILD_TYPE := RELEASE
BUILD_OUTPUT_DIR := build/release
TOOLCHAIN_FILE = 
CROSS_COMPILER =

ifneq ($(CROSS_COMPILER),)
	export CROSS_COMPILER=$(CROSS_COMPILER)
endif

all: config
	cmake --build ${BUILD_OUTPUT_DIR}
	cd $(BUILD_OUTPUT_DIR)

config:
ifeq ($(TOOLCHAIN_FILE),)
	cmake -S . -B ${BUILD_OUTPUT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
else
	cmake -S . -B ${BUILD_OUTPUT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE)
endif

clean:
	rm -rf $(BUILD_OUTPUT_DIR)

rebuild: clean all

menuconfig:
	cmake-gui ${BUILD_OUTPUT_DIR}
