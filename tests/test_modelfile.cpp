// ModelFile: glTF 2.0 files (tests/data/gltf, written by make_fixtures.py)
// read into meshes, materials and a node hierarchy, and files with errors.
// The drawing of glTF models is tested in test_drawing.cpp.

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <image/image.h>
#include <mesh/mesh.h>
#include <mesh/model.h>
#include <mesh/modelfile.h>
#include <renderer/material.h>
#include <renderer/scenenode.h>

#include "testenv.h"

using namespace CGE;

namespace{

//! linear 0.5 in sRGB, as glTF's colour factors are converted
const float kHalfInSrgb = 0.735357f;

::testing::AssertionResult isNear(const Vec3f& v, float x, float y, float z, float tolerance = 1e-4f){
  if (fabs(v.x-x) <= tolerance && fabs(v.y-y) <= tolerance && fabs(v.z-z) <= tolerance)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "(" << v.x << ", " << v.y << ", " << v.z << ") instead of ("
    << x << ", " << y << ", " << z << ")";
}

std::unique_ptr<ModelFile> load(const std::string& name, std::string* error = nullptr){
  //the files are assets below tests/data (main.cpp sets the root)
  return ModelFile::load("gltf/"+name, error);
}

//! the triangle's vertex indices
::testing::AssertionResult hasTriangle(const Mesh& mesh, int index, int v0, int v1, int v2){
  const Triangle* tri = mesh.getTriangleIndices(index);
  if (tri->v0 == v0 && tri->v1 == v1 && tri->v2 == v2)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "triangle " << index << " is (" << tri->v0 << ", " << tri->v1 << ", "
    << tri->v2 << ") instead of (" << v0 << ", " << v1 << ", " << v2 << ")";
}

}

//! parts.gltf (external files), parts_embedded.gltf (data URIs) and parts.glb hold the same model
class ModelFileParts : public ::testing::TestWithParam<const char*>{
protected:
  void SetUp() override{
    std::string error;
    model_ = load(GetParam(), &error);
    ASSERT_NE(model_, nullptr) << error;
    EXPECT_EQ(error, "");
  }
  std::unique_ptr<ModelFile> model_;
};

INSTANTIATE_TEST_SUITE_P(Files, ModelFileParts, ::testing::Values("parts.gltf", "parts_embedded.gltf", "parts.glb"));

TEST_P(ModelFileParts, HasTheNodesOfTheScene){
  SceneNode* root = model_->getRoot();
  EXPECT_EQ(root->getType(), SceneNode::GROUP);
  EXPECT_EQ(root->getName(), "");
  ASSERT_EQ(root->getChildren().size(), 3u);
  EXPECT_EQ(root->getChildren()[0]->getName(), "body");
  EXPECT_EQ(root->getChildren()[1]->getName(), "marker");
  EXPECT_EQ(root->getChildren()[2]->getName(), "body_copy");
  //nodes that aren't in the scene are left out
  EXPECT_EQ(model_->findNode("unused"), nullptr);
  EXPECT_TRUE(model_->getWarnings().empty());

  SceneNode* body = model_->findNode("body");
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->getParent(), root);
  ASSERT_EQ(body->getChildren().size(), 3u);
  EXPECT_EQ(body->getChildren()[0]->getName(), "socket_left");
  EXPECT_EQ(body->getChildren()[1]->getName(), "socket_right");
  EXPECT_EQ(body->getChildren()[2]->getName(), "arm");
  SceneNode* tip = model_->findNode("tip");
  ASSERT_NE(tip, nullptr);
  EXPECT_EQ(tip->getParent(), body->getChildren()[1]);
  EXPECT_EQ(model_->findNode("marker")->getType(), SceneNode::GROUP);
  EXPECT_EQ(model_->findNode("socket_left")->getType(), SceneNode::GROUP);
}

TEST_P(ModelFileParts, NodesHaveTheirTransformations){
  //translation
  EXPECT_TRUE(isNear(model_->findNode("body")->getTrafo()*Vec3f(0, 0, 0), 1, 2, 3));
  //translation and rotation (90 degrees around y: x becomes -z), under body
  EXPECT_TRUE(isNear(model_->findNode("socket_left")->getTrafo()*Vec3f(1, 0, 0), 0.5f, 0, -1));
  EXPECT_TRUE(isNear(model_->findNode("socket_left")->getWorldTrafo()*Vec3f(1, 0, 0), 1.5f, 2, 2));
  //scale, under body
  EXPECT_TRUE(isNear(model_->findNode("tip")->getWorldTrafo()*Vec3f(1, 0, 0), 3, 2, 3));
  //a matrix
  EXPECT_TRUE(isNear(model_->findNode("arm")->getWorldTrafo()*Vec3f(0, 0, 0), 1, 3, 3));
}

TEST_P(ModelFileParts, NodesShareTheirMeshes){
  ASSERT_EQ(model_->getMeshes().size(), 3u);
  Model* body = static_cast<Model*>(model_->findNode("body"));
  Model* copy = static_cast<Model*>(model_->findNode("body_copy"));
  ASSERT_EQ(body->getType(), SceneNode::MESH);
  EXPECT_EQ(body->getMesh(), model_->getMeshes()[0].get());
  EXPECT_EQ(copy->getMesh(), model_->getMeshes()[0].get());
  EXPECT_EQ(static_cast<Model*>(model_->findNode("arm"))->getMesh(), model_->getMeshes()[1].get());
  EXPECT_EQ(static_cast<Model*>(model_->findNode("tip"))->getMesh(), model_->getMeshes()[2].get());
  EXPECT_EQ(model_->getMeshes()[0]->getName(), "box");
  EXPECT_EQ(model_->getMeshes()[1]->getName(), "strip");
  EXPECT_EQ(model_->getMeshes()[2]->getName(), "mesh 2");
}

TEST_P(ModelFileParts, PrimitivesAreSubmeshesWithTheirAttributes){
  const Mesh& box = *model_->getMeshes()[0];
  EXPECT_EQ(box.getNumVertices(), 8);
  EXPECT_EQ(box.getNumTriangles(), 4);
  ASSERT_EQ(box.getSubMeshes().size(), 2u);
  EXPECT_EQ(box.getSubMeshes()[0].offset, 0);
  EXPECT_EQ(box.getSubMeshes()[0].count, 6);
  EXPECT_EQ(box.getSubMeshes()[1].offset, 6);
  EXPECT_EQ(box.getSubMeshes()[1].count, 6);
  //16-bit and 8-bit indices, the second primitive's after the first's vertices
  EXPECT_TRUE(hasTriangle(box, 0, 0, 1, 2));
  EXPECT_TRUE(hasTriangle(box, 1, 0, 2, 3));
  EXPECT_TRUE(hasTriangle(box, 2, 4, 5, 6));
  EXPECT_TRUE(isNear(box.getVertex(5), 0, 0, 1));
  //the normals as they are
  ASSERT_EQ(box.getNumNormals(), 8);
  EXPECT_TRUE(isNear(box.getStoredNormal(0), 0, 0, 1));
  EXPECT_TRUE(isNear(box.getStoredNormal(4), -1, 0, 0));
  //texture coordinates, v from the top of the image; one more for the
  //primitive without them
  ASSERT_EQ(box.getNumTexCoords(), 5);
  EXPECT_TRUE(isNear(box.getTexCoord(0), 0, 1, -1));
  EXPECT_TRUE(isNear(box.getTexCoord(2), 1, 0, -1));
  EXPECT_EQ(box.getTriangleIndices(2)->texdata[0], 4);
  //colours (normalized bytes), white for the primitive without them
  ASSERT_EQ(box.getNumColors(), 8);
  Vec4f white = box.getColor(0);
  EXPECT_FLOAT_EQ(white.x+white.y+white.z+white.w, 4);
  Vec4f red = box.getColor(4);
  EXPECT_FLOAT_EQ(red.x, 1);
  EXPECT_FLOAT_EQ(red.y, 0);
  EXPECT_FLOAT_EQ(red.w, 1);
  EXPECT_NEAR(box.getColor(7).w, 0.2f, 1e-6);
}

TEST_P(ModelFileParts, StripsAndFansBecomeTriangles){
  const Mesh& strip = *model_->getMeshes()[1];
  EXPECT_EQ(strip.getNumVertices(), 4);
  ASSERT_EQ(strip.getNumTriangles(), 2);
  //the second triangle turned, so that both face +z
  EXPECT_TRUE(hasTriangle(strip, 0, 0, 1, 2));
  EXPECT_TRUE(hasTriangle(strip, 1, 2, 1, 3));
  //without normals: one per triangle (flat)
  ASSERT_EQ(strip.getNumNormals(), 2);
  EXPECT_TRUE(isNear(strip.getStoredNormal(0), 0, 0, 1));
  EXPECT_TRUE(isNear(strip.getStoredNormal(1), 0, 0, 1));
  EXPECT_EQ(strip.getTriangleIndices(1)->normdata[0], 1);
  //normalized 16-bit texture coordinates
  ASSERT_EQ(strip.getNumTexCoords(), 4);
  EXPECT_TRUE(isNear(strip.getTexCoord(3), 1, 1, -1));
  EXPECT_EQ(strip.getNumColors(), 0);

  const Mesh& fan = *model_->getMeshes()[2];
  ASSERT_EQ(fan.getNumTriangles(), 2);
  EXPECT_TRUE(hasTriangle(fan, 0, 1, 2, 0));
  EXPECT_TRUE(hasTriangle(fan, 1, 2, 3, 0));
  EXPECT_EQ(fan.getNumTexCoords(), 0);
}

TEST_P(ModelFileParts, MaterialsAreMappedOntoTheEngines){
  const std::vector<Material*>& materials = model_->getMeshes()[0]->getMaterials();
  ASSERT_EQ(materials.size(), 3u);
  EXPECT_EQ(materials[0], nullptr);
  EXPECT_EQ(model_->getMeshes()[0]->getSubMeshes()[0].material, 1);
  EXPECT_EQ(model_->getMeshes()[0]->getSubMeshes()[1].material, 2);

  //a texture, an alpha mask, double-sided; the factor from linear to sRGB
  const Material& picture = *materials[1];
  EXPECT_EQ(picture.getName(), "picture");
  EXPECT_NEAR(picture.getDiffuse().r, kHalfInSrgb, 1e-4);
  EXPECT_NEAR(picture.getDiffuse().b, kHalfInSrgb, 1e-4);
  EXPECT_EQ(picture.getAlphaMode(), Material::AlphaMode::Mask);
  EXPECT_FLOAT_EQ(picture.getAlphaCutoff(), 0.25f);
  EXPECT_TRUE(picture.isDoubleSided());
  EXPECT_FALSE(picture.isTransparent());
  //rough: the broadest highlight
  EXPECT_FLOAT_EQ(picture.getPower(), 1);
  //and the metallic-roughness material itself
  EXPECT_EQ(picture.getShading(), Material::Shading::MetallicRoughness);
  EXPECT_FLOAT_EQ(picture.getMetallic(), 0);
  EXPECT_FLOAT_EQ(picture.getRoughness(), 1);
  EXPECT_EQ(picture.getMetallicRoughnessTexture(), nullptr);
  EXPECT_EQ(picture.getNormalTexture(), nullptr);
  ASSERT_NE(picture.getDiffuseTexture(), nullptr);
  const Image* image = picture.getDiffuseTexture()->getImage();
  ASSERT_NE(image, nullptr);
  EXPECT_EQ(image->getWidth(), 3u);
  EXPECT_EQ(image->getHeight(), 2u);
  //the top left pixel of image.png's picture is red
  EXPECT_EQ(image->getData()[0], 255);
  EXPECT_EQ(image->getData()[1], 0);

  //blended, metallic: the specular colour is the base colour
  const Material& metal = *materials[2];
  EXPECT_EQ(metal.getAlphaMode(), Material::AlphaMode::Blend);
  EXPECT_TRUE(metal.isTransparent());
  EXPECT_FLOAT_EQ(metal.getOpacity(), 0.5f);
  EXPECT_FLOAT_EQ(metal.getDiffuse().r, 1);
  EXPECT_FLOAT_EQ(metal.getSpecular().r, 1);
  EXPECT_FLOAT_EQ(metal.getSpecular().g, 0);
  EXPECT_FALSE(metal.isDoubleSided());
  EXPECT_EQ(metal.getDiffuseTexture(), nullptr);
  //roughness 0.5: alpha 0.25, (2/alpha^2-2)/4
  EXPECT_FLOAT_EQ(metal.getPower(), 7.5f);
  EXPECT_FLOAT_EQ(metal.getMetallic(), 1);
  EXPECT_FLOAT_EQ(metal.getRoughness(), 0.5f);

  //no material: glTF's default, white and opaque
  const std::vector<Material*>& stripMaterials = model_->getMeshes()[1]->getMaterials();
  ASSERT_EQ(stripMaterials.size(), 2u);
  EXPECT_EQ(stripMaterials[1]->getAlphaMode(), Material::AlphaMode::Opaque);
  EXPECT_FLOAT_EQ(stripMaterials[1]->getDiffuse().g, 1);
  EXPECT_FALSE(stripMaterials[1]->isTransparent());
  //glTF's default: a rough metal
  EXPECT_TRUE(stripMaterials[1]->isMetallicRoughness());
  EXPECT_FLOAT_EQ(stripMaterials[1]->getMetallic(), 1);
  EXPECT_FLOAT_EQ(stripMaterials[1]->getRoughness(), 1);
}

TEST(ModelFiles, MetallicRoughnessMaterialsKeepTheirTexturesAndFactors){
  std::string error;
  std::unique_ptr<ModelFile> model = load("pbr.gltf", &error);
  ASSERT_NE(model, nullptr) << error;
  EXPECT_TRUE(model->getWarnings().empty());
  const std::vector<Material*>& materials = model->getMeshes()[0]->getMaterials();
  ASSERT_EQ(materials.size(), 2u);
  const Material& pbr = *materials[1];
  EXPECT_EQ(pbr.getName(), "pbr");
  EXPECT_EQ(pbr.getShading(), Material::Shading::MetallicRoughness);
  EXPECT_NEAR(pbr.getDiffuse().g, kHalfInSrgb, 1e-4);
  EXPECT_FLOAT_EQ(pbr.getMetallic(), 0.25f);
  EXPECT_FLOAT_EQ(pbr.getRoughness(), 0.75f);
  EXPECT_FLOAT_EQ(pbr.getNormalScale(), 0.5f);
  EXPECT_FLOAT_EQ(pbr.getOcclusionStrength(), 0.8f);
  //the emissive factor in sRGB, its strength (KHR_materials_emissive_strength) apart
  EXPECT_FLOAT_EQ(pbr.getEmissive().r, 1);
  EXPECT_NEAR(pbr.getEmissive().g, kHalfInSrgb, 1e-4);
  EXPECT_FLOAT_EQ(pbr.getEmissive().b, 0);
  EXPECT_FLOAT_EQ(pbr.getEmissiveStrength(), 2);
  //the textures, each its own image
  ASSERT_NE(pbr.getMetallicRoughnessTexture(), nullptr);
  ASSERT_NE(pbr.getNormalTexture(), nullptr);
  ASSERT_NE(pbr.getOcclusionTexture(), nullptr);
  ASSERT_NE(pbr.getEmissiveTexture(), nullptr);
  EXPECT_EQ(pbr.getDiffuseTexture(), nullptr);
  const Image* normals = pbr.getNormalTexture()->getImage();
  ASSERT_NE(normals, nullptr);
  EXPECT_EQ(normals->getWidth(), 3u);
  EXPECT_EQ(normals->getHeight(), 1u);
  const Image* metalRoughness = pbr.getMetallicRoughnessTexture()->getImage();
  ASSERT_NE(metalRoughness, nullptr);
  EXPECT_EQ(metalRoughness->getWidth(), 2u);
  EXPECT_NE(pbr.getOcclusionTexture(), pbr.getEmissiveTexture());
}

TEST_P(ModelFileParts, BoundsCoverTheTransformedMeshes){
  BBox bounds = model_->getBounds();
  EXPECT_TRUE(isNear(bounds.getMin(), -1, 0, 0));
  EXPECT_TRUE(isNear(bounds.getMax(), 3, 4, 4));
}

TEST_P(ModelFileParts, ClonesCopyTheHierarchyAndShareTheMeshes){
  std::unique_ptr<SceneNode> copy(model_->findNode("body")->clone());
  EXPECT_EQ(copy->getParent(), nullptr);
  EXPECT_NE(copy->getID(), model_->findNode("body")->getID());
  ASSERT_EQ(copy->getType(), SceneNode::MESH);
  EXPECT_EQ(static_cast<Model*>(copy.get())->getMesh(), model_->getMeshes()[0].get());
  SceneNode* tip = copy->findNode("tip");
  ASSERT_NE(tip, nullptr);
  EXPECT_NE(tip, model_->findNode("tip"));
  EXPECT_EQ(static_cast<Model*>(tip)->getMesh(), model_->getMeshes()[2].get());
  //the copy can be moved elsewhere on its own
  copy->setTrafo(Matrix(Matrix::Identity));
  EXPECT_TRUE(isNear(tip->getWorldTrafo()*Vec3f(1, 0, 0), 2, 0, 0));
  EXPECT_TRUE(isNear(model_->findNode("tip")->getWorldTrafo()*Vec3f(1, 0, 0), 3, 2, 3));
}

TEST(SceneNodes, ChildrenMoveBetweenParents){
  SceneNode a(SceneNode::GROUP), b(SceneNode::GROUP);
  SceneNode* child = new SceneNode(SceneNode::GROUP);
  child->setName("child");
  a.addChild(child);
  EXPECT_EQ(a.findNode("child"), child);
  b.addChild(child);
  EXPECT_TRUE(a.getChildren().empty());
  EXPECT_EQ(child->getParent(), &b);
  EXPECT_TRUE(b.removeChild(child));
  EXPECT_FALSE(b.removeChild(child));
  EXPECT_EQ(child->getParent(), nullptr);
  delete child;
}

TEST(ModelFiles, ObjFilesBecomeOneNode){
  std::filesystem::path dir = std::filesystem::temp_directory_path()/"cge_test_modelfile";
  std::filesystem::create_directories(dir);
  {
    std::ofstream out(dir/"tri.obj", std::ios::binary);
    out << "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n";
  }
  std::string error;
  std::unique_ptr<ModelFile> model = ModelFile::load((dir/"tri.obj").string(), &error);
  ASSERT_NE(model, nullptr) << error;
  ASSERT_EQ(model->getRoot()->getChildren().size(), 1u);
  EXPECT_EQ(model->getRoot()->getChildren()[0]->getName(), "tri");
  ASSERT_EQ(model->getMeshes().size(), 1u);
  EXPECT_EQ(model->getMeshes()[0]->getNumTriangles(), 1);
}

TEST(ModelFiles, ErrorsGiveAReason){
  std::string error;
  EXPECT_EQ(load("nothere.gltf", &error), nullptr);
  EXPECT_EQ(error, "file not found");
  EXPECT_EQ(load("broken.gltf", &error), nullptr);
  EXPECT_EQ(error, "invalid JSON");
  EXPECT_EQ(load("missing_buffer.gltf", &error), nullptr);
  EXPECT_EQ(error, "a file was not found");
  EXPECT_EQ(load("truncated.glb", &error), nullptr);
  EXPECT_NE(error, "");
  //an index past the vertices
  EXPECT_EQ(load("bad_index.gltf", &error), nullptr);
  EXPECT_EQ(error, "invalid glTF: the data is too short");
  EXPECT_EQ(load("draco.gltf", &error), nullptr);
  EXPECT_EQ(error, "the extension KHR_draco_mesh_compression is required, which is not supported");
  EXPECT_EQ(ModelFile::load("image.png", &error), nullptr);
  EXPECT_NE(error.find("unknown file type"), std::string::npos);
}

TEST(ModelFiles, LinesAreLeftOut){
  std::string error;
  std::unique_ptr<ModelFile> model = load("lines.gltf", &error);
  ASSERT_NE(model, nullptr) << error;
  ASSERT_EQ(model->getMeshes().size(), 1u);
  EXPECT_EQ(model->getMeshes()[0]->getNumTriangles(), 1);
  EXPECT_EQ(model->getMeshes()[0]->getSubMeshes().size(), 1u);
  ASSERT_EQ(model->getWarnings().size(), 1u);
  EXPECT_EQ(model->getWarnings()[0], "mesh mixed: points and lines are left out");
}

TEST(ModelFiles, BrokenImagesAreLeftOut){
  for (const char* name : {"garbage", "broken_png", "broken_jpeg"}){
    std::string error;
    std::unique_ptr<ModelFile> model = load(std::string("bad_image_")+name+".gltf", &error);
    ASSERT_NE(model, nullptr) << name << ": " << error;
    EXPECT_EQ(model->getMeshes()[0]->getMaterials()[1]->getDiffuseTexture(), nullptr) << name;
    ASSERT_EQ(model->getWarnings().size(), 1u) << name;
    EXPECT_EQ(model->getWarnings()[0], std::string("image ")+name+".png could not be decoded (PNG and JPEG are supported)");
  }
}

//! every .gltf and .glb below the folder in the environment variable
//! CGE_GLTF_SAMPLES (e.g. a checkout of Khronos' glTF-Sample-Assets/Models):
//! they load or say why, and the counts are printed
TEST(ModelFiles, SampleModelsLoadOrSayWhy){
  const char* samples = getenv("CGE_GLTF_SAMPLES");
  if (!samples)
    GTEST_SKIP() << "CGE_GLTF_SAMPLES isn't set";
  int loaded = 0, failed = 0, warned = 0;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(samples)){
    std::string ext = entry.path().extension().string();
    if (ext != ".gltf" && ext != ".glb")
      continue;
    std::string error;
    //UTF-8 (a sample has non-ASCII characters in its name)
    auto utf8 = [](const std::filesystem::path& path){
      std::u8string text = path.generic_u8string();
      return std::string(text.begin(), text.end());
    };
    std::unique_ptr<ModelFile> model = ModelFile::load(utf8(entry.path()), &error);
    std::string name = utf8(std::filesystem::relative(entry.path(), samples));
    if (!model){
      ++failed;
      std::cout << "FAILED  " << name << ": " << error << "\n";
      continue;
    }
    ++loaded;
    int triangles = 0;
    for (const auto& mesh : model->getMeshes())
      triangles += mesh->getNumTriangles();
    if (!model->getWarnings().empty())
      ++warned;
    std::cout << "loaded  " << name << ": " << model->getMeshes().size() << " meshes, " << triangles << " triangles";
    for (const std::string& warning : model->getWarnings())
      std::cout << "; " << warning;
    std::cout << "\n";
  }
  std::cout << loaded << " loaded (" << warned << " with warnings), " << failed << " failed\n";
}
