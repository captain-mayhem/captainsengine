SET(PRE window)

SET(Engine_window_SRCS
	${PRE}/nativeLinux.cpp
	${PRE}/nativeWindows.cpp
	${PRE}/window.cpp)

SET(Engine_window_HDRS
	${PRE}/nativeLinux.h 
	${PRE}/nativeWindows.h 
	${PRE}/window.h
)

INSTALL(FILES ${Engine_window_HDRS} DESTINATION Engine/include/${PRE})