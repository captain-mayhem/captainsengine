SET(PRE renderpass)

SET(Engine_renderpass_SRCS
	${PRE}/Forward.cpp
	${PRE}/LightPrepass.cpp
	${PRE}/Technique.cpp
)

SET(Engine_renderpass_HDRS
	${PRE}/Forward.h
	${PRE}/LightPrepass.h
	${PRE}/Technique.h
)

INSTALL(FILES ${Engine_renderpass_HDRS} DESTINATION Engine/include/${PRE})