INCLUDE(CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME Linux)

SET(PLATFORM_PATH "$ENV{ANDROID_NDK_ROOT}/platforms/android-8")

SET(PLATFORM_NAME windows)

CMAKE_FORCE_C_COMPILER("$ENV{ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/${PLATFORM_NAME}/bin/arm-linux-androideabi-gcc.exe" GNU)
CMAKE_FORCE_CXX_COMPILER("$ENV{ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/${PLATFORM_NAME}/bin/arm-linux-androideabi-g++.exe" GNU)

SET(CMAKE_FIND_ROOT_PATH "$ENV{ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/${PLATFORM_NAME}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

INCLUDE_DIRECTORIES(${PLATFORM_PATH}/arch-arm/usr/include)

SET(CMAKE_SHARED_LINKER_FLAGS
-Wl,--no-undefined
)