#define _USE_MATH_DEFINES
#include <math.h>
#include "../system/engine.h"
#include "renderer.h"
#include "forms.h"
#include "drawcontext.h"
#include "renderdevice.h"
#include "../math/matrixstack.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace CGE;

Forms::Forms() : mLineWidth(1) {
  quad_ = NULL;
  cylinder_ = NULL;
  lines_ = NULL;
  for (int i = 0; i < 3; ++i){
    mCylinderInds[i] = NULL;
  }
  constructVBOs();
  Gfx::RenderDevice* device = CGE::Engine::instance()->getRenderDevice();
  if (device)
    createShapes(*device);
}

Forms::Forms(Gfx::RenderDevice& device) : mLineWidth(1) {
  quad_ = NULL;
  cylinder_ = NULL;
  lines_ = NULL;
  for (int i = 0; i < 3; ++i){
    mCylinderInds[i] = NULL;
  }
  createShapes(device);
}

Forms::~Forms(){
  SAFE_DELETE(quad_);
  SAFE_DELETE(cylinder_);
  for (int i = 0; i < 3; ++i){
    SAFE_DELETE(mCylinderInds[i]);
  }
  SAFE_DELETE(lines_);
}

void Forms::constructVBOs(){
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  //generate textured quad
  quad_ = rend->createVertexBuffer();
  if (quad_ == NULL)
    return;
  quad_->create(VB_POSITION | VB_TEXCOORD | VB_NORMAL, 4);
  quad_->lockVertexPointer();
  quad_->setPosition(3, Vector3D(0.5,0.5,0));
  quad_->setPosition(2, Vector3D(-0.5,0.5,0));
  quad_->setPosition(0, Vector3D(-0.5,-0.5,0));
  quad_->setPosition(1, Vector3D(0.5,-0.5,0));
  quad_->setTexCoord(3, Vec2f(1,0));
  quad_->setTexCoord(2, Vec2f(0,0));
  quad_->setTexCoord(0, Vec2f(0,1));
  quad_->setTexCoord(1, Vec2f(1,1));
  quad_->setNormal(3, Vec3f(0, 0, 1));
  quad_->setNormal(2, Vec3f(0, 0, 1));
  quad_->setNormal(0, Vec3f(0, 0, 1));
  quad_->setNormal(1, Vec3f(0, 0, 1));
  quad_->unlockVertexPointer();
  
  //generate cylinder
  cylinder_ = createCylinder(1,1,32,mCylinderInds);
  //generate line
  lines_ = rend->createVertexBuffer();
  lines_->create(VB_POSITION | VB_TEXCOORD | VB_NORMAL, 4);
}

void Forms::activateQuad(){
  quad_->activate();
}

void Forms::drawQuad(){
  quad_->draw(VB_Tristrip, 0);
}

void Forms::drawQuad(const Vector2D& position, const Vector2D& size, float rotation) const {
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->pushMatrix();
  Vec2f offset = size/2.0f;
  if (rotation != 0){
    Matrix rot = Matrix(Matrix::Rotation, Vec3f(0, 0, -1), rotation/180*(float)M_PI);
    Vec3f rotoff = rot * Vec3f(offset.x, offset.y, 0);
    offset = Vec2f(abs(rotoff.x), abs(rotoff.y));
  }
  rend->translate(position.x+offset.x, position.y+offset.y, 0);
  if (rotation != 0)
    rend->rotate(rotation, 0, 0, -1);
  rend->scale(size.x, size.y, 1);
  quad_->draw(VB_Tristrip, 0);
  rend->popMatrix();
}

void Forms::activateCylinder(){
  cylinder_->activate();
}

void Forms::drawCylinder(){
  cylinder_->draw(VB_Tristrip, mCylinderInds[0]);
  cylinder_->draw(VB_Tristrip, mCylinderInds[1]);
  cylinder_->draw(VB_Tristrip, mCylinderInds[2]);
}

VertexBuffer* Forms::createCylinder(float radius, float height, int cylinder_segments, IndexBuffer* indices[3]){
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  VertexBuffer* cylinder = rend->createVertexBuffer();
  cylinder->create(VB_POSITION | VB_NORMAL, 4*cylinder_segments+2);
  cylinder->lockVertexPointer();
  float angle = 0;
  for (int i = 0; i < 2*cylinder_segments; i+=2){
    //the hull
    cylinder->setPosition(i, Vec3f(radius*cos(angle),-height/2, radius*sin(angle)));
    cylinder->setNormal(i, Vec3f(cos(angle),0,sin(angle)));
    cylinder->setPosition(i+1, Vec3f(radius*cos(angle),height/2, radius*sin(angle)));
    cylinder->setNormal(i+1, Vec3f(cos(angle),0,sin(angle)));
    //the caps
    cylinder->setPosition(2*cylinder_segments+i, Vec3f(radius*cos(angle),-height/2, radius*sin(angle)));
    cylinder->setNormal(2*cylinder_segments+i, Vec3f(0,-1,0));
    cylinder->setPosition(2*cylinder_segments+i+1, Vec3f(radius*cos(angle),height/2, radius*sin(angle)));
    cylinder->setNormal(2*cylinder_segments+i+1, Vec3f(0,1,0));
    angle += (float)(M_PI*2./cylinder_segments);
  }
  cylinder->setPosition(4*cylinder_segments, Vec3f(0.0f,-height/2,0.0f));
  cylinder->setNormal(4*cylinder_segments, Vec3f(0,-1,0));
  cylinder->setPosition(4*cylinder_segments+1, Vec3f(0.0f,height/2,0.0f));
  cylinder->setNormal(4*cylinder_segments+1, Vec3f(0,1,0));
  cylinder->unlockVertexPointer();

  indices[0] = rend->createIndexBuffer(IndexBuffer::IB_USHORT,(cylinder_segments+1)*2);
  short* hull = (short*)indices[0]->lockIndexPointer();
  for (int i = 0; i < (cylinder_segments+1)*2; ++i){
    hull[i] = i%(2*cylinder_segments);
  }
  indices[0]->unlockIndexPointer();
  indices[1] = rend->createIndexBuffer(IndexBuffer::IB_USHORT,2*cylinder_segments+1);
  short* bottom = (short*)indices[1]->lockIndexPointer();
  bottom[0] = 4 * cylinder_segments - 2;
  int count = 0;
  for (int i = 1; i <= cylinder_segments; ++i){
    bottom[2*i-1] = 2*cylinder_segments+count;
    bottom[2 * i] = 4 * cylinder_segments;
    count+=2;
  }
  indices[1]->unlockIndexPointer();
  indices[2] = rend->createIndexBuffer(IndexBuffer::IB_USHORT,2*cylinder_segments+1);
  short* top = (short*)indices[2]->lockIndexPointer();
  top[0] = 2 * cylinder_segments + 1;
  count = 4*cylinder_segments-1;
  for (int i = 1; i <= cylinder_segments; ++i){
    top[2 * i - 1] = count;
    top[2*i] = 4 * cylinder_segments + 1;
    count-=2;
  }
  indices[2]->unlockIndexPointer();
  return cylinder;
}

VertexBuffer* Forms::createBox(float width, float height, float depth, IndexBuffer** indices){
  
   //1-----2//
  //      //
  //0----3// 6
  //|     |/ 
  //4---7//
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  VertexBuffer* box = rend->createVertexBuffer();
  box->create(VB_POSITION | VB_NORMAL, 24);
  box->lockVertexPointer();
  box->setPosition(0, Vec3f(-width/2,height/2,depth/2));
  box->setNormal(0, Vec3f(-1,0,0));
  box->setPosition(8, Vec3f(-width/2,height/2,depth/2));
  box->setNormal(8, Vec3f(0,1,0));
  box->setPosition(16, Vec3f(-width/2,height/2,depth/2));
  box->setNormal(16, Vec3f(0,0,1));

  box->setPosition(1, Vec3f(-width/2,height/2,-depth/2));
  box->setNormal(1, Vec3f(-1,0,0));
  box->setPosition(9, Vec3f(-width/2,height/2,-depth/2));
  box->setNormal(9, Vec3f(0,1,0));
  box->setPosition(17, Vec3f(-width/2,height/2,-depth/2));
  box->setNormal(17, Vec3f(0,0,-1));

  box->setPosition(2, Vec3f(width/2,height/2,-depth/2));
  box->setNormal(2, Vec3f(1,0,0));
  box->setPosition(10, Vec3f(width/2,height/2,-depth/2));
  box->setNormal(10, Vec3f(0,1,0));
  box->setPosition(18, Vec3f(width/2,height/2,-depth/2));
  box->setNormal(18, Vec3f(0,0,-1));

  box->setPosition(3, Vec3f(width/2,height/2,depth/2));
  box->setNormal(3, Vec3f(1,0,0));
  box->setPosition(11, Vec3f(width/2,height/2,depth/2));
  box->setNormal(11, Vec3f(0,1,0));
  box->setPosition(19, Vec3f(width/2,height/2,depth/2));
  box->setNormal(19, Vec3f(0,0,1));

  box->setPosition(4, Vec3f(-width/2,-height/2,depth/2));
  box->setNormal(4, Vec3f(-1,0,0));
  box->setPosition(12, Vec3f(-width/2,-height/2,depth/2));
  box->setNormal(12, Vec3f(0,-1,0));
  box->setPosition(20, Vec3f(-width/2,-height/2,depth/2));
  box->setNormal(20, Vec3f(0,0,1));

  box->setPosition(5, Vec3f(-width/2,-height/2,-depth/2));
  box->setNormal(5, Vec3f(-1,0,0));
  box->setPosition(13, Vec3f(-width/2,-height/2,-depth/2));
  box->setNormal(13, Vec3f(0,-1,0));
  box->setPosition(21, Vec3f(-width/2,-height/2,-depth/2));
  box->setNormal(21, Vec3f(0,0,-1));

  box->setPosition(6, Vec3f(width/2,-height/2,-depth/2));
  box->setNormal(6, Vec3f(1,0,0));
  box->setPosition(14, Vec3f(width/2,-height/2,-depth/2));
  box->setNormal(14, Vec3f(0,-1,0));
  box->setPosition(22, Vec3f(width/2,-height/2,-depth/2));
  box->setNormal(22, Vec3f(0,0,-1));

  box->setPosition(7, Vec3f(width/2,-height/2,depth/2));
  box->setNormal(7, Vec3f(1,0,0));
  box->setPosition(15, Vec3f(width/2,-height/2,depth/2));
  box->setNormal(15, Vec3f(0,-1,0));
  box->setPosition(23, Vec3f(width/2,-height/2,depth/2));
  box->setNormal(23, Vec3f(0,0,1));
  box->unlockVertexPointer();
  
  indices[0] = rend->createIndexBuffer(IndexBuffer::IB_USHORT, 36);
  short* idx = (short*)indices[0]->lockIndexPointer();
  //top
  idx[0] = 8; idx[1] = 11; idx[2] = 9;
  idx[3] = 9; idx[4] = 11; idx[5] = 10;
  //front
  idx[6] = 20; idx[7] = 23; idx[8] = 16;
  idx[9] = 16; idx[10] = 23; idx[11] = 19;
  //left
  idx[12] = 5; idx[13] = 4; idx[14] = 1;
  idx[15] = 1; idx[16] = 4; idx[17] = 0;
  //back
  idx[18] = 22; idx[19] = 21; idx[20] = 18;
  idx[21] = 18; idx[22] = 21; idx[23] = 17;
  //right
  idx[24] = 7; idx[25] = 6; idx[26] = 3;
  idx[27] = 3; idx[28] = 6; idx[29] = 2;
  //bottom
  idx[30] = 13; idx[31] = 14; idx[32] = 12;
  idx[33] = 12; idx[34] = 14; idx[35] = 15;
  indices[0]->unlockIndexPointer();
  return box;
}

void Forms::activateLines(){
  lines_->activate();
}

void Forms::drawLine(Vec2f const& from, Vec2f const& to){
  Vec2f dir = to - from;
  Vec2f ortho = dir.cross().normalize()*mLineWidth/2;
  Vec2f ul = from - ortho;
  Vec2f ll = to - ortho;
  Vec2f ur = from + ortho;
  Vec2f lr = to + ortho;
  lines_->lockVertexPointer();
  lines_->setPosition(0, Vec3f(ul.x, ul.y, 0));
  lines_->setPosition(2, Vec3f(ll.x, ll.y, 0));
  lines_->setPosition(1, Vec3f(ur.x, ur.y, 0));
  lines_->setPosition(3, Vec3f(lr.x, lr.y, 0));
  lines_->unlockVertexPointer();
  lines_->draw(VB_Tristrip, NULL);
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
  //the same corners and texture coordinates as the old renderers' quad
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
