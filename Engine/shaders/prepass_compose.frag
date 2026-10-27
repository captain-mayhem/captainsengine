// Light prepass, step 3: the surfaces lit by the light buffer (with
// fullscreen.vert), writing the G-buffer's depth, so transparent surfaces
// drawn afterwards are hidden behind opaque ones.

// texture slots 0 to 3
uniform sampler2D cge_lightBuffer;
uniform sampler2D cge_gbufferDiffuse;
uniform sampler2D cge_gbufferSpecular;
uniform sampler2D cge_gbufferDepth;

out vec4 o_color;

void main(){
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  vec4 diffuse = texelFetch(cge_gbufferDiffuse, pixel, 0);
  if (diffuse.a == 0.0)
    discard;
  vec4 specular = texelFetch(cge_gbufferSpecular, pixel, 0);
  vec4 light = texelFetch(cge_lightBuffer, pixel, 0);
  // the specular light takes the diffuse light's hue
  float luminance = 0.299*light.r + 0.587*light.g + 0.114*light.b;
  vec3 spec = light.rgb/max(luminance, 0.0001)*light.a;
  o_color = vec4(diffuse.rgb*light.rgb + specular.rgb*spec, 1.0);
  gl_FragDepth = texelFetch(cge_gbufferDepth, pixel, 0).r;
}
