# videoconv: converts the videos added to a game into Ogg Theora with Vorbis
# (see main.cpp), with Windows' decoders (Media Foundation, Video for Windows).
# Built on its own as a 32-bit program (see ../CMakeLists.txt): the classic
# AVI codecs (Cinepak, or an installed Indeo) exist only as 32-bit DLLs.
cmake_minimum_required(VERSION 3.21)
project(videoconv C CXX)

if (PROJECT_IS_TOP_LEVEL)
  # on its own: the static C runtime, so the program needs no redistributable,
  # and the libraries from the engine's extern/
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set(CGE_EXTERN ${CMAKE_CURRENT_SOURCE_DIR}/../../../extern)
  function(cge_extern_defaults target)
    if (MSVC)
      target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    endif()
  endfunction()
  set(BUILD_SHARED_LIBS OFF)
  set(INSTALL_CMAKE_PACKAGE_MODULE OFF)
  add_subdirectory(${CGE_EXTERN}/libogg libogg EXCLUDE_FROM_ALL)
  add_subdirectory(${CGE_EXTERN}/libvorbis libvorbis EXCLUDE_FROM_ALL)
  add_subdirectory(${CGE_EXTERN}/libtheora libtheora EXCLUDE_FROM_ALL)
  add_subdirectory(${CGE_EXTERN}/pl_mpeg pl_mpeg EXCLUDE_FROM_ALL)
endif()

add_executable(videoconv main.cpp)
target_compile_features(videoconv PRIVATE cxx_std_17)
target_link_libraries(videoconv PRIVATE theoraenc vorbisenc pl_mpeg mfplat mfreadwrite mfuuid vfw32 msacm32 ole32)
target_compile_definitions(videoconv PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
if (PROJECT_IS_TOP_LEVEL)
  install(TARGETS videoconv RUNTIME DESTINATION .)
endif()
