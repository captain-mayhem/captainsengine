IF (OS STREQUAL windows)

SET(EFXUTIL_LIBRARY CACHE STRING "The EFX-Util library for OpenAL")

if (EFXUTIL_LIBRARY STREQUAL "")
  return()
endif()

ADD_EXECUTABLE(eax2efx
main.cpp
)

TARGET_LINK_LIBRARIES(eax2efx  ${EFXUTIL_LIBRARY})

ENDIF (OS STREQUAL windows)
