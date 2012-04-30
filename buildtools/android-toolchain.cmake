INCLUDE(CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME Linux)

SET(PLATFORM_NDK_PATH "$ENV{ANDROID_NDK_ROOT}/platforms/android-8")
SET(PLATFORM_SDK_PATH "$ENV{ANDROID_SDK_ROOT}/platforms/android-8")

IF (CMAKE_HOST_UNIX)
SET(PLATFORM_NAME linux-x86)
ELSE()
SET(PLATFORM_NAME windows)
SET(EXE_SUFFIX .exe)
ENDIF()

CMAKE_FORCE_C_COMPILER("$ENV{ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/${PLATFORM_NAME}/bin/arm-linux-androideabi-gcc${EXE_SUFFIX}" GNU)
CMAKE_FORCE_CXX_COMPILER("$ENV{ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/${PLATFORM_NAME}/bin/arm-linux-androideabi-g++${EXE_SUFFIX}" GNU)

SET(CMAKE_FIND_ROOT_PATH "$ENV{ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/${PLATFORM_NAME}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

INCLUDE_DIRECTORIES(
${PLATFORM_NDK_PATH}/arch-arm/usr/include
$ENV{ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/include
$ENV{ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/armeabi/include
)

SET(LINK_DIR ${PLATFORM_NDK_PATH}/arch-arm/usr/lib) 
SET(CMAKE_SHARED_LINKER_FLAGS
"-Wl,--no-undefined --sysroot=${PLATFORM_NDK_PATH}/arch-arm -L${LINK_DIR} -L$ENV{ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/armeabi -lgnustl_shared"
)
