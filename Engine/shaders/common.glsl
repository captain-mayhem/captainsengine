// The engine's uniform blocks (std140), shared by the built-in shaders.
// Gfx::FrameUniforms and Gfx::DrawUniforms (renderer/shaderlibrary.h) have
// the same layout.

// uniform buffer slot 0 (Gfx::SLOT_FRAME): set once per frame or pass
layout(std140) uniform CGE_Frame{
  mat4 cge_projection;
  mat4 cge_view;
  // x: seconds since the start
  vec4 cge_time;
};

// uniform buffer slot 1 (Gfx::SLOT_DRAW): set per draw
layout(std140) uniform CGE_Draw{
  mat4 cge_model;
  // multiplied with the vertex colour
  vec4 cge_color;
};
