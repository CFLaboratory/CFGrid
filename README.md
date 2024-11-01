`CFGrid` is a mesh-handling utility library, it is designed to support the common operations
required by mesh-based codes, principally mesh reading and partitioning.
It is intended for use in an HPC environment so, in addition to reading mesh files that are often
written for workstation use, `CFGrid` supports writing the partitioned mesh in an HPC-focused format
for use in subsequent runs.

# Building

`CFGrid`'s build system is based around `CMake`.
To perform a basic configuration run
```
cmake -B build .
```
from the project root directory, this will generate a build system under `build/` which can be
further configured using the `ccmake` utility.
`CFGrid` can then be built via `cmake`:
```
cmake --build build
```

Testing is enabled by default (see below) and, depending on your system setup, this may cause
configuration errors if `cmake` can't find the testing framework.
Building the tests can be disabled by including `-DBUILD_TESTING=OFF` in the configuration
arguments, *i.e.*
```
cmake -B build . -DBUILD_TESTING=OFF
```

## Testing

`CFGrid` uses the [Catch2](https://github.com/catchorg/Catch2) testing framework.
The build system must be able to find `Catch` in order to build the tests by setting `Catch2_DIR`,
this can be done interactively through `ccmake` or via environment variables, for example
```
export Catch2_DIR=/path/to/Catch2/lib/cmake/Catch2
cmake -B build .
```
after which the build system should be able to find `Catch2`.

Once built, the tests can be run via the `ctest` utility
```
ctest --test-dir build
```

# License

`CFGrid` is released under the Apache 2.0 License, see the [LICENSE](LICENSE) file for further
details.
