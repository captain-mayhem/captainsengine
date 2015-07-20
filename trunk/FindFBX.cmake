set(FBX_SEARCH_PATH
  "$ENV{ProgramW6432}/Autodesk/FBX/FBX SDK/2015.1"
  "$ENV{PROGRAMFILES}/Autodesk/FBX/FBX SDK/2015.1"
)

set(FBX_LIBNAME "libfbxsdk-mt")
set(FBX_LIBNAME_DEBUG "libfbxsdk-md")

find_path(FBX_INCLUDE_DIR "fbxsdk.h"
  PATHS ${FBX_SEARCH_PATH}
  PATH_SUFFIXES "include"
)

find_library(FBX_LIBRARY ${FBX_LIBNAME}
  PATHS ${FBX_SEARCH_PATH}
  PATH_SUFFIXES lib/vs2013/${CPU}/release
)

find_library(FBX_LIBRARY_DEBUG ${FBX_LIBNAME_DEBUG}
  PATHS ${FBX_SEARCH_PATH}
  PATH_SUFFIXES lib/vs2013/${CPU}/debug
)

if(FBX_LIBRARY AND FBX_LIBRARY_DEBUG AND FBX_INCLUDE_DIR)
  message(STATUS "Found FBX SDK")
  add_library(fbxsdk STATIC IMPORTED)
  set_target_properties(fbxsdk PROPERTIES IMPORTED_LOCATION_RELEASE ${FBX_LIBRARY})
  set_target_properties(fbxsdk PROPERTIES IMPORTED_LOCATION_DEBUG ${FBX_LIBRARY_DEBUG})
  set(FBX_FOUND 1)
else()
  set(FBX_FOUND 0)
endif()