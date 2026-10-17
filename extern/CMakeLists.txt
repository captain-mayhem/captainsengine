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

add_subdirectory(jpeg)
add_subdirectory(lua)
add_subdirectory(libxml)
add_subdirectory(zlib)
if (CGE_WITH_PHYSICS)
  add_subdirectory(ode)
endif()
add_subdirectory(gif)
add_subdirectory(png)
if (NOT OS STREQUAL "android")
  add_subdirectory(glew)
endif()
