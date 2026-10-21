// One triangle that covers the whole target, without a vertex buffer: draw
// 3 vertices with an empty vertex layout. The fragment shaders read their
// targets' pixels with texelFetch at gl_FragCoord, so they work the same on
// every backend.

void main(){
  vec2 corner = vec2(float((gl_VertexID & 1) << 2), float((gl_VertexID & 2) << 1));
  gl_Position = vec4(corner - 1.0, 0.0, 1.0);
}
