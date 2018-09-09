SET(PRE window)

SET(Engine_window_SRCS
	${PRE}/nativeLinux.cpp
	${PRE}/nativeWindows.cpp
	${PRE}/window.cpp
	${PRE}/nativeQNX.cpp
	${PRE}/splashwindow.cpp
	${PRE}/nativeAndroid.cpp
)

SET(Engine_window_HDRS
	${PRE}/nativeLinux.h 
	${PRE}/nativeWindows.h 
	${PRE}/window.h
	${PRE}/nativeQNX.h
	${PRE}/splashwindow.h
	${PRE}/nativeAndroid.h
)

INSTALL(FILES ${Engine_window_HDRS} DESTINATION Engine/include/${PRE})