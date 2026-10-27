SET(PRE ui)

SET(Engine_ui_SRCS
	${PRE}/console.cpp
	${PRE}/imgui_impl_cge_platform.cpp
	${PRE}/imgui_impl_cge_render.cpp
)

SET(Engine_ui_HDRS
	${PRE}/console.h
	${PRE}/imgui_config.h
	${PRE}/imgui_impl_cge_platform.h
	${PRE}/imgui_impl_cge_render.h
)

INSTALL(FILES ${Engine_ui_HDRS} DESTINATION Engine/include/${PRE})
INSTALL(FILES
	${CaptainsEngine_SOURCE_DIR}/extern/imgui/imgui.h
	${CaptainsEngine_SOURCE_DIR}/extern/imgui/imconfig.h
	DESTINATION Engine/include
)
