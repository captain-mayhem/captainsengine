# Dear ImGui core with FreeType font rendering (imconfig.h defines
# IMGUI_ENABLE_FREETYPE). The platform and renderer backends are the engine's
# own (Phase 1b), so none of ImGui's backends are part of this copy.
add_library(imgui STATIC
	imconfig.h
	imgui.cpp
	imgui.h
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_internal.h
	imgui_tables.cpp
	imgui_widgets.cpp
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
	misc/freetype/imgui_freetype.cpp
	misc/freetype/imgui_freetype.h
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui PRIVATE freetype)
