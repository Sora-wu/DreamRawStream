rm -rf third/build/linux

cd third/src/DreamNet
rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX=../../build/linux -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
cmake --install build