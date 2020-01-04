# CMake find scripts

This repo contrains find scripts for several libraries that are installed in non-system locations.

To use, your new project should include the following line at the beginning of your top most CMakeLists.txt

```
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{A17_ROOT}/cmake)
```
