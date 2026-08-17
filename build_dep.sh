rm -rf third/build/linux

cd DreamNet
rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX=../third/build/linux -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
cmake --install build