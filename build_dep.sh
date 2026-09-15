rm -rf third/build/linux

cd third/src/DreamNet
rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX=../../build/linux -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
cmake --install build

cd ../ffmpeg
make clean
./configure --prefix=../../build/linux --enable-shared --disable-programs --enable-libx264 --enable-gpl \
  --enable-libfdk-aac --enable-nonfree --enable-libx265
make -j
make install