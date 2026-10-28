#include "imgui_impl_cge_render.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>

#include "../io/Tracing.h"
#include "../renderer/renderdevice.h"
#include "../renderer/shaderlibrary.h"

using namespace CGE;
using namespace CGE::Gfx;

TR_CHANNEL(CGE_ImGui);

namespace{

//! the backend's state in io.BackendRendererUserData
struct BackendData{
  RenderDevice* device = nullptr;
  std::unique_ptr<Shader> shader;
  std::unique_ptr<Pipeline> pipeline;
  std::unique_ptr<Sampler> sampler;
  //! the sampler a draw list chose (ImGui_ImplCGE_SetSampler), null for the default
  Sampler* listSampler = nullptr;
  //! the projection (FrameUniforms)
  std::unique_ptr<Buffer> frame;
  //! all vertices and indices of a frame's draw data
  std::unique_ptr<Buffer> vertices;
  std::unique_ptr<Buffer> indices;
  //! what is uploaded into them, also tightly packed texture updates
  std::vector<unsigned char> staging;
  //! the device can't create the backend's resources: draw nothing
  bool failed = false;
};

BackendData* getData(){
  return ImGui::GetCurrentContext() ? (BackendData*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

VertexLayout imguiLayout(){
  VertexLayout layout;
  layout.attributes = {
    {ATTR_POSITION, VertexFormat::Float2, 0, (int)offsetof(ImDrawVert, pos)},
    {ATTR_TEXCOORD, VertexFormat::Float2, 0, (int)offsetof(ImDrawVert, uv)},
    {ATTR_COLOR, VertexFormat::UByte4Norm, 0, (int)offsetof(ImDrawVert, col)},
  };
  layout.buffers = {{(int)sizeof(ImDrawVert), false}};
  return layout;
}

//! a vertex or index buffer of at least size bytes, rewritten every frame
void reserve(BackendData& bd, std::unique_ptr<Buffer>& buffer, BufferType type, size_t size){
  if (buffer && buffer->getDesc().size >= size)
    return;
  BufferDesc desc;
  desc.type = type;
  desc.usage = Usage::Stream;
  //some room for growing, so it isn't recreated every few frames
  desc.size = size + size/2 + 4096;
  buffer = bd.device->createBuffer(desc);
}

void destroyTexture(ImTextureData* tex){
  std::unique_ptr<Texture> texture((Texture*)tex->BackendUserData);
  tex->BackendUserData = nullptr;
  tex->SetTexID(ImTextureID_Invalid);
  tex->SetStatus(ImTextureStatus_Destroyed);
}

//! handles ImGui's request to create, update or destroy one of its textures
void updateTexture(BackendData& bd, ImTextureData* tex){
  TR_USE(CGE_ImGui);
  if (tex->Status == ImTextureStatus_WantCreate){
    IM_ASSERT(tex->Format == ImTextureFormat_RGBA32);
    TextureDesc desc;
    desc.format = Format::RGBA8;
    desc.width = tex->Width;
    desc.height = tex->Height;
    //row 0 of ImGui's pixels is at v = 0, as in the texture
    std::unique_ptr<Texture> texture = bd.device->createTexture(desc, tex->GetPixels());
    if (!texture){
      TR_ERROR("ImGui's %dx%d texture can't be created", tex->Width, tex->Height);
      bd.failed = true;
      return;
    }
    tex->SetTexID(ImGui_ImplCGE_TextureID(texture.get()));
    tex->BackendUserData = texture.release();
    tex->SetStatus(ImTextureStatus_OK);
  }
  else if (tex->Status == ImTextureStatus_WantUpdates){
    Texture* texture = (Texture*)tex->BackendUserData;
    for (const ImTextureRect& rect : tex->Updates){
      //the device takes tightly packed rows
      const size_t pitch = (size_t)rect.w*tex->BytesPerPixel;
      bd.staging.resize(pitch*rect.h);
      for (int y = 0; y < rect.h; y++)
        memcpy(&bd.staging[y*pitch], tex->GetPixelsAt(rect.x, rect.y + y), pitch);
      TextureRegion region;
      region.x = rect.x;
      region.y = rect.y;
      region.width = rect.w;
      region.height = rect.h;
      bd.device->updateTexture(texture, region, bd.staging.data());
    }
    tex->SetStatus(ImTextureStatus_OK);
  }
  else if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0){
    destroyTexture(tex);
  }
}

//! the state every draw starts from (and ImDrawCallback_ResetRenderState returns to)
void setupRenderState(BackendData& bd, int width, int height){
  RenderDevice& device = *bd.device;
  device.setPipeline(bd.pipeline.get());
  device.setVertexBuffer(0, bd.vertices.get());
  device.setIndexBuffer(bd.indices.get(), sizeof(ImDrawIdx) == 2 ? IndexType::UInt16 : IndexType::UInt32);
  device.setUniformBuffer(SLOT_FRAME, bd.frame.get());
  Viewport viewport;
  viewport.width = (float)width;
  viewport.height = (float)height;
  device.setViewport(viewport);
  bd.listSampler = nullptr;
}

void setListSampler(const ImDrawList*, const ImDrawCmd* cmd){
  BackendData* bd = getData();
  if (bd)
    bd->listSampler = (Sampler*)cmd->UserCallbackData;
}

}

void ImGui_ImplCGE_SetSampler(ImDrawList* list, CGE::Gfx::Sampler* sampler){
  list->AddCallback(setListSampler, sampler);
}

bool ImGui_ImplCGE_Render_Init(RenderDevice& device){
  TR_USE(CGE_ImGui);
  ImGuiIO& io = ImGui::GetIO();
  IM_ASSERT(io.BackendRendererUserData == nullptr && "the renderer backend is already initialised");
  BackendData* bd = new BackendData();
  bd->device = &device;
  io.BackendRendererUserData = bd;
  io.BackendRendererName = "imgui_impl_cge_render";
  //VtxOffset: each draw sets the vertex buffer at its offset
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
  ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
  platformIO.Renderer_TextureMaxWidth = platformIO.Renderer_TextureMaxHeight = device.getCaps().maxTextureSize;

  bd->shader = ShaderLibrary::createBuiltin(device, BuiltinShader::ImGui);
  if (bd->shader){
    PipelineDesc desc;
    desc.shader = bd->shader.get();
    desc.vertexLayout = imguiLayout();
    desc.blend.enabled = true;
    desc.blend.srcColor = BlendFactor::SrcAlpha;
    desc.blend.dstColor = BlendFactor::OneMinusSrcAlpha;
    desc.blend.srcAlpha = BlendFactor::One;
    desc.blend.dstAlpha = BlendFactor::OneMinusSrcAlpha;
    desc.name = "ImGui";
    bd->pipeline = device.createPipeline(desc);
  }
  SamplerDesc sampler;
  sampler.addressU = sampler.addressV = AddressMode::ClampToEdge;
  bd->sampler = device.createSampler(sampler);
  BufferDesc frame;
  frame.type = BufferType::Uniform;
  frame.usage = Usage::Dynamic;
  frame.size = sizeof(FrameUniforms);
  bd->frame = device.createBuffer(frame);
  if (!bd->pipeline || !bd->sampler || !bd->frame){
    TR_ERROR("The ImGui renderer can't create its resources on the %s device", device.getName());
    bd->failed = true;
    return false;
  }
  return true;
}

void ImGui_ImplCGE_Render_Shutdown(){
  BackendData* bd = getData();
  if (!bd)
    return;
  for (ImTextureData* tex : ImGui::GetPlatformIO().Textures){
    if (tex->RefCount == 1 && tex->BackendUserData)
      destroyTexture(tex);
  }
  ImGuiIO& io = ImGui::GetIO();
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
  ImGui::GetPlatformIO().ClearRendererHandlers();
  delete bd;
}

void ImGui_ImplCGE_Render_NewFrame(){
  IM_ASSERT(getData() != nullptr && "ImGui_ImplCGE_Render_Init wasn't called");
}

void ImGui_ImplCGE_Render_RenderDrawData(ImDrawData* drawData){
  TR_USE(CGE_ImGui);
  BackendData* bd = getData();
  if (!bd || !drawData || bd->failed)
    return;
  const int width = (int)(drawData->DisplaySize.x*drawData->FramebufferScale.x);
  const int height = (int)(drawData->DisplaySize.y*drawData->FramebufferScale.y);
  if (width <= 0 || height <= 0)
    return;
  RenderDevice& device = *bd->device;

  //the textures first, also when nothing is drawn
  if (drawData->Textures){
    for (ImTextureData* tex : *drawData->Textures){
      if (tex->Status != ImTextureStatus_OK)
        updateTexture(*bd, tex);
    }
  }
  if (bd->failed || drawData->TotalVtxCount == 0)
    return;
  if (!device.isInPass()){
    TR_ERROR("ImGui_ImplCGE_Render_RenderDrawData needs a pass");
    return;
  }

  //all lists into one vertex and one index buffer, as whole-buffer updates
  //(the device can give the buffer new storage instead of waiting for the GPU)
  reserve(*bd, bd->vertices, BufferType::Vertex, drawData->TotalVtxCount*sizeof(ImDrawVert));
  reserve(*bd, bd->indices, BufferType::Index, drawData->TotalIdxCount*sizeof(ImDrawIdx));
  if (!bd->vertices || !bd->indices)
    return;
  bd->staging.resize(bd->vertices->getDesc().size);
  size_t offset = 0;
  for (const ImDrawList* list : drawData->CmdLists){
    memcpy(&bd->staging[offset], list->VtxBuffer.Data, list->VtxBuffer.Size*sizeof(ImDrawVert));
    offset += list->VtxBuffer.Size*sizeof(ImDrawVert);
  }
  device.updateBuffer(bd->vertices.get(), 0, bd->staging.size(), bd->staging.data());
  bd->staging.resize(bd->indices->getDesc().size);
  offset = 0;
  for (const ImDrawList* list : drawData->CmdLists){
    memcpy(&bd->staging[offset], list->IdxBuffer.Data, list->IdxBuffer.Size*sizeof(ImDrawIdx));
    offset += list->IdxBuffer.Size*sizeof(ImDrawIdx);
  }
  device.updateBuffer(bd->indices.get(), 0, bd->staging.size(), bd->staging.data());

  //ImGui's display (y down) onto the whole target
  FrameUniforms frame;
  const float left = drawData->DisplayPos.x;
  const float top = drawData->DisplayPos.y;
  frame.projection = device.ortho(left, left + drawData->DisplaySize.x, top + drawData->DisplaySize.y, top, -1, 1);
  device.updateBuffer(bd->frame.get(), 0, sizeof(frame), &frame);

  setupRenderState(*bd, width, height);
  const ImVec2 clipOffset = drawData->DisplayPos;
  const ImVec2 clipScale = drawData->FramebufferScale;
  int vertexBase = 0;
  int indexBase = 0;
  for (const ImDrawList* list : drawData->CmdLists){
    //a sampler chosen in one list doesn't go on into the next
    bd->listSampler = nullptr;
    for (const ImDrawCmd& cmd : list->CmdBuffer){
      if (cmd.UserCallback){
        if (cmd.UserCallback == ImDrawCallback_ResetRenderState)
          setupRenderState(*bd, width, height);
        else
          cmd.UserCallback(list, &cmd);
        continue;
      }
      //the clip rectangle in pixels, clamped to the target
      float x0 = (cmd.ClipRect.x - clipOffset.x)*clipScale.x;
      float y0 = (cmd.ClipRect.y - clipOffset.y)*clipScale.y;
      float x1 = (cmd.ClipRect.z - clipOffset.x)*clipScale.x;
      float y1 = (cmd.ClipRect.w - clipOffset.y)*clipScale.y;
      if (x0 < 0) x0 = 0;
      if (y0 < 0) y0 = 0;
      if (x1 > width) x1 = (float)width;
      if (y1 > height) y1 = (float)height;
      if (x1 <= x0 || y1 <= y0)
        continue;
      //Rect counts from the bottom
      Rect scissor;
      scissor.x = (int)x0;
      scissor.y = (int)(height - y1);
      scissor.width = (int)(x1 - x0);
      scissor.height = (int)(y1 - y0);
      device.setScissor(scissor);
      device.setTexture(SLOT_TEXTURE, (Texture*)(uintptr_t)cmd.GetTexID(), bd->listSampler ? bd->listSampler : bd->sampler.get());
      device.setVertexBuffer(0, bd->vertices.get(), (vertexBase + cmd.VtxOffset)*sizeof(ImDrawVert));
      device.drawIndexed((int)cmd.ElemCount, 1, indexBase + (int)cmd.IdxOffset);
    }
    vertexBase += list->VtxBuffer.Size;
    indexBase += list->IdxBuffer.Size;
  }
  Rect whole;
  whole.width = width;
  whole.height = height;
  device.setScissor(whole);
}
