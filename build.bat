mkdir build
pushd build
cmake -G "Visual Studio 17 2022" -A x64 .. -DCMAKE_BUILD_TYPE=Release
copy /y compile_commands.json ..
cmake --build . -j
popd