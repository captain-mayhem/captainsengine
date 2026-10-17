FILE(GLOB alut_SRCS src/*.c scr/*.h)

ADD_LIBRARY(alut
${alut_SRCS}
)
target_include_directories(alut PUBLIC include)
target_link_libraries(alut PUBLIC ${OPENAL_LIBRARY})
target_include_directories(alut PUBLIC ${OPENAL_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR}/..)
# this crazy define prevents us from getting declspec dllexport/import defined
target_compile_definitions(alut PUBLIC _XBOX)
target_compile_definitions(alut PRIVATE ALUT_BUILD_LIBRARY)
IF(OS STREQUAL "windows")
target_compile_definitions(alut PRIVATE HAVE_SLEEP HAVE_WINDOWS_H HAVE__STAT _CRT_SECURE_NO_DEPRECATE)
ENDIF(OS STREQUAL "windows")
IF(OS STREQUAL "linux" OR OS STREQUAL "android")
target_compile_definitions(alut PRIVATE HAVE_NANOSLEEP HAVE_TIME_H HAVE_STDINT_H HAVE_STAT HAVE_UNISTD_H)
ENDIF()
