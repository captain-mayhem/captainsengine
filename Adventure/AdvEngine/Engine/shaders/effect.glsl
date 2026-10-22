// The uniforms of the post-processing effects (PostProcessing.cpp;
// EffectUniforms has the same layout). The effects draw a quad from 0 to 1
// over the whole target, without the engine's matrices.

// uniform buffer slot 4 (Graphics::SLOT_ADVENTURE)
layout(std140) uniform ADV_Effect{
  // the part of the input texture the image covers (image size / texture size)
  vec2 tex_scale;
  // the size of a pixel in texture coordinates
  vec2 pixel_offset;
  // fog and flash: the colour mixed in
  vec4 blendcol;
  // pixelate: the camera's position within a block, in pixels
  vec2 cam_offset;
  // the strength or opacity of the effect
  float strength;
};
