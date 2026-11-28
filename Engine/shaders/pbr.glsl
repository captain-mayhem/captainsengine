// glTF's metallic-roughness material, lit by the engine's lights in view
// space (lighting.glsl): the BRDF of glTF's specification (GGX, the
// height-correlated Smith visibility, Schlick's Fresnel), in linear
// colours. The engine's colours and textures are sRGB; cge_srgbToLinear and
// cge_linearToSrgb convert them. The light from everywhere (the material's
// ambient colour times the lights' colours) is diffuse and an approximation
// of the specular reflection of an even surrounding (Karis' analytic
// environment BRDF), dimmed by the occlusion texture. Includes lighting.glsl.
#include "engine/lighting.glsl"

#define CGE_PI 3.14159265358979

// texture slots 1 to 4 (Gfx::SLOT_METALLIC_ROUGHNESS ... SLOT_EMISSIVE); a
// bit in cge_materialFlags.y says that one is bound (Gfx::MaterialTextureBit)
uniform sampler2D cge_metallicRoughnessTexture;
uniform sampler2D cge_normalTexture;
uniform sampler2D cge_occlusionTexture;
uniform sampler2D cge_emissiveTexture;

vec3 cge_srgbToLinear(vec3 c){
  c = clamp(c, 0.0, 1.0);
  return mix(c/12.92, pow((c + 0.055)/1.055, vec3(2.4)), step(0.04045, c));
}

vec3 cge_linearToSrgb(vec3 c){
  c = clamp(c, 0.0, 1.0);
  return mix(c*12.92, 1.055*pow(c, vec3(1.0/2.4)) - 0.055, step(0.0031308, c));
}

// what the surface is at a point, in linear colours
struct CGE_PbrSurface{
  vec3 baseColor;
  float metallic;
  float roughness;
  // the light from everywhere that reaches the point (0..1)
  float occlusion;
  vec3 emissive;
  // view space, normalised, towards the viewer on the back of double-sided materials
  vec3 normal;
};

// the material's metalness, roughness, occlusion and emissive colour at texcoord
void cge_pbrMaterial(inout CGE_PbrSurface surface, vec2 texcoord){
  surface.metallic = cge_pbr.x;
  surface.roughness = cge_pbr.y;
  if ((cge_materialFlags.y & 1) != 0){
    vec4 mr = texture(cge_metallicRoughnessTexture, texcoord);
    surface.roughness *= mr.g;
    surface.metallic *= mr.b;
  }
  surface.metallic = clamp(surface.metallic, 0.0, 1.0);
  surface.roughness = clamp(surface.roughness, 0.0, 1.0);
  surface.occlusion = 1.0;
  if ((cge_materialFlags.y & 4) != 0)
    surface.occlusion = 1.0 + cge_pbr.w*(texture(cge_occlusionTexture, texcoord).r - 1.0);
  surface.emissive = cge_srgbToLinear(cge_emissive.rgb)*cge_emissive.w;
  if ((cge_materialFlags.y & 8) != 0)
    surface.emissive *= cge_srgbToLinear(texture(cge_emissiveTexture, texcoord).rgb);
}

// the surface normal in view space with the normal map, if one is bound
/*! The tangent frame comes from the derivatives of the position and the
    texture coordinates (glTF's v from the top of the image), so meshes need
    no tangents. Call it in fragment shaders, outside branches. */
vec3 cge_pbrNormal(vec3 normal, vec3 position, vec2 texcoord){
  vec3 ng = normalize(normal);
  vec2 uvdx = dFdx(texcoord);
  vec2 uvdy = dFdy(texcoord);
  vec3 pdx = dFdx(position);
  vec3 pdy = dFdy(position);
  bool back = cge_materialParams.y != 0.0 && !gl_FrontFacing;
  if ((cge_materialFlags.y & 2) == 0)
    return back ? -ng : ng;
  float det = uvdx.x*uvdy.y - uvdy.x*uvdx.y;
  if (det == 0.0)
    return back ? -ng : ng;
  // the directions in which u and v grow
  vec3 dpdu = (uvdy.y*pdx - uvdx.y*pdy)/det;
  vec3 dpdv = (uvdx.x*pdy - uvdy.x*pdx)/det;
  vec3 t = dpdu - ng*dot(ng, dpdu);
  if (dot(t, t) < 1e-20)
    return back ? -ng : ng;
  t = normalize(t);
  // glTF's +y is up in the image, against v; mirrored textures turn it round
  vec3 b = cross(ng, t);
  if (dot(b, dpdv) > 0.0)
    b = -b;
  if (back){
    t = -t;
    b = -b;
    ng = -ng;
  }
  vec3 n = texture(cge_normalTexture, texcoord).rgb*2.0 - 1.0;
  n.xy *= cge_pbr.z;
  return normalize(mat3(t, b, ng)*n);
}

// the specular reflection of an even surrounding (scale and bias of F0)
vec2 cge_pbrEnvironment(float roughness, float NdotV){
  const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
  const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
  vec4 r = roughness*c0 + c1;
  float a004 = min(r.x*r.x, exp2(-9.28*NdotV))*r.x + r.y;
  return vec2(-1.04, 1.04)*a004 + r.zw;
}

// the surface lit by the lights, in linear colours; position in view space
/*! Without lights, the base and the emissive colour (unlit). A light of
    colour c shining straight onto a white rough dielectric gives about c. */
vec3 cge_pbrShade(CGE_PbrSurface surface, vec3 position){
  int count = min(cge_lightCount.x, CGE_MAX_LIGHTS);
  if (count == 0)
    return surface.baseColor + surface.emissive;
  vec3 n = surface.normal;
  vec3 v = normalize(-position);
  float NdotV = clamp(abs(dot(n, v)), 0.0001, 1.0);
  vec3 f0 = mix(vec3(0.04), surface.baseColor, surface.metallic);
  vec3 diffuseColor = surface.baseColor*(1.0 - surface.metallic);
  float alpha = max(surface.roughness*surface.roughness, 0.002);
  float alpha2 = alpha*alpha;
  vec3 color = vec3(0.0);
  vec3 lightSum = vec3(0.0);
  for (int i = 0; i < count; ++i){
    float att;
    vec3 l = cge_lightVector(cge_lightPosition[i], cge_lightDirection[i], cge_lightAttenuation[i].x, position, att);
    vec3 lightColor = cge_srgbToLinear(cge_lightColor[i].rgb);
    lightSum += lightColor;
    float NdotL = dot(n, l);
    if (NdotL <= 0.0 || att <= 0.0)
      continue;
    vec3 h = normalize(l + v);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);
    vec3 fresnel = f0 + (1.0 - f0)*pow(1.0 - VdotH, 5.0);
    float d = NdotH*NdotH*(alpha2 - 1.0) + 1.0;
    float distribution = alpha2/(CGE_PI*d*d);
    float visibility = 0.5/(NdotL*sqrt(NdotV*NdotV*(1.0 - alpha2) + alpha2) +
      NdotV*sqrt(NdotL*NdotL*(1.0 - alpha2) + alpha2));
    // the lights' colours are the irradiance of a surface facing them, times pi
    vec3 brdf = (1.0 - fresnel)*diffuseColor + CGE_PI*fresnel*distribution*visibility;
    color += brdf*lightColor*NdotL*att;
  }
  vec2 env = cge_pbrEnvironment(surface.roughness, NdotV);
  vec3 ambient = cge_srgbToLinear(cge_ambient.rgb)*lightSum;
  color += ambient*(diffuseColor + max(f0*env.x + env.y, 0.0))*surface.occlusion;
  return color + surface.emissive;
}
