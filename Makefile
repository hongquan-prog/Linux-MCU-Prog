.PHONY: all clean update flash menuconfig rebuild  

# Build type and output directory
BUILD_TYPE := RELEASE
BUILD_OUTPUT_DIR := build/release

all: config
	cmake --build ${BUILD_OUTPUT_DIR}
	cd $(BUILD_OUTPUT_DIR)

config:
	cmake -S . -B ${BUILD_OUTPUT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

clean:
	rm -rf $(BUILD_OUTPUT_DIR)

rebuild: clean all

menuconfig:
	cmake-gui ${BUILD_OUTPUT_DIR}
