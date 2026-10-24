SET(PRE platform)

SET(Engine_platform_SRCS
	${PRE}/assets.cpp
	${PRE}/services.cpp
	${PRE}/settings.cpp
	${PRE}/systemfonts.cpp
	${PRE}/userdirs.cpp
)

SET(Engine_platform_HDRS
	${PRE}/assets.h
	${PRE}/services.h
	${PRE}/settings.h
	${PRE}/systemfonts.h
	${PRE}/userdirs.h
)

# internal to the engine, not installed
SET(Engine_platform_internal_HDRS
	${PRE}/platform_internal.h
)

INSTALL(FILES ${Engine_platform_HDRS} DESTINATION Engine/include/${PRE})
