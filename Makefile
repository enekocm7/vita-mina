BUILD_DIR ?= build
BUILD_TYPE ?= Release
JOBS ?= 4
CMAKE ?= cmake
HOST_CXX ?= g++

APP_NAME := vita_minesweeper
VPK := $(BUILD_DIR)/$(APP_NAME).vpk
SELF := $(BUILD_DIR)/$(APP_NAME).self
BOARD_TEST := $(BUILD_DIR)/board_tests

.PHONY: all configure vpk self test clean rebuild help

all: vpk

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

vpk: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS) --target $(APP_NAME).vpk-vpk
	@printf '\nCreated %s\n' "$(VPK)"

self: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS) --target $(APP_NAME).self-self
	@printf '\nCreated %s\n' "$(SELF)"

test: configure
	$(HOST_CXX) -std=c++17 -Wall -Wextra -Wpedantic -Isrc \
		src/board.cpp tests/board_test.cpp -o $(BOARD_TEST)
	$(BOARD_TEST)

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)

rebuild: clean all

help:
	@printf '%s\n' \
		'make          Build the installable Vita VPK' \
		'make self     Build only the Vita SELF executable' \
		'make test     Build and run board logic tests on the host' \
		'make rebuild  Delete the build directory and rebuild the VPK' \
		'make clean    Delete generated build files' \
		'make help     Show this help' \
		'' \
		'Optional variables: BUILD_DIR, BUILD_TYPE, JOBS, CMAKE, HOST_CXX'
