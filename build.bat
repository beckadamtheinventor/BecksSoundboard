mkdir build
pushd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
copy /y compile_commands.json ..
cmake --build . -j
popd