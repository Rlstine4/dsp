# DSP

**A header-only C++20 DSP library**

## Requirements

- CMake 3.26 or newer
- C++20 compiler
- Network access if using CPM to bring in packages

## Build

The project provides CMake presets. Build files are written to
`build/<preset>/` directory; CMake's default generator (e.g Ninja) is used.

| Preset | Build type | Tests |
| --- | --- | --- |
| `debug` | Debug | Off |
| `release` | Release | Off |
| `test` | Debug | On |

```sh
cmake --preset debug
cmake --build --preset debug
```

Dependencies are managed with [CPM](https://github.com/cpm-cmake/CPM.cmake).
By default, CPM first attempts to use locally installed packages and fetches missing
dependencies during configuration.

## Tests

Tests use [Catch2](https://github.com/catchorg/Catch2) and are registered with
CTest. The `test` preset enables them:

```sh
cmake --preset test
cmake --build --preset test
ctest --preset test
```
