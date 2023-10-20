Requires cmake 3.20 or later, and C++20 compiler.

`git submodule init`

`git submodule update`

`cd ../`

`mkdir build`

`cd build`

Example cmake generators:

`cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../false_sharing_test`

`cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release ../false_sharing_test`

`cmake -G "Visual Studio 17 2022 ../false_sharing_test`

Then:

`make main -j 16`

Used Google Benchmark was `f30c99a7c861e8cabc6d3d9c0b60a4f218c7f87a` (1.8.3 as release at the time)
