mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cp compile_commands.json ..
cmake --build . -j12
cd ..