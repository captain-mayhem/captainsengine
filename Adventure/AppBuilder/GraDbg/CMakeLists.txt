ADD_LIBRARY(GraDbg SHARED
  ogl.cpp
  wgl.cpp
  util.h
  util.cpp
)
TARGET_LINK_LIBRARIES(GraDbg GraDbgRuntime)

ADD_LIBRARY(GraDbgRuntime SHARED
  wgl_run.cpp
  wgl_run.h
  util_run.h
  util_run.cpp
)

#TARGET_LINK_LIBRARIES(GraDbgRuntime OpenGL32)