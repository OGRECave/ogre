macro(getenv_path VAR)
   set(ENV_${VAR} $ENV{${VAR}})
   # replace won't work if var is blank
   if (ENV_${VAR})
     string( REGEX REPLACE "\\\\" "/" ENV_${VAR} ${ENV_${VAR}} )
   endif ()
endmacro(getenv_path)


getenv_path(L4TROOT)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER ${ENV_L4TROOT}/_out/3rdparty/arm-2009q1/bin/arm-none-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${ENV_L4TROOT}/_out/3rdparty/arm-2009q1/bin/arm-none-linux-gnueabi-g++)

set(ZLIB_INC_SEARCH_PATH ${ENV_L4TROOT}/_out/targetfs/usr/include)

set(CMAKE_FIND_ROOT_PATH ${ENV_L4TROOT}/_out/3rdparty/arm-2009q1 ${ENV_L4TROOT}_out/targetfs/usr/lib/gcc/arm-linux-gnueabi/4.3 ${ENV_L4TROOT}/_out/targetfs/ ${ENV_L4TROOT}/_out/targetfs/usr ${ENV_L4TROOT}/_out/3rdparty/arm-2009q1/arm-none-linux-gnueabi/libc/usr/lib ${ENV_L4TROOT}/_out/3rdparty/xorg/arm-none-linux-gnueabi/ ${CMAKE_CURRENT_SOURCE_DIR}/Tegra2Dependencies)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

