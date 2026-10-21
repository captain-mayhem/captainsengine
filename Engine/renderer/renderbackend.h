#ifndef CGE_RENDER_BACKEND_H
#define CGE_RENDER_BACKEND_H

// The entry points of a render backend plugin (internal to the engine and its
// backends). A plugin library exports both functions with C linkage;
// RenderDevice::create loads the library from the engine library's folder and
// only uses it if its version matches.

namespace CGE{
namespace Gfx{
class RenderDevice;
}
}

//! changes whenever RenderDevice or the resource classes change their layout
#define CGE_RENDER_BACKEND_VERSION 2

#define CGE_RENDER_BACKEND_VERSION_FUNC "cgeRenderBackendVersion"
#define CGE_RENDER_BACKEND_CREATE_FUNC "cgeCreateRenderDevice"

typedef int (*CgeRenderBackendVersionFunc)();
typedef CGE::Gfx::RenderDevice* (*CgeCreateRenderDeviceFunc)();

#endif
