// Light prepass, step 2: one light into the light buffer (blended additively,
// with fullscreen.vert): rgb the diffuse light, alpha the specular factor.
#include "engine/lighting.glsl"

// uniform buffer slot 4 (LightPrepassRenderer): set per light
layout(std140) uniform CGE_PrepassLight{
  // from clip space back to view space
  mat4 cge_inverseProjection;
  // x: 1 if clip-space depth runs from 0 to 1 instead of -1 to 1
  vec4 cge_clipDepth;
  // one light like in CGE_Lights
  vec4 cge_prepassLightPosition;
  vec4 cge_prepassLightDirection;
  vec4 cge_prepassLightColor;
  vec4 cge_prepassLightAttenuation;
};

// texture slots 0 and 1
uniform sampler2D cge_gbufferNormal;
uniform sampler2D cge_gbufferDepth;

out vec4 o_light;

void main(){
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  vec2 size = vec2(textureSize(cge_gbufferDepth, 0));
  float depth = texelFetch(cge_gbufferDepth, pixel, 0).r;
  vec4 encoded = texelFetch(cge_gbufferNormal, pixel, 0);
  vec3 normal = normalize(encoded.xyz*2.0 - 1.0);
  float shininess = encoded.a*255.0;

  vec3 ndc = vec3(gl_FragCoord.xy/size*2.0 - 1.0, cge_clipDepth.x != 0.0 ? depth : depth*2.0 - 1.0);
  vec4 view = cge_inverseProjection*vec4(ndc, 1.0);
  vec3 position = view.xyz/view.w;

  float att;
  vec3 lightvec = cge_lightVector(cge_prepassLightPosition, cge_prepassLightDirection, cge_prepassLightAttenuation.x, position, att);
  vec3 refl = normalize(reflect(-lightvec, normal));
  float NL = max(dot(normal, lightvec), 0.0);
  float spec = 0.0;
  if (NL > 0.0)
    spec = pow(max(dot(refl, normalize(-position)), 0.0), shininess);
  o_light = vec4(cge_prepassLightColor.rgb*NL*att, spec*att);
}
