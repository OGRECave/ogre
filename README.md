# OGRE LoongArch64 LSX Optimizations

This branch (`loongarch-lsx-support`) contains optimizations for LoongArch64 architecture, specifically targeting Loongson 3B6000M series processors.

## Changes Made

### CMakeLists.txt (Lines 121-147)

Added LoongArch64-specific compiler optimizations in the CMake configuration:

```cmake
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "loongarch|LoongArch|LOONGARCH")
  # LoongArch specific optimizations
  include(CheckCXXCompilerFlag)
  # Check for LoongArch SIMD extensions (LSX/ LASX)
  check_cxx_compiler_flag(-march=loongarch64 OGRE_LOONGARCH_HAS_ARCH)
  if (OGRE_LOONGARCH_HAS_ARCH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=loongarch64")
  endif ()
  # Check for LoongArch SIMD (LSX) - supported on 3B6000 series
  check_cxx_compiler_flag(-mlsx OGRE_LOONGARCH_HAS_LSX)
  if (OGRE_LOONGARCH_HAS_LSX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlsx")
  endif ()
  # Note: LASX is not enabled as it's not supported on Loongson-3B6000M
  # Loongson-3B6000M only supports LSX, not LASX
```

## What These Optimizations Do

1. **Architecture Detection**: Automatically detects LoongArch64 processors
2. **Base Architecture**: Enables `-march=loongarch64` for optimal instruction set usage
3. **LSX SIMD**: Enables `-mlsx` to utilize Loongson SIMD extensions for improved vector operations
4. **Compatibility**: Specifically designed for Loongarch series (supports LSX but not LASX)

## Benefits

- Improved performance for vector mathematics operations
- Better matrix transformation performance
- Enhanced texture processing capabilities
- Optimized geometry calculations

## Building

To build with these optimizations, simply use the standard OGRE build process:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

The optimizations will be automatically detected and applied on LoongArch64 systems.