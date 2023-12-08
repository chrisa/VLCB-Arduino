#!/bin/sh

cmake -S. -Bbuild-pico -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/rp2040.toolchain.cmake -D CMAKE_BUILD_TYPE=Debug -GNinja
