SET(PRE window)

SET(Engine_window_SRCS
	${PRE}/nativeLinux.cpp
	${PRE}/nativeWindows.cpp
	${PRE}/window.cpp)

INSTALL(FILES 	
	${PRE}/nativeLinux.h 
	${PRE}/nativeWindows.h 
	${PRE}/window.h
DESTINATION Engine/include/${PRE})