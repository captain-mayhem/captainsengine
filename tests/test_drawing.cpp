#include <gtest/gtest.h>

#include <climits>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include <image/image.h>
#include <io/Tracing.h>
#include <io/TraceManager.h>
#include <image/loader.h>
#include <math/matrixstack.h>
#include <mesh/mesh.h>
#include <mesh/model.h>
#include <platform/assets.h>
#include <renderer/camera.h>
#include <renderer/drawcontext.h>
#include <renderer/screentext.h>
#include <renderer/spritebatch.h>
#include <renderer/forms.h>
#include <renderer/light.h>
#include <renderer/renderdevice.h>
#include <renderer/scene.h>
#include <renderer/uniformstream.h>
#include <renderpass/Forward.h>
#include <renderpass/LightPrepass.h>

#include "testenv.h"
#include "testwindow.h"

using namespace CGE;
using namespace CGE::Gfx;

// The engine's drawing code on a render device: the matrix stack, the
// uniform stream, the draw context, Forms, ScreenText, Mesh, Scene and the render
// techniques. The GL tests draw into render targets on a hidden window and
// read the pixels back.

TEST(MatrixStack, MultipliesFromTheRight){
  MatrixStack stack;
  EXPECT_EQ(stack.depth(), 1);
  stack.translate(10, 0, 0);
  stack.scale(2, 2, 1);
  Vec3f p = stack.top()*Vec3f(1, 1, 0);
  EXPECT_FLOAT_EQ(p.x, 12);
  EXPECT_FLOAT_EQ(p.y, 2);
  EXPECT_FLOAT_EQ(p.z, 0);
}

TEST(MatrixStack, RotatesInDegrees){
  MatrixStack stack;
  stack.rotate(90, 0, 0, 1);
  Vec3f p = stack.top()*Vec3f(1, 0, 0);
  EXPECT_NEAR(p.x, 0, 1e-6);
  EXPECT_NEAR(p.y, 1, 1e-6);
}

TEST(MatrixStack, PopRestoresThePushedMatrix){
  MatrixStack stack;
  stack.translate(1, 2, 3);
  stack.push();
  EXPECT_EQ(stack.depth(), 2);
  stack.loadIdentity();
  stack.multiply(Matrix(Matrix::Scale, Vec3f(5, 5, 5)));
  stack.pop();
  EXPECT_EQ(stack.depth(), 1);
  Vec3f p = stack.top()*Vec3f(0, 0, 0);
  EXPECT_FLOAT_EQ(p.x, 1);
  EXPECT_FLOAT_EQ(p.y, 2);
  EXPECT_FLOAT_EQ(p.z, 3);
  //a pop without a push keeps the last matrix
  stack.pop();
  EXPECT_EQ(stack.depth(), 1);
  EXPECT_FLOAT_EQ(stack.top().getTranslation().z, 3);
}

namespace{

//! a headless Null device in a frame
class NullDeviceTest : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("Null");
    ASSERT_TRUE(device_->init(NativeWindow(), DeviceDesc()));
  }
  std::unique_ptr<RenderDevice> device_;
};

}

TEST_F(NullDeviceTest, UniformStreamGivesEveryPushItsOwnRange){
  //the Null device has an offset alignment of 256 bytes, like many GPUs
  const size_t align = device_->getCaps().uniformBufferOffsetAlignment;
  ASSERT_EQ(align, 256u);
  UniformStream stream(*device_, 4*align);
  ASSERT_TRUE(device_->beginFrame());
  float a[16] = {1};
  float b[16] = {2};
  UniformRange first = stream.push(a, sizeof(a));
  UniformRange second = stream.push(b, sizeof(b));
  ASSERT_NE(first.buffer, nullptr);
  ASSERT_NE(second.buffer, nullptr);
  EXPECT_EQ(first.buffer, second.buffer);
  EXPECT_EQ(first.offset, 0u);
  EXPECT_EQ(second.offset, align);
  EXPECT_EQ(second.size, sizeof(b));
  device_->endFrame();
}

TEST_F(NullDeviceTest, UniformStreamGrowsByBlocksAndStartsOverEachFrame){
  const size_t align = device_->getCaps().uniformBufferOffsetAlignment;
  UniformStream stream(*device_, 2*align);
  float data[32] = {};
  ASSERT_TRUE(device_->beginFrame());
  //two aligned pushes fill a block, the third needs another one
  stream.push(data, sizeof(data));
  uint64_t generation = stream.getGeneration();
  stream.push(data, sizeof(data));
  UniformRange third = stream.push(data, sizeof(data));
  EXPECT_EQ(stream.getBlockCount(), 2);
  EXPECT_EQ(third.offset, 0u);
  EXPECT_EQ(stream.getGeneration(), generation);
  device_->endFrame();

  ASSERT_TRUE(device_->beginFrame());
  UniformRange next = stream.push(data, sizeof(data));
  EXPECT_NE(stream.getGeneration(), generation);
  EXPECT_EQ(next.offset, 0u);
  EXPECT_EQ(stream.getBlockCount(), 2);
  //more than a block doesn't fit
  std::vector<uint8_t> big(2*align+1);
  EXPECT_EQ(stream.push(big.data(), big.size()).buffer, nullptr);
  device_->endFrame();
}

TEST_F(NullDeviceTest, DrawContextNeedsAPass){
  DrawContext ctx(*device_);
  Forms forms(*device_);
  ASSERT_TRUE(device_->beginFrame());
  EXPECT_FALSE(ctx.prepare(BuiltinShader::Color, Forms::getVertexLayout(), PrimitiveType::TriangleStrip));
  ASSERT_TRUE(device_->beginPass(PassDesc()));
  EXPECT_TRUE(ctx.prepare(BuiltinShader::Color, Forms::getVertexLayout(), PrimitiveType::TriangleStrip));
  //the pipeline is made once per state
  Pipeline* pipeline = ctx.getPipeline(ctx.getShader(BuiltinShader::Color), Forms::getVertexLayout(), PrimitiveType::TriangleStrip);
  EXPECT_EQ(ctx.getPipeline(ctx.getShader(BuiltinShader::Color), Forms::getVertexLayout(), PrimitiveType::TriangleStrip), pipeline);
  ctx.setBlend(BlendMode::Alpha);
  EXPECT_NE(ctx.getPipeline(ctx.getShader(BuiltinShader::Color), Forms::getVertexLayout(), PrimitiveType::TriangleStrip), pipeline);
  forms.drawQuad(ctx, Vec2f(0, 0), Vec2f(1, 1));
  device_->endPass();
  device_->endFrame();
}

#if defined(CGE_TEST_WINDOW)

namespace{

using TestEnv::TestWindow;

struct Pixel{
  uint8_t r, g, b, a;
};

::testing::AssertionResult isNear(const Pixel& p, int r, int g, int b, int tolerance = 2){
  if (abs(p.r-r) <= tolerance && abs(p.g-g) <= tolerance && abs(p.b-b) <= tolerance)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "pixel (" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b
    << ") instead of (" << r << ", " << g << ", " << b << ")";
}

//! the picture in tests/data/image.png, rows top to bottom
const int picture[2][3][3] = {
  {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}},
  {{255, 255, 255}, {0, 0, 0}, {128, 64, 32}},
};

//! collects the errors and warnings that are traced
class ProblemCollector : public CGE::TraceOutputter{
public:
  bool init() override {return true;}
  void trace(unsigned channel, int level, const char* function, const char* message) override{
    if (level <= TRACE_WARNING)
      problems.push_back(message);
  }
  std::vector<std::string> problems;
};

//! writes a text file
void writeFile(const std::filesystem::path& path, const char* text){
  std::ofstream out(path, std::ios::binary);
  out << text;
}

class Drawing : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("OpenGL");
    ASSERT_NE(device_, nullptr);
    DeviceDesc desc;
    desc.debug = true;
    if (!window_.create(*device_, desc))
      GTEST_SKIP() << "no window";
    if (!device_->init(window_.native(), desc))
      GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
    ctx_ = std::make_unique<DrawContext>(*device_);
    forms_ = std::make_unique<Forms>(*device_);
    SamplerDesc nearest;
    nearest.minFilter = nearest.magFilter = Filter::Nearest;
    nearest.addressU = nearest.addressV = AddressMode::ClampToEdge;
    nearest_ = device_->createSampler(nearest);
    //the drawing must not cause errors or warnings, also none from the driver (KHR_debug)
    CGE::TraceManager::instance()->setTraceOutputter(&collector_);
    //the manager replays what was traced before
    collector_.problems.clear();
    collecting_ = true;
  }

  void TearDown() override{
    //everything on the device goes before it
    nearest_.reset();
    forms_.reset();
    ctx_.reset();
    if (collecting_){
      CGE::TraceManager::instance()->removeTraceOutputter(&collector_);
      for (const std::string& problem : collector_.problems)
        ADD_FAILURE() << "traced: " << problem;
    }
  }

  std::unique_ptr<Gfx::RenderTarget> target(int width, int height, Format depth = Format::Undefined){
    RenderTargetDesc desc;
    desc.width = width;
    desc.height = height;
    desc.depthFormat = depth;
    return device_->createRenderTarget(desc);
  }

  //! a frame and a black pass into target, with a projection in pixels (y up)
  void begin(Gfx::RenderTarget* target){
    ASSERT_TRUE(device_->beginFrame());
    PassDesc pass;
    pass.target = target;
    pass.clearColor = {0, 0, 0, 1};
    ASSERT_TRUE(device_->beginPass(pass));
    const RenderTargetDesc& desc = target->getDesc();
    ctx_->setProjection(device_->ortho(0, (float)desc.width, 0, (float)desc.height, -10, 10));
    ctx_->setView(Matrix(Matrix::Identity));
  }

  void end(){
    device_->endPass();
    device_->endFrame();
  }

  //! the pixels, rows from the bottom up
  std::vector<Pixel> read(Gfx::RenderTarget* target){
    const RenderTargetDesc& desc = target->getDesc();
    std::vector<Pixel> pixels(desc.width*desc.height);
    EXPECT_TRUE(device_->readPixels(target, 0, {0, 0, desc.width, desc.height}, pixels.data()));
    return pixels;
  }

  //! image.png as a texture
  std::unique_ptr<Gfx::Texture> pictureTexture(){
    std::unique_ptr<Image> image(ImageLoader::load(TestEnv::dataFile("image.png").c_str()));
    if (!image)
      return nullptr;
    image->convertFormat(4);
    TextureDesc desc;
    desc.width = 3;
    desc.height = 2;
    return device_->createTexture(desc, image->getData());
  }

  //! writes an OBJ of a quad from (x0, y0) to (x1, y1) at depth z
  /*! material: the lines of its MTL material; by default white with
      image.png; nullptr for a quad without a material */
  std::string writeQuadObj(const std::string& name, float x0, float y0, float x1, float y1, float z,
    const char* material = "Kd 1 1 1\nmap_Kd image.png\n"){
    std::filesystem::path dir = std::filesystem::temp_directory_path()/"cge_test_drawing";
    std::filesystem::create_directories(dir);
    std::filesystem::copy_file(TestEnv::dataFile("image.png"), dir/"image.png", std::filesystem::copy_options::overwrite_existing);
    char text[1024];
    std::string mtllib, usemtl;
    if (material){
      snprintf(text, sizeof(text), "newmtl pic\n%s", material);
      writeFile(dir/(name+".mtl"), text);
      mtllib = "mtllib "+name+".mtl\n";
      usemtl = "usemtl pic\n";
    }
    snprintf(text, sizeof(text),
      "%s"
      "v %g %g %g\nv %g %g %g\nv %g %g %g\nv %g %g %g\n"
      "vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\n"
      "vn 0 0 1\n"
      "%s"
      "f 1/1/1 2/2/1 3/3/1 4/4/1\n",
      mtllib.c_str(), x0, y0, z, x1, y0, z, x1, y1, z, x0, y1, z, usemtl.c_str());
    writeFile(dir/(name+".obj"), text);
    return (dir/(name+".obj")).string();
  }

  //! a scene with the quads, a point light and a camera looking at them
  void fillScene(Scene& scene, Camera& cam, const std::vector<std::string>& objs){
    for (const std::string& obj : objs){
      Mesh* mesh = new Mesh();
      ASSERT_TRUE(mesh->loadFromFile(obj));
      ASSERT_TRUE(mesh->buildBuffers(*device_));
      scene.addMesh(mesh);
      scene.addModel(new Model(mesh));
    }
    Light* light = new Light(Light::Point, Vec3f(1.5f, 1, 2));
    light->setColor(Color(1, 0.8f, 0.6f, 1));
    scene.addLight(light);
    cam.project(60, 1.5f, 0.1f, 100);
    cam.lookAt(Vec3f(1.5f, 1, 2.5f), Vec3f(1.5f, 1, 0), Vec3f(0, 1, 0));
    scene.setActiveCam(&cam);
  }

  //declared first, so the device goes before the window
  TestWindow window_;
  std::unique_ptr<RenderDevice> device_;
  std::unique_ptr<DrawContext> ctx_;
  std::unique_ptr<Forms> forms_;
  std::unique_ptr<Sampler> nearest_;
  ProblemCollector collector_;
  bool collecting_ = false;
};

}

TEST_F(Drawing, QuadsTakeTheContextsColorAndModelMatrix){
  auto rt = target(8, 8);
  begin(rt.get());
  ctx_->setColor(1, 0, 0);
  forms_->drawQuad(*ctx_, Vec2f(2, 2), Vec2f(4, 4));
  //the unit quad in the model matrix
  ctx_->setColor(0, 0, 1);
  ctx_->model().push();
  ctx_->model().translate(7, 7, 0);
  ctx_->model().scale(2, 2, 1);
  forms_->drawQuad(*ctx_);
  ctx_->model().pop();
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[2*8+2], 255, 0, 0));
  EXPECT_TRUE(isNear(pixels[5*8+5], 255, 0, 0));
  EXPECT_TRUE(isNear(pixels[1*8+1], 0, 0, 0));
  EXPECT_TRUE(isNear(pixels[6*8+3], 0, 0, 0));
  EXPECT_TRUE(isNear(pixels[7*8+7], 0, 0, 255));
  EXPECT_TRUE(isNear(pixels[6*8+6], 0, 0, 255));
  EXPECT_EQ(ctx_->model().depth(), 1);
}

TEST_F(Drawing, EveryDrawKeepsItsUniforms){
  //eight draws in one pass with eight colours: the stream must not reuse their ranges
  auto rt = target(8, 1);
  begin(rt.get());
  for (int i = 0; i < 8; ++i){
    ctx_->setColor(i/7.0f, 1-i/7.0f, (i%2)*1.0f);
    forms_->drawQuad(*ctx_, Vec2f((float)i, 0), Vec2f(1, 1));
  }
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int i = 0; i < 8; ++i)
    EXPECT_TRUE(isNear(pixels[i], (int)(i/7.0f*255+0.5f), (int)((1-i/7.0f)*255+0.5f), (i%2)*255)) << "draw " << i;
}

TEST_F(Drawing, TexturedQuadsAreUpright){
  auto rt = target(3, 2);
  auto tex = pictureTexture();
  ASSERT_NE(tex, nullptr);
  begin(rt.get());
  ctx_->setTexture(tex.get(), nearest_.get());
  forms_->drawQuad(*ctx_, Vec2f(0, 0), Vec2f(3, 2));
  ctx_->setTexture(nullptr);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int y = 0; y < 2; ++y){
    for (int x = 0; x < 3; ++x){
      const int* expected = picture[1-y][x];
      EXPECT_TRUE(isNear(pixels[y*3+x], expected[0], expected[1], expected[2])) << x << ", " << y;
    }
  }
}

TEST_F(Drawing, LinesHaveTheLineWidth){
  auto rt = target(8, 8);
  begin(rt.get());
  ctx_->setColor(0, 1, 0);
  forms_->setLineWidth(2);
  forms_->drawLine(*ctx_, Vec2f(0, 4), Vec2f(8, 4));
  forms_->drawLine(*ctx_, Vec2f(1, 0), Vec2f(1, 2));
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int x = 0; x < 8; ++x){
    EXPECT_TRUE(isNear(pixels[3*8+x], 0, 255, 0));
    EXPECT_TRUE(isNear(pixels[4*8+x], 0, 255, 0));
    EXPECT_TRUE(isNear(pixels[6*8+x], 0, 0, 0));
  }
  EXPECT_TRUE(isNear(pixels[0*8+0], 0, 255, 0));
  EXPECT_TRUE(isNear(pixels[1*8+1], 0, 255, 0));
  EXPECT_TRUE(isNear(pixels[1*8+2], 0, 0, 0));
}

TEST_F(Drawing, BlendModesCombineWithWhatIsDrawn){
  auto rt = target(4, 1);
  begin(rt.get());
  ctx_->setColor(0.5f, 0.5f, 0.5f);
  forms_->drawQuad(*ctx_, Vec2f(0, 0), Vec2f(4, 1));
  //premultiplied: src + dst*(1-alpha)
  ctx_->setBlend(BlendMode::PremultipliedAlpha);
  ctx_->setColor(0.5f, 0, 0, 0.5f);
  forms_->drawQuad(*ctx_, Vec2f(0, 0), Vec2f(1, 1));
  //multiply: src*dst
  ctx_->setBlend(BlendMode::Multiply);
  ctx_->setColor(1, 0.5f, 0);
  forms_->drawQuad(*ctx_, Vec2f(1, 0), Vec2f(1, 1));
  //additive: src*alpha + dst
  ctx_->setBlend(BlendMode::Additive);
  ctx_->setColor(0, 0, 1, 0.5f);
  forms_->drawQuad(*ctx_, Vec2f(2, 0), Vec2f(1, 1));
  ctx_->setBlend(BlendMode::Opaque);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[0], 191, 64, 64));
  EXPECT_TRUE(isNear(pixels[1], 128, 64, 0));
  EXPECT_TRUE(isNear(pixels[2], 128, 128, 255));
  EXPECT_TRUE(isNear(pixels[3], 128, 128, 128));
}

TEST_F(Drawing, DepthOnlyDrawsMaskLaterOnes){
  //a mask in the depth buffer only, then a draw that passes where depth is equal
  auto rt = target(4, 1, Format::Depth24);
  ASSERT_TRUE(device_->beginFrame());
  PassDesc pass;
  pass.target = rt.get();
  pass.clearColor = {0, 0, 0, 1};
  pass.clearDepth = 0;
  ASSERT_TRUE(device_->beginPass(pass));
  ctx_->setProjection(device_->ortho(0, 4, 0, 1, -1, 1));
  ctx_->setView(Matrix(Matrix::Identity));
  ctx_->setColorWrite(false);
  ctx_->setDepth(true, true, CompareFunc::GreaterEqual);
  ctx_->setColor(1, 1, 1);
  forms_->drawQuad(*ctx_, Vec2f(1, 0), Vec2f(2, 1));
  ctx_->setColorWrite(true);
  ctx_->setDepth(true, false, CompareFunc::Equal);
  ctx_->setColor(0, 1, 0);
  forms_->drawQuad(*ctx_, Vec2f(0, 0), Vec2f(4, 1));
  ctx_->setDepth(false, false);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[0], 0, 0, 0));
  EXPECT_TRUE(isNear(pixels[1], 0, 255, 0));
  EXPECT_TRUE(isNear(pixels[2], 0, 255, 0));
  EXPECT_TRUE(isNear(pixels[3], 0, 0, 0));
}

TEST_F(Drawing, ShapesAreLitByTheLights){
  auto rt = target(8, 8, Format::Depth24);
  auto box = Forms::createBox(*device_, 4, 4, 4);
  ASSERT_NE(box, nullptr);
  Light light(Light::Directional, Vec3f(0, 0, 1));
  light.setColor(Color(0.5f, 0.25f, 1, 1));
  begin(rt.get());
  ctx_->setDepth(true, true);
  //unlit: the material's diffuse colour times the context's colour
  ctx_->setColor(0, 1, 0);
  ctx_->model().push();
  ctx_->model().translate(2, 2, 0);
  Forms::drawShape(*ctx_, *box);
  ctx_->model().pop();
  //lit from the front: the light's colour
  ctx_->setColor(1, 1, 1);
  ctx_->setLights({&light});
  ctx_->model().push();
  ctx_->model().translate(6, 6, 0);
  Forms::drawShape(*ctx_, *box);
  ctx_->model().pop();
  ctx_->clearLights();
  ctx_->setDepth(false, false);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[1*8+1], 0, 255, 0));
  EXPECT_TRUE(isNear(pixels[6*8+6], 128, 64, 255));
}

TEST_F(Drawing, MeshesShowTheirMaterialsTextures){
  Mesh mesh;
  ASSERT_TRUE(mesh.loadFromFile(writeQuadObj("picture", 0, 0, 3, 2, 0)));
  ASSERT_TRUE(mesh.buildBuffers(*device_));
  auto rt = target(3, 2);
  begin(rt.get());
  mesh.draw(*ctx_, Mesh::DRAW_ALL);
  end();
  EXPECT_EQ(ctx_->getTexture(), nullptr);
  std::vector<Pixel> pixels = read(rt.get());
  for (int y = 0; y < 2; ++y){
    for (int x = 0; x < 3; ++x){
      const int* expected = picture[1-y][x];
      EXPECT_TRUE(isNear(pixels[y*3+x], expected[0], expected[1], expected[2])) << x << ", " << y;
    }
  }
}

namespace{

//! expects the picture of image.png in pixels of a 3x2 target (rows from the bottom up)
void expectPicture(const std::vector<Pixel>& pixels){
  for (int y = 0; y < 2; ++y){
    for (int x = 0; x < 3; ++x){
      const int* expected = picture[1-y][x];
      EXPECT_TRUE(isNear(pixels[y*3+x], expected[0], expected[1], expected[2])) << x << ", " << y;
    }
  }
}

}

TEST_F(Drawing, MeshesWithoutMaterialKeepTheContextsTexture){
  Mesh mesh;
  ASSERT_TRUE(mesh.loadFromFile(writeQuadObj("plain", 0, 0, 3, 2, 0, nullptr)));
  ASSERT_TRUE(mesh.buildBuffers(*device_));
  auto tex = pictureTexture();
  ASSERT_NE(tex, nullptr);
  auto rt = target(3, 2);
  begin(rt.get());
  ctx_->setTexture(tex.get(), nearest_.get());
  mesh.draw(*ctx_, Mesh::DRAW_ALL);
  end();
  EXPECT_EQ(ctx_->getTexture(), tex.get());
  EXPECT_EQ(ctx_->getSampler(), nearest_.get());
  ctx_->setTexture(nullptr);
  expectPicture(read(rt.get()));
}

TEST_F(Drawing, ModelsGiveMeshesWithoutMaterialTheirDeviceTexture){
  Mesh mesh;
  ASSERT_TRUE(mesh.loadFromFile(writeQuadObj("plain", 0, 0, 3, 2, 0, nullptr)));
  ASSERT_TRUE(mesh.buildBuffers(*device_));
  auto tex = pictureTexture();
  ASSERT_NE(tex, nullptr);
  Model model(&mesh);
  model.assignDeviceTexture(tex.get(), 0);
  auto rt = target(3, 2);
  begin(rt.get());
  model.render(*ctx_, Mesh::DRAW_ALL);
  end();
  EXPECT_EQ(ctx_->getTexture(), nullptr);
  expectPicture(read(rt.get()));
}

TEST_F(Drawing, ScenesLoadTheirTexturesOntoTheDevice){
  //a scene file with the quad without material, image.png and one model with it
  std::string obj = writeQuadObj("plain", 0, 0, 3, 2, 0, nullptr);
  std::filesystem::path dir = std::filesystem::path(obj).parent_path();
  {
    std::ofstream out(dir/"test.hsf", std::ios::binary);
    auto writeUnsigned = [&](unsigned value){out.write((const char*)&value, sizeof(value));};
    auto writeString = [&](const std::string& text){writeUnsigned((unsigned)text.size()); out.write(text.c_str(), text.size());};
    int version = 1;
    out.write((const char*)&version, sizeof(version));
    writeUnsigned(1);
    writeString("plain.obj");
    writeUnsigned(1);
    writeString("image.png");
    //no lights
    writeUnsigned(0);
    writeUnsigned(1);
    //the model: id, mesh, textures, transformation, attributes
    writeUnsigned(7);
    writeUnsigned(0);
    for (int i = 0; i < MAX_TEXTURES; ++i)
      writeUnsigned(i == 0 ? 0 : UINT_MAX);
    Matrix identity(Matrix::Identity);
    out.write((const char*)identity.getData(), 16*sizeof(float));
    for (int i = 0; i < MAX_ATTRIBS; ++i){
      int attrib = 0;
      out.write((const char*)&attrib, sizeof(attrib));
    }
  }
  //the scene's files are relative to the working directory
  std::filesystem::path cwd = std::filesystem::current_path();
  std::filesystem::current_path(dir);
  Scene scene;
  scene.load("test.hsf", *device_);
  std::filesystem::current_path(cwd);

  Gfx::Texture* tex = scene.getDeviceTexture("image.png");
  ASSERT_NE(tex, nullptr);
  Model* model = scene.getModel(7);
  ASSERT_NE(model, nullptr);
  EXPECT_EQ(model->getDeviceTexture(0), tex);
  auto rt = target(3, 2);
  begin(rt.get());
  scene.render(*ctx_);
  end();
  expectPicture(read(rt.get()));
  //what is on the device goes before it
  scene.clear();
  EXPECT_EQ(scene.getDeviceTexture("image.png"), nullptr);
}

TEST_F(Drawing, ForwardAndLightPrepassLightTheSame){
  Scene scene;
  Camera cam;
  fillScene(scene, cam, {writeQuadObj("lit", 0, 0, 3, 2, 0)});
  ForwardRenderer forward;
  forward.setScene(scene);
  LightPrepassRenderer prepass;
  prepass.setScene(scene);
  ASSERT_TRUE(prepass.createResources(*ctx_));

  auto forwardTarget = target(30, 20, Format::Depth24);
  auto prepassTarget = target(30, 20, Format::Depth24);
  ASSERT_TRUE(device_->beginFrame());
  PassDesc output;
  output.target = forwardTarget.get();
  forward.render(*ctx_, output);
  output.target = prepassTarget.get();
  prepass.render(*ctx_, output);
  device_->endFrame();
  prepass.releaseResources(*ctx_);

  std::vector<Pixel> forwardPixels = read(forwardTarget.get());
  std::vector<Pixel> prepassPixels = read(prepassTarget.get());
  int lit = 0;
  for (size_t i = 0; i < forwardPixels.size(); ++i){
    const Pixel& p = forwardPixels[i];
    EXPECT_TRUE(isNear(prepassPixels[i], p.r, p.g, p.b, 4)) << "pixel " << i;
    if (p.r+p.g+p.b > 20)
      ++lit;
  }
  //the quad covers about half of the view, and one of its texels is black
  EXPECT_GT(lit, (int)forwardPixels.size()/4);
  EXPECT_LT(lit, (int)forwardPixels.size());
}

TEST_F(Drawing, LightPrepassHidesTransparentSurfacesBehindOpaqueOnes){
  //a white opaque quad in front of a larger blue transparent one
  const char* white = "Kd 1 1 1\n";
  const char* blue = "Kd 0 0 1\nd 0.5\n";
  Scene scene;
  Camera cam;
  fillScene(scene, cam, {writeQuadObj("front", 1, 0.5f, 2, 1.5f, 0, white), writeQuadObj("back", 0, 0, 3, 2, -0.5f, blue)});
  LightPrepassRenderer prepass;
  prepass.setScene(scene);
  ASSERT_TRUE(prepass.createResources(*ctx_));
  auto rt = target(30, 20, Format::Depth24);
  ASSERT_TRUE(device_->beginFrame());
  PassDesc output;
  output.target = rt.get();
  prepass.render(*ctx_, output);
  device_->endFrame();
  prepass.releaseResources(*ctx_);
  std::vector<Pixel> pixels = read(rt.get());

  //the opaque quad alone, drawn forward
  Scene opaqueOnly;
  Camera cam2;
  fillScene(opaqueOnly, cam2, {writeQuadObj("front2", 1, 0.5f, 2, 1.5f, 0, white)});
  ForwardRenderer forward;
  forward.setScene(opaqueOnly);
  auto reference = target(30, 20, Format::Depth24);
  ASSERT_TRUE(device_->beginFrame());
  output.target = reference.get();
  forward.render(*ctx_, output);
  device_->endFrame();
  std::vector<Pixel> expected = read(reference.get());

  //the centre: the opaque quad, not blended with the blue one behind it
  const Pixel& centre = pixels[10*30+15];
  const Pixel& opaque = expected[10*30+15];
  EXPECT_GT(opaque.r, 20);
  EXPECT_TRUE(isNear(centre, opaque.r, opaque.g, opaque.b, 4));
  //beside it: only the blue quad, blended over black
  const Pixel& beside = pixels[10*30+8];
  EXPECT_LT(beside.r, 3);
  EXPECT_GT(beside.b, 20);
  EXPECT_LT(beside.b, 140);
}


TEST_F(Drawing, ScreenTextDrawsItsTextsInTheirColours){
  //the font is the asset fonts/LiberationMono-Regular.ttf of the engine's folder
  ScreenText text;
  SpriteBatch batch(*ctx_);
  //"A" in red for one frame at (2, 2), "B" in green at (20, 2) for two seconds
  text.setColor(1, 0, 0);
  text.print(2, 2, "A", 0);
  text.setColor(0, 1, 0);
  text.print(20, 2, "B", 0, 2.0f);
  auto rt = target(40, 20);
  std::string root = Assets::getRoot();
  Assets::setRoot(CGE_TEST_ENGINE_DIR);
  begin(rt.get());
  //the virtual screen with y down
  ctx_->setProjection(device_->ortho(0, 40, 20, 0, -10, 10));
  text.render(batch, 20, 0.5);
  batch.flush();
  end();
  Assets::setRoot(root);
  std::vector<Pixel> pixels = read(rt.get());
  //each text's cell is 16 units high from its position (y up), the characters about 10 units wide
  int red = 0, green = 0;
  for (int y = 0; y < 20; ++y){
    for (int x = 0; x < 40; ++x){
      const Pixel& p = pixels[y*40+x];
      EXPECT_EQ(p.b, 0) << x << "," << y;
      if (p.r > 128){
        ++red;
        EXPECT_TRUE(x >= 2 && x < 12 && y >= 0 && y < 18) << x << "," << y;
      }
      if (p.g > 128){
        ++green;
        EXPECT_TRUE(x >= 20 && x < 30 && y >= 0 && y < 18) << x << "," << y;
      }
    }
  }
  EXPECT_GT(red, 10);
  EXPECT_GT(green, 10);
  //the red text was shown for its frame, the green one stays
  EXPECT_FALSE(text.isEmpty());
  begin(rt.get());
  ctx_->setProjection(device_->ortho(0, 40, 20, 0, -10, 10));
  text.render(batch, 20, 0.5);
  batch.flush();
  end();
  pixels = read(rt.get());
  red = green = 0;
  for (const Pixel& p : pixels){
    red += p.r > 128;
    green += p.g > 128;
  }
  EXPECT_EQ(red, 0);
  EXPECT_GT(green, 10);
  text.clear();
  EXPECT_TRUE(text.isEmpty());
  //the fonts go before the device
  text.releaseFonts();
}

#endif
