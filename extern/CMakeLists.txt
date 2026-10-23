# Third-party libraries. Each one is a target that carries its own include
# directories and definitions; nothing here changes global compiler settings.

# MSVC: the old library sources use the "insecure" CRT functions
function(cge_extern_defaults target)
  if (MSVC)
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
  endif()
endfunction()

add_custom_target(unpack
  tar -xzf "${CMAKE_CURRENT_SOURCE_DIR}/../contrib/libxml2-2.7.2.tar.gz"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../contrib
  COMMENT "unpacking archives"
  VERBATIM
)

# zlib, libpng, libjpeg, FreeType, Dear ImGui, miniaudio, libogg and libvorbis
# are the same copies as in guitads (see guitads' CMakeLists.txt), so both
# projects use the same library versions.
set(BUILD_SHARED_LIBS OFF)
set(SKIP_INSTALL_ALL ON)

add_subdirectory(zlib)

# minizip (zip archive reading and writing) comes with zlib
set(MINIZIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib/contrib/minizip)
add_library(minizip STATIC
  ${MINIZIP_DIR}/ioapi.c
  ${MINIZIP_DIR}/ioapi.h
  ${MINIZIP_DIR}/unzip.c
  ${MINIZIP_DIR}/unzip.h
  ${MINIZIP_DIR}/zip.c
  ${MINIZIP_DIR}/zip.h
)
target_include_directories(minizip PUBLIC ${MINIZIP_DIR})
target_link_libraries(minizip PUBLIC ZLIB::ZLIB)
cge_extern_defaults(minizip)

# libpng enables ASM in its project() call; see libpng-policy-fixup.cmake
set(CMAKE_PROJECT_libpng_INCLUDE_BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/libpng-policy-fixup.cmake")
set(PNG_TOOLS OFF)
add_subdirectory(libpng)

add_subdirectory(jpeg)
cge_extern_defaults(jpeg)

# FreeType without optional dependencies, so it never picks up system libraries
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)
add_subdirectory(freetype)

add_subdirectory(imgui)
add_subdirectory(miniaudio)
# libogg finds its integer types with check_type_size, which reads the sizes
# from the test program; linked with emscripten that is a .js file without
# them (CMake before 4.x), so the checks only compile on the web
if (EMSCRIPTEN)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()
add_subdirectory(libogg)
unset(CMAKE_TRY_COMPILE_TARGET_TYPE)
add_subdirectory(libvorbis)

add_subdirectory(lua)
add_subdirectory(libxml)
if (CGE_WITH_PHYSICS)
  add_subdirectory(ode)
endif()
add_subdirectory(gif)

# googletest 1.17.0 for the unit tests (without googlemock and gtest's own
# tests, samples and docs)
if (CGE_BUILD_TESTS)
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  # the DLL runtime, like everything else (CMAKE_MSVC_RUNTIME_LIBRARY)
  set(gtest_force_shared_crt ON)
  add_subdirectory(googletest)
endif()
if (NOT OS STREQUAL "android")
  add_subdirectory(glad)
endif()
