#!/bin/sh
rm -rf build2
mkdir build2
cd build2
cmake ..  -DHAVE_VULKAN=NO -DHAVE_GLES2=NO
