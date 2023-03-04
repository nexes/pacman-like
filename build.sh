#!/bin/bash

echo "Creating build folder"
mkdir ./build/

echo "Building..."
cmake -B ./build/ -S .

cd ./build/
make -j 14

echo "Build finished"