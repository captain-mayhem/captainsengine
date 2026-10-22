# Dear ImGui core with FreeType font rendering (imconfig.h defines
# IMGUI_ENABLE_FREETYPE). The platform and renderer backends are the engine's
# own (Engine/ui/), so none of ImGui's backends are part of this copy.
#
# ImGui is compiled into the engine library and exported from it (there must
# be exactly one ImGui context), so this is an INTERFACE library whose sources
# become part of the library that links it: only the engine does, privately.
add_library(imgui INTERFACE)
target_sources(imgui INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_internal.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imstb_rectpack.h
	${CMAKE_CURRENT_SOURCE_DIR}/imstb_textedit.h
	${CMAKE_CURRENT_SOURCE_DIR}/imstb_truetype.h
	${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype/imgui_freetype.h
)
target_include_directories(imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui INTERFACE freetype)
