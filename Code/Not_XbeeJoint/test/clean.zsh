#!/bin/zsh

rm -rf build
mkdir build
cd build
export PICO_SDK_PATH=../../pico-sdk
cmake ..
make