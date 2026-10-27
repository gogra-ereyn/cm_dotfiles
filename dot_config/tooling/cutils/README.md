# cuteils: cute utils

## Installation

Installs to ~/.local/bin by default, defined in cmake/common.cmake.

To install simply:

`make install`

which builds everything, and copies to the install dir. 

To install to a different location: 

```cmake
cmake -B build -DCMAKE_INSTALL_PREFIX=/some/other/path
cmake --build build
cmake --install build
```

