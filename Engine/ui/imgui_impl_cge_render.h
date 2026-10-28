#ifndef CGE_IMGUI_IMPL_CGE_RENDER_H
#define CGE_IMGUI_IMPL_CGE_RENDER_H

#include <cge_export.h>

#include <cstdint>

#include <imgui.h>

namespace CGE{
namespace Gfx{
class RenderDevice;
class Sampler;
class Texture;
}
}

// The engine's Dear ImGui renderer backend: draws ImGui's draw data with the
// render device (the built-in ImGui shader), so it works on every render
// backend. It supports ImDrawCmd::VtxOffset and ImGui's texture requests
// (the font atlas grows as glyphs are needed).
//
// The engine sets it up for its window and draws ImGui on top of each frame
// (Engine::renderFrame), so applications only call ImGui's functions in their
// render(). The functions below are for drawing ImGui elsewhere (tests, or
// into a render target).

//! creates the shader, the pipeline and the sampler on device
CGE_API bool ImGui_ImplCGE_Render_Init(CGE::Gfx::RenderDevice& device);
//! deletes everything on the device, also ImGui's textures
CGE_API void ImGui_ImplCGE_Render_Shutdown();
//! before ImGui::NewFrame
CGE_API void ImGui_ImplCGE_Render_NewFrame();
//! draws the draw data inside the current pass, which goes into a target of
//! the draw data's size (DisplaySize*FramebufferScale pixels)
/*! Leaves the pass with the scissor rectangle on the whole target. */
CGE_API void ImGui_ImplCGE_Render_RenderDrawData(ImDrawData* drawData);

//! draws the following images of list with sampler (null: the default, linear
//! filtering and clamped to the edges)
/*! E.g. repeating for tiled images (UVs beyond 0..1) or nearest filtering for
    pixel art. Until the next call for the list, which ends with the list. */
CGE_API void ImGui_ImplCGE_SetSampler(ImDrawList* list, CGE::Gfx::Sampler* sampler);

//! a device texture as an ImGui texture id, for ImGui::Image and the like
inline ImTextureID ImGui_ImplCGE_TextureID(CGE::Gfx::Texture* texture){
  return (ImTextureID)(uintptr_t)texture;
}

#endif
