#!/bin/sh
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../ps2sdk.cmake -DIMPORT_EXECUTABLES=../build2/ImportExecutables.cmake -DHAVE_VULKAN=NO -DHAVE_GLES2=NO
