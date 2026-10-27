#define _USE_MATH_DEFINES
#include <math.h>
#include "../system/engine.h"
#include "forms.h"
#include "drawcontext.h"
#include "renderdevice.h"
#include "../math/matrixstack.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace CGE;

Forms::Forms(Gfx::RenderDevice& device) : mLineWidth(1) {
  createShapes(device);
}

Forms::~Forms(){
}

// --- through a render device ---

namespace{

struct FormVertex{
  float position[3];
  float normal[3];
  uint8_t color[4];
  float texcoord[2];
};

FormVertex formVertex(const Vec3f& pos, const Vec3f& normal, float u = 0, float v = 0){
  FormVertex vtx = {{pos.x, pos.y, pos.z}, {normal.x, normal.y, normal.z}, {255, 255, 255, 255}, {u, v}};
  return vtx;
}

std::unique_ptr<Forms::Shape> createShape(Gfx::RenderDevice& device, const std::vector<FormVertex>& vertices,
  const std::vector<uint16_t>& indices){
  using namespace Gfx;
  std::unique_ptr<Forms::Shape> shape = std::make_unique<Forms::Shape>();
  shape->vertices = device.createBuffer({BufferType::Vertex, Usage::Immutable, vertices.size()*sizeof(FormVertex)}, vertices.data());
  shape->indices = device.createBuffer({BufferType::Index, Usage::Immutable, indices.size()*sizeof(uint16_t)}, indices.data());
  if (!shape->vertices || !shape->indices)
    return nullptr;
  return shape;
}

}

Forms::Shape::Shape(){
}

Forms::Shape::~Shape(){
}

const Gfx::VertexLayout& Forms::getVertexLayout(){
  using namespace Gfx;
  static const VertexLayout layout = {
    {
      {ATTR_POSITION, VertexFormat::Float3, 0, 0},
      {ATTR_NORMAL, VertexFormat::Float3, 0, 12},
      {ATTR_COLOR, VertexFormat::UByte4Norm, 0, 24},
      {ATTR_TEXCOORD, VertexFormat::Float2, 0, 28},
    },
    {{(int)sizeof(FormVertex), false}},
  };
  return layout;
}

void Forms::createShapes(Gfx::RenderDevice& device){
  //v = 0 at the top edge
  std::vector<FormVertex> quad = {
    formVertex(Vec3f(-0.5f, -0.5f, 0), Vec3f(0, 0, 1), 0, 1),
    formVertex(Vec3f(0.5f, -0.5f, 0), Vec3f(0, 0, 1), 1, 1),
    formVertex(Vec3f(-0.5f, 0.5f, 0), Vec3f(0, 0, 1), 0, 0),
    formVertex(Vec3f(0.5f, 0.5f, 0), Vec3f(0, 0, 1), 1, 0),
  };
  mQuad = createShape(device, quad, {0, 1, 2, 3});
  if (mQuad)
    mQuad->parts.push_back({Gfx::PrimitiveType::TriangleStrip, 0, 4});
  mCylinder = createCylinder(device, 1, 1, 32);
}

void Forms::drawQuad(Gfx::DrawContext& ctx) const{
  if (!mQuad)
    return;
  ctx.draw(ctx.getTexture() ? Gfx::BuiltinShader::Textured : Gfx::BuiltinShader::Color, mQuad->vertices.get(),
    getVertexLayout(), Gfx::PrimitiveType::TriangleStrip, 4);
}

void Forms::drawQuad(Gfx::DrawContext& ctx, const Vec2f& position, const Vec2f& size, float rotation) const{
  MatrixStack& model = ctx.model();
  model.push();
  Vec2f offset = size/2.0f;
  if (rotation != 0){
    //the quad stays in the box from position to position+size
    Matrix rot = Matrix(Matrix::Rotation, Vec3f(0, 0, -1), rotation/180*(float)M_PI);
    Vec3f rotoff = rot * Vec3f(offset.x, offset.y, 0);
    offset = Vec2f(fabsf(rotoff.x), fabsf(rotoff.y));
  }
  model.translate(position.x+offset.x, position.y+offset.y, 0);
  if (rotation != 0)
    model.rotate(rotation, 0, 0, -1);
  model.scale(size.x, size.y, 1);
  drawQuad(ctx);
  model.pop();
}

void Forms::drawLine(Gfx::DrawContext& ctx, const Vec2f& from, const Vec2f& to) const{
  Vec2f dir = to - from;
  float length = dir.length();
  if (length == 0)
    return;
  MatrixStack& model = ctx.model();
  model.push();
  model.translate((from.x+to.x)/2, (from.y+to.y)/2, 0);
  //the quad's x axis along the line, its y axis across it; built from the
  //direction instead of an angle, so axis-aligned lines get exact corners
  //(their edges often fall on pixel centres)
  Vec2f across = Vec2f(-dir.y, dir.x)*(mLineWidth/length);
  model.multiply(Matrix(Vec3f(dir.x, across.x, 0), Vec3f(dir.y, across.y, 0), Vec3f(0, 0, 1), Vec3f()));
  drawQuad(ctx);
  model.pop();
}

void Forms::drawCylinder(Gfx::DrawContext& ctx) const{
  if (mCylinder)
    drawShape(ctx, *mCylinder);
}

std::unique_ptr<Forms::Shape> Forms::createCylinder(Gfx::RenderDevice& device, float radius, float height, int segments){
  //the hull, the bottom and top rings with the caps' normals, the caps' centres
  std::vector<FormVertex> vertices(4*segments+2);
  float angle = 0;
  for (int i = 0; i < 2*segments; i+=2){
    Vec3f bottom(radius*cosf(angle), -height/2, radius*sinf(angle));
    Vec3f top(radius*cosf(angle), height/2, radius*sinf(angle));
    Vec3f side(cosf(angle), 0, sinf(angle));
    vertices[i] = formVertex(bottom, side);
    vertices[i+1] = formVertex(top, side);
    vertices[2*segments+i] = formVertex(bottom, Vec3f(0, -1, 0));
    vertices[2*segments+i+1] = formVertex(top, Vec3f(0, 1, 0));
    angle += (float)(M_PI*2./segments);
  }
  vertices[4*segments] = formVertex(Vec3f(0, -height/2, 0), Vec3f(0, -1, 0));
  vertices[4*segments+1] = formVertex(Vec3f(0, height/2, 0), Vec3f(0, 1, 0));

  //three triangle strips: the hull, the bottom and the top, as in the old renderers
  std::vector<uint16_t> indices;
  for (int i = 0; i < (segments+1)*2; ++i)
    indices.push_back((uint16_t)(i%(2*segments)));
  int bottomStart = (int)indices.size();
  indices.push_back((uint16_t)(4*segments-2));
  for (int i = 0, ring = 2*segments; i < segments; ++i, ring+=2){
    indices.push_back((uint16_t)ring);
    indices.push_back((uint16_t)(4*segments));
  }
  int topStart = (int)indices.size();
  indices.push_back((uint16_t)(2*segments+1));
  for (int i = 0, ring = 4*segments-1; i < segments; ++i, ring-=2){
    indices.push_back((uint16_t)ring);
    indices.push_back((uint16_t)(4*segments+1));
  }
  std::unique_ptr<Shape> shape = createShape(device, vertices, indices);
  if (!shape)
    return nullptr;
  shape->parts.push_back({Gfx::PrimitiveType::TriangleStrip, 0, bottomStart});
  shape->parts.push_back({Gfx::PrimitiveType::TriangleStrip, bottomStart, topStart-bottomStart});
  shape->parts.push_back({Gfx::PrimitiveType::TriangleStrip, topStart, (int)indices.size()-topStart});
  return shape;
}

std::unique_ptr<Forms::Shape> Forms::createBox(Gfx::RenderDevice& device, float width, float height, float depth){
  //each corner three times, once per face it belongs to
  const Vec3f corners[8] = {
    Vec3f(-width/2, height/2, depth/2), Vec3f(-width/2, height/2, -depth/2),
    Vec3f(width/2, height/2, -depth/2), Vec3f(width/2, height/2, depth/2),
    Vec3f(-width/2, -height/2, depth/2), Vec3f(-width/2, -height/2, -depth/2),
    Vec3f(width/2, -height/2, -depth/2), Vec3f(width/2, -height/2, depth/2),
  };
  //the normals of the corners' x faces, y faces and z faces
  std::vector<FormVertex> vertices(24);
  for (int i = 0; i < 8; ++i){
    vertices[i] = formVertex(corners[i], Vec3f(corners[i].x < 0 ? -1.0f : 1.0f, 0, 0));
    vertices[8+i] = formVertex(corners[i], Vec3f(0, corners[i].y < 0 ? -1.0f : 1.0f, 0));
    vertices[16+i] = formVertex(corners[i], Vec3f(0, 0, corners[i].z < 0 ? -1.0f : 1.0f));
  }
  std::vector<uint16_t> indices = {
    8, 11, 9, 9, 11, 10, //top
    20, 23, 16, 16, 23, 19, //front
    5, 4, 1, 1, 4, 0, //left
    22, 21, 18, 18, 21, 17, //back
    7, 6, 3, 3, 6, 2, //right
    13, 14, 12, 12, 14, 15, //bottom
  };
  std::unique_ptr<Shape> shape = createShape(device, vertices, indices);
  if (!shape)
    return nullptr;
  shape->parts.push_back({Gfx::PrimitiveType::Triangles, 0, (int)indices.size()});
  return shape;
}

void Forms::drawShape(Gfx::DrawContext& ctx, const Shape& shape){
  for (const Shape::Part& part : shape.parts){
    ctx.drawIndexed(Gfx::BuiltinShader::Lit, shape.vertices.get(), getVertexLayout(), shape.indices.get(),
      Gfx::IndexType::UInt16, part.primitive, part.indexCount, part.firstIndex);
  }
}
