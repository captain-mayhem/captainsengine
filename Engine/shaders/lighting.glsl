// The engine's material and lights (std140) for lit shaders, and the
// lighting of the old renderers. Includes common.glsl.
// Gfx::MaterialUniforms and Gfx::LightUniforms (renderer/shaderlibrary.h)
// have the same layout.
#include "engine/common.glsl"

#define CGE_MAX_LIGHTS 8

// uniform buffer slot 2 (Gfx::SLOT_MATERIAL): set per draw
layout(std140) uniform CGE_Material{
  // rgb, a: the opacity
  vec4 cge_diffuse;
  vec4 cge_ambient;
  // rgb, w: the shininess (specular exponent)
  vec4 cge_specular;
  // x: 1 if the surface is textured with cge_texture
  ivec4 cge_materialFlags;
};

// uniform buffer slot 3 (Gfx::SLOT_LIGHTS): the lights in view space
layout(std140) uniform CGE_Lights{
  // x: the number of lights; without lights, surfaces are drawn unlit
  ivec4 cge_lightCount;
  // w = 0: a directional light shining from the direction xyz
  vec4 cge_lightPosition[CGE_MAX_LIGHTS];
  // xyz: the spot direction, w: the cutoff angle in radians
  vec4 cge_lightDirection[CGE_MAX_LIGHTS];
  vec4 cge_lightColor[CGE_MAX_LIGHTS];
  // x: the quadratic attenuation
  vec4 cge_lightAttenuation[CGE_MAX_LIGHTS];
};

// texture slot 0 (Gfx::SLOT_TEXTURE)
uniform sampler2D cge_texture;

// the material's diffuse colour and texture times cge_color
vec4 cge_surfaceColor(vec2 texcoord){
  vec4 color = cge_diffuse*cge_color;
  if (cge_materialFlags.x != 0)
    color *= texture(cge_texture, texcoord);
  return color;
}

// a light's direction from position (normalised) and its attenuation
// (0 outside a spot light's cone)
vec3 cge_lightVector(vec4 lightPos, vec4 spot, float attenuation, vec3 position, out float att){
  att = 1.0;
  if (lightPos.w == 0.0)
    return normalize(lightPos.xyz);
  vec3 lightvec = normalize(lightPos.xyz - position);
  float angle = acos(dot(-lightvec, normalize(spot.xyz)));
  if (angle > spot.w)
    att = 0.0;
  else{
    float dist = length(lightPos.xyz - position);
    att = 1.0/(1.0 + attenuation*dist*dist);
  }
  return lightvec;
}

// colour lit by the lights (Phong, per pixel); normal and position in view space
vec4 cge_shade(vec4 color, vec3 normal, vec3 position){
  int count = min(cge_lightCount.x, CGE_MAX_LIGHTS);
  if (count == 0)
    return color;
  vec3 eye = normalize(-position);
  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  vec3 ambient = vec3(0.0);
  for (int i = 0; i < count; ++i){
    float att;
    vec3 lightvec = cge_lightVector(cge_lightPosition[i], cge_lightDirection[i], cge_lightAttenuation[i].x, position, att);
    vec3 refl = normalize(reflect(-lightvec, normal));
    float NL = max(dot(normal, lightvec), 0.0);
    float spec = 0.0;
    if (NL > 0.0)
      spec = pow(max(dot(refl, eye), 0.0), cge_specular.w);
    diffuse += cge_lightColor[i].rgb*NL*att;
    specular += cge_lightColor[i].rgb*spec*att;
    ambient += cge_lightColor[i].rgb*cge_ambient.rgb;
  }
  return vec4(color.rgb*(ambient + diffuse) + specular*cge_specular.rgb, color.a);
}
