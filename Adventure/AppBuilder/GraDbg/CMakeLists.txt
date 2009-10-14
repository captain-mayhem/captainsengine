#ADD_DEFINITIONS(-DUSE_GLES)

ADD_LIBRARY(GraDbg SHARED
  ogl.cpp
	wgl.cpp
#	egl.cpp
  util.h
  util.cpp
)
TARGET_LINK_LIBRARIES(GraDbg GraDbgRuntime)

ADD_CUSTOM_COMMAND(TARGET GraDbg POST_BUILD
COMMAND copy debug\\GraDbg.dll ..\\AppBuilder\\debug\\OpenGL32.dll /Y
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_LIBRARY(GraDbgRuntime SHARED
  wgl_run.cpp
  wgl_run.h
#	egl_run.cpp
#	egl_run.h
  util_run.h
  util_run.cpp
)

#TARGET_LINK_LIBRARIES(GraDbgRuntime OpenGL32)