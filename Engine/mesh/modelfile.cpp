// Model files: glTF 2.0 read with cgltf, and the formats Mesh reads.

#include "modelfile.h"

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "model.h"
#include "../image/image.h"
#include "../image/loader.h"
#include "../io/Tracing.h"
#include "../platform/assets.h"
#include "../renderer/drawcontext.h"
#include "../renderer/material.h"
#include "../renderer/scenenode.h"
#include "../system/file.h"

TR_CHANNEL(CGE_ModelFile);

namespace CGE{

namespace{

//! glTF's colours are linear; the engine's (like the textures') are sRGB
float linearToSrgb(float c){
  if (c <= 0.0f)
    return 0.0f;
  if (c >= 1.0f)
    return 1.0f;
  if (c <= 0.0031308f)
    return 12.92f*c;
  return 1.055f*powf(c, 1.0f/2.4f) - 0.055f;
}

//! the extensions the reader handles (the specular-glossiness material as far as its diffuse part)
bool isSupported(const char* name){
  return strcmp(name, "KHR_mesh_quantization") == 0 || strcmp(name, "KHR_materials_pbrSpecularGlossiness") == 0;
}

//! the extensions without which the meshes can't be read (compressed geometry)
/*! Files that require other extensions are read without them: they change
    how things look (materials, lights, texture formats), which the engine's
    mapping of the materials only approximates anyway. */
bool isNeeded(const char* name){
  return strcmp(name, "KHR_draco_mesh_compression") == 0 || strcmp(name, "KHR_meshopt_compression") == 0 ||
    strcmp(name, "EXT_meshopt_compression") == 0;
}

//! files for cgltf (the external buffers) are read as assets
cgltf_result readAsset(const cgltf_memory_options* memory, const cgltf_file_options* file, const char* path,
  cgltf_size* size, void** data){
  std::vector<uint8> bytes;
  if (!Assets::load(path, bytes))
    return cgltf_result_file_not_found;
  if (*size != 0 && bytes.size() < *size)
    return cgltf_result_data_too_short;
  void* copy = malloc(bytes.empty() ? 1 : bytes.size());
  if (!copy)
    return cgltf_result_out_of_memory;
  if (!bytes.empty())
    memcpy(copy, bytes.data(), bytes.size());
  *size = bytes.size();
  *data = copy;
  return cgltf_result_success;
}

void releaseAsset(const cgltf_memory_options* memory, const cgltf_file_options* file, void* data){
  free(data);
}

const char* resultText(cgltf_result result){
  switch (result){
    case cgltf_result_success: return "success";
    case cgltf_result_data_too_short: return "the data is too short";
    case cgltf_result_unknown_format: return "unknown format";
    case cgltf_result_invalid_json: return "invalid JSON";
    case cgltf_result_invalid_gltf: return "invalid glTF";
    case cgltf_result_invalid_options: return "invalid options";
    case cgltf_result_file_not_found: return "a file was not found";
    case cgltf_result_io_error: return "a file could not be read";
    case cgltf_result_out_of_memory: return "out of memory";
    case cgltf_result_legacy_gltf: return "glTF 1.0 is not supported";
    default: return "unknown error";
  }
}

//! the image type from the first bytes (glTF's images are PNG or JPEG)
ImageLoader::Type imageType(const uint8* data, size_t size){
  if (size >= 4 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G')
    return ImageLoader::PNG;
  if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF)
    return ImageLoader::JPG;
  return ImageLoader::UNKNOWN;
}

}

//! reads a glTF file into a ModelFile
class GltfReader{
public:
  GltfReader(ModelFile& model, const std::string& path) : mModel(model), mPath(path), mData(nullptr){
    memset(&mOptions, 0, sizeof(mOptions));
    mOptions.file.read = readAsset;
    mOptions.file.release = releaseAsset;
  }

  ~GltfReader(){
    if (mData)
      cgltf_free(mData);
  }

  bool read(std::string& error){
    if (!Assets::load(mPath, mBytes)){
      error = "file not found";
      return false;
    }
    //the data of a .glb stays in mBytes, which outlives mData
    cgltf_result result = cgltf_parse(&mOptions, mBytes.data(), mBytes.size(), &mData);
    if (result == cgltf_result_success)
      result = cgltf_load_buffers(&mOptions, mData, mPath.c_str());
    if (result != cgltf_result_success){
      error = resultText(result);
      return false;
    }
    //e.g. indices past the vertices, accessors past their buffers
    result = cgltf_validate(mData);
    if (result != cgltf_result_success){
      error = std::string("invalid glTF: ")+resultText(result);
      return false;
    }
    for (cgltf_size i = 0; i < mData->extensions_required_count; ++i){
      const char* name = mData->extensions_required[i];
      if (isNeeded(name)){
        error = std::string("the extension ")+name+" is required, which is not supported";
        return false;
      }
    }
    for (cgltf_size i = 0; i < mData->extensions_used_count; ++i){
      const char* name = mData->extensions_used[i];
      if (!isSupported(name))
        warn(std::string("the extension ")+name+" is left out");
    }
    if (mData->animations_count > 0)
      warn("animations are left out");
    if (mData->skins_count > 0)
      warn("skins are left out (the meshes are shown as they are stored)");
    if (mData->cameras_count > 0)
      warn("cameras are left out");

    for (cgltf_size i = 0; i < mData->meshes_count; ++i)
      mModel.mMeshes.push_back(readMesh(mData->meshes[i]));

    mModel.mRoot.reset(new SceneNode(SceneNode::GROUP));
    const cgltf_scene* scene = mData->scene;
    if (!scene && mData->scenes_count > 0)
      scene = &mData->scenes[0];
    if (scene){
      for (cgltf_size i = 0; i < scene->nodes_count; ++i)
        mModel.mRoot->addChild(readNode(scene->nodes[i]));
    }
    else{
      //no scene: the nodes without a parent
      for (cgltf_size i = 0; i < mData->nodes_count; ++i){
        if (!mData->nodes[i].parent)
          mModel.mRoot->addChild(readNode(&mData->nodes[i]));
      }
    }
    return true;
  }

private:
  //! a message once per file
  void warn(const std::string& message){
    TR_USE(CGE_ModelFile);
    if (mWarned.insert(message).second){
      TR_WARN("%s: %s", mPath.c_str(), message.c_str());
      mModel.mWarnings.push_back(message);
    }
  }

  std::string nameOf(const char* name, const char* kind, size_t index){
    if (name && name[0])
      return name;
    return std::string(kind)+" "+std::to_string(index);
  }

  //! the floats of an accessor, components per element; empty if it can't be read
  std::vector<float> unpack(const cgltf_accessor* accessor, size_t components){
    std::vector<float> values;
    if (cgltf_num_components(accessor->type) != components)
      return values;
    values.resize(accessor->count*components);
    if (cgltf_accessor_unpack_floats(accessor, values.data(), values.size()) != values.size())
      values.clear();
    return values;
  }

  const cgltf_accessor* attribute(const cgltf_primitive& prim, cgltf_attribute_type type, int index = 0){
    for (cgltf_size i = 0; i < prim.attributes_count; ++i){
      if (prim.attributes[i].type == type && prim.attributes[i].index == index)
        return prim.attributes[i].data;
    }
    return nullptr;
  }

  //! the image as a texture source shared by the materials, or nullptr if it can't be read
  std::shared_ptr<MaterialTexture> texture(const cgltf_texture* tex){
    const cgltf_image* image = tex->image;
    if (!image){
      warn("texture "+nameOf(tex->name, "texture", tex-mData->textures)+" has no image in PNG or JPEG (KTX2 and WebP are not supported)");
      return nullptr;
    }
    size_t index = image-mData->images;
    auto found = mImages.find(index);
    if (found != mImages.end())
      return found->second;
    std::shared_ptr<MaterialTexture> result;
    std::string name = nameOf(image->name ? image->name : image->uri, "image", index);
    if (name.compare(0, 5, "data:") == 0)
      name = nameOf(nullptr, "image", index);
    std::vector<uint8> bytes;
    const uint8* data = nullptr;
    size_t size = 0;
    if (image->buffer_view){
      const cgltf_buffer_view* view = image->buffer_view;
      if (view->buffer->data && view->offset+view->size <= view->buffer->size){
        data = (const uint8*)view->buffer->data+view->offset;
        size = view->size;
      }
    }
    else if (image->uri && strncmp(image->uri, "data:", 5) == 0){
      const char* comma = strchr(image->uri, ',');
      if (comma && comma-image->uri >= 7 && strncmp(comma-7, ";base64", 7) == 0){
        const char* base64 = comma+1;
        size_t length = strlen(base64);
        size_t padding = 0;
        while (padding < 2 && length > padding && base64[length-1-padding] == '=')
          ++padding;
        size = length*3/4-padding;
        void* decoded = nullptr;
        if (size > 0 && cgltf_load_buffer_base64(&mOptions, size, base64, &decoded) == cgltf_result_success){
          bytes.assign((uint8*)decoded, (uint8*)decoded+size);
          free(decoded);
          data = bytes.data();
        }
      }
    }
    else if (image->uri){
      std::string uri = image->uri;
      cgltf_decode_uri(&uri[0]);
      uri.resize(strlen(uri.c_str()));
      std::string dir = mPath.substr(0, mPath.find_last_of("/\\")+1);
      if (Assets::load(dir+uri, bytes)){
        data = bytes.data();
        size = bytes.size();
      }
      else
        warn("image "+name+" was not found");
    }
    if (data){
      ImageLoader::Type type = imageType(data, size);
      std::unique_ptr<Image> decoded;
      if (type != ImageLoader::UNKNOWN)
        decoded.reset(ImageLoader::load((void*)data, (unsigned)size, type));
      if (decoded && decoded->getWidth() > 0 && decoded->getHeight() > 0)
        result = std::make_shared<MaterialTexture>(std::move(decoded), mPath+": "+name);
      else
        warn("image "+name+" could not be decoded (PNG and JPEG are supported)");
    }
    mImages[index] = result;
    return result;
  }

  //! the material (nullptr: glTF's default material)
  Material* readMaterial(const cgltf_material* mat){
    Material* result = new Material(mat ? nameOf(mat->name, "material", mat-mData->materials) : "default");
    float base[4] = {1, 1, 1, 1};
    float metallic = 1;
    float roughness = 1;
    const cgltf_texture_view* baseTexture = nullptr;
    if (mat && mat->has_pbr_metallic_roughness){
      memcpy(base, mat->pbr_metallic_roughness.base_color_factor, sizeof(base));
      metallic = mat->pbr_metallic_roughness.metallic_factor;
      roughness = mat->pbr_metallic_roughness.roughness_factor;
      baseTexture = &mat->pbr_metallic_roughness.base_color_texture;
    }
    else if (mat && mat->has_pbr_specular_glossiness){
      memcpy(base, mat->pbr_specular_glossiness.diffuse_factor, sizeof(base));
      metallic = 0;
      roughness = 1-mat->pbr_specular_glossiness.glossiness_factor;
      baseTexture = &mat->pbr_specular_glossiness.diffuse_texture;
    }
    Color diffuse(linearToSrgb(base[0]), linearToSrgb(base[1]), linearToSrgb(base[2]), 1);
    result->setDiffuse(diffuse);
    result->setOpacity(base[3]);
    //a little light from all sides, so that the sides away from the lights aren't black
    result->setAmbient(Color(0.2f, 0.2f, 0.2f, 1));
    //the specular colour of metals is their colour, that of other materials 4% grey
    Color specular;
    float dielectric = linearToSrgb(0.04f);
    specular.r = dielectric+(diffuse.r-dielectric)*metallic;
    specular.g = dielectric+(diffuse.g-dielectric)*metallic;
    specular.b = dielectric+(diffuse.b-dielectric)*metallic;
    result->setSpecular(specular);
    //the roughness as a Phong exponent: Blinn-Phong's equivalent of GGX's
    //alpha = roughness^2 (2/alpha^2 - 2), a quarter of it for Phong
    float alpha = roughness*roughness;
    float shininess = alpha > 0.001f ? (2.0f/(alpha*alpha)-2.0f)/4.0f : 128.0f;
    if (shininess < 1.0f)
      shininess = 1.0f;
    if (shininess > 128.0f)
      shininess = 128.0f;
    result->setPower(shininess);
    if (mat){
      switch (mat->alpha_mode){
        case cgltf_alpha_mode_mask:
          result->setAlphaMode(Material::AlphaMode::Mask);
          result->setAlphaCutoff(mat->alpha_cutoff);
          break;
        case cgltf_alpha_mode_blend:
          result->setAlphaMode(Material::AlphaMode::Blend);
          break;
        default:
          result->setAlphaMode(Material::AlphaMode::Opaque);
          break;
      }
      result->setDoubleSided(mat->double_sided != 0);
    }
    else
      result->setAlphaMode(Material::AlphaMode::Opaque);
    if (baseTexture && baseTexture->texture){
      if (baseTexture->texcoord != 0)
        warn("material "+result->getName()+": the base colour texture uses texture coordinates "+std::to_string(baseTexture->texcoord)+"; only the first are read");
      else
        result->setDiffuseTexture(texture(baseTexture->texture));
    }
    return result;
  }

  //! the triangles' indices of a primitive, or false if it has none
  bool triangles(const cgltf_primitive& prim, size_t count, std::vector<uint32_t>& result){
    std::vector<uint32_t> indices;
    if (prim.indices){
      indices.resize(prim.indices->count);
      if (cgltf_accessor_unpack_indices(prim.indices, indices.data(), sizeof(uint32_t), indices.size()) != indices.size())
        return false;
    }
    else{
      indices.resize(count);
      for (size_t i = 0; i < count; ++i)
        indices[i] = (uint32_t)i;
    }
    for (uint32_t index : indices){
      if (index >= count)
        return false;
    }
    result.clear();
    switch (prim.type){
      case cgltf_primitive_type_triangles:
        for (size_t i = 0; i+2 < indices.size(); i += 3)
          result.insert(result.end(), {indices[i], indices[i+1], indices[i+2]});
        break;
      case cgltf_primitive_type_triangle_strip:
        //every other triangle turned, so that all face the same way
        for (size_t i = 0; i+2 < indices.size(); ++i){
          if (i%2 == 0)
            result.insert(result.end(), {indices[i], indices[i+1], indices[i+2]});
          else
            result.insert(result.end(), {indices[i+1], indices[i], indices[i+2]});
        }
        break;
      case cgltf_primitive_type_triangle_fan:
        for (size_t i = 1; i+1 < indices.size(); ++i)
          result.insert(result.end(), {indices[i], indices[i+1], indices[0]});
        break;
      default:
        return false;
    }
    return true;
  }

  std::unique_ptr<Mesh> readMesh(const cgltf_mesh& gmesh){
    std::unique_ptr<Mesh> mesh(new Mesh());
    std::string name = nameOf(gmesh.name, "mesh", &gmesh-mData->meshes);
    mesh->setName(name);
    mesh->setFilename(mPath);
    //texture coordinates and colours for all primitives if one has them
    bool hasTexCoords = false;
    bool hasColors = false;
    for (cgltf_size p = 0; p < gmesh.primitives_count; ++p){
      hasTexCoords |= attribute(gmesh.primitives[p], cgltf_attribute_type_texcoord) != nullptr;
      hasColors |= attribute(gmesh.primitives[p], cgltf_attribute_type_color) != nullptr;
    }
    std::map<const cgltf_material*, int> materials;
    int noTexCoord = -1;
    for (cgltf_size p = 0; p < gmesh.primitives_count; ++p){
      const cgltf_primitive& prim = gmesh.primitives[p];
      if (prim.type != cgltf_primitive_type_triangles && prim.type != cgltf_primitive_type_triangle_strip &&
        prim.type != cgltf_primitive_type_triangle_fan){
        warn("mesh "+name+": points and lines are left out");
        continue;
      }
      if (prim.targets_count > 0)
        warn("morph targets are left out");
      const cgltf_accessor* positionAcc = attribute(prim, cgltf_attribute_type_position);
      std::vector<float> positions;
      if (positionAcc)
        positions = unpack(positionAcc, 3);
      std::vector<uint32_t> tris;
      size_t count = positions.size()/3;
      if (positions.empty() || !triangles(prim, count, tris)){
        warn("mesh "+name+": a primitive without readable positions or indices is left out");
        continue;
      }
      std::vector<float> normals, texCoords, colors;
      int colorComponents = 4;
      if (const cgltf_accessor* acc = attribute(prim, cgltf_attribute_type_normal))
        normals = unpack(acc, 3);
      if (const cgltf_accessor* acc = attribute(prim, cgltf_attribute_type_texcoord))
        texCoords = unpack(acc, 2);
      if (const cgltf_accessor* acc = attribute(prim, cgltf_attribute_type_color)){
        colorComponents = (int)cgltf_num_components(acc->type);
        colors = unpack(acc, colorComponents);
      }
      if (normals.size() != count*3)
        normals.clear();
      if (texCoords.size() != count*2)
        texCoords.clear();
      if (colors.size() != count*colorComponents)
        colors.clear();

      int vertexBase = mesh->getNumVertices();
      int normalBase = mesh->getNumNormals();
      int texBase = mesh->getNumTexCoords();
      for (size_t i = 0; i < count; ++i)
        mesh->addVertex(positions[3*i], positions[3*i+1], positions[3*i+2]);
      if (hasColors){
        for (size_t i = 0; i < count; ++i){
          if (colors.empty())
            mesh->addColor(1, 1, 1, 1);
          else{
            const float* c = &colors[colorComponents*i];
            mesh->addColor(linearToSrgb(c[0]), linearToSrgb(c[1]), linearToSrgb(c[2]), colorComponents == 4 ? c[3] : 1.0f);
          }
        }
      }
      //glTF's v starts at the top of the image; addTexCoord takes it from the bottom (as OBJ)
      for (size_t i = 0; i < texCoords.size()/2; ++i)
        mesh->addTexCoord(texCoords[2*i], 1.0f-texCoords[2*i+1]);
      if (hasTexCoords && texCoords.empty() && noTexCoord < 0){
        noTexCoord = mesh->getNumTexCoords();
        mesh->addTexCoord(0, 1);
      }
      //without normals, each triangle gets its own (flat shading)
      if (!normals.empty()){
        for (size_t i = 0; i < count; ++i)
          mesh->addNormal(normals[3*i], normals[3*i+1], normals[3*i+2]);
      }
      else{
        for (size_t t = 0; t < tris.size(); t += 3){
          Vec3f a(&positions[3*tris[t]]), b(&positions[3*tris[t+1]]), c(&positions[3*tris[t+2]]);
          Vec3f n = (b-a).cross(c-a);
          if (n.length() > 0)
            n.normalize();
          mesh->addNormal(n.x, n.y, n.z);
        }
      }
      auto found = materials.find(prim.material);
      int materialIndex;
      if (found == materials.end()){
        materialIndex = mesh->addMaterial(readMaterial(prim.material));
        materials[prim.material] = materialIndex;
      }
      else
        materialIndex = found->second;
      int firstTriangle = mesh->getNumTriangles();
      for (size_t t = 0; t < tris.size(); t += 3){
        int v[3], tc[3], n[3];
        for (int k = 0; k < 3; ++k){
          v[k] = vertexBase+(int)tris[t+k];
          tc[k] = texCoords.empty() ? noTexCoord : texBase+(int)tris[t+k];
          n[k] = normals.empty() ? normalBase+(int)(t/3) : normalBase+(int)tris[t+k];
        }
        mesh->addTriangle(v[0], v[1], v[2], tc[0], tc[1], tc[2], n[0], n[1], n[2]);
      }
      mesh->addSubMesh(firstTriangle, mesh->getNumTriangles()-firstTriangle, materialIndex);
    }
    return mesh;
  }

  SceneNode* readNode(const cgltf_node* gnode){
    SceneNode* node;
    if (gnode->mesh)
      node = new Model(mModel.mMeshes[gnode->mesh-mData->meshes].get());
    else
      node = new SceneNode(SceneNode::GROUP);
    if (gnode->name)
      node->setName(gnode->name);
    if (gnode->light)
      warn("lights are left out");
    float matrix[16];
    cgltf_node_transform_local(gnode, matrix);
    node->setTrafo(Matrix(matrix));
    for (cgltf_size i = 0; i < gnode->children_count; ++i)
      node->addChild(readNode(gnode->children[i]));
    return node;
  }

  ModelFile& mModel;
  std::string mPath;
  std::vector<uint8> mBytes;
  cgltf_options mOptions;
  cgltf_data* mData;
  std::map<size_t, std::shared_ptr<MaterialTexture>> mImages;
  std::set<std::string> mWarned;
};

ModelFile::ModelFile(){
}

ModelFile::~ModelFile(){
  //the nodes refer to the meshes
  mRoot.reset();
  mMeshes.clear();
}

std::unique_ptr<ModelFile> ModelFile::load(const std::string& path, std::string* error){
  TR_USE(CGE_ModelFile);
  std::unique_ptr<ModelFile> model(new ModelFile());
  std::string ext = Filesystem::getExtension(path);
  for (char& c : ext)
    c = (char)tolower((unsigned char)c);
  std::string message;
  bool ok = false;
  if (ext == "gltf" || ext == "glb"){
    GltfReader reader(*model, path);
    ok = reader.read(message);
  }
  else if (ext == "obj" || ext == "off"){
    //Mesh reads plain files
    std::string file = Assets::isAbsolute(path) ? path : Assets::getFilePath(path);
    std::unique_ptr<Mesh> mesh(new Mesh());
    if (!file.empty() && mesh->loadFromFile(file)){
      mesh->setFilename(path);
      model->mRoot.reset(new SceneNode(SceneNode::GROUP));
      Model* node = new Model(mesh.get());
      std::string name = Filesystem::getFileComponent(path);
      node->setName(name.substr(0, name.find_last_of('.')));
      model->mRoot->addChild(node);
      model->mMeshes.push_back(std::move(mesh));
      ok = true;
    }
    else
      message = "file not found";
  }
  else
    message = "unknown file type (glTF, GLB, OBJ and OFF are read)";
  if (!ok){
    TR_ERROR("cannot load model %s: %s", path.c_str(), message.c_str());
    if (error)
      *error = message;
    return nullptr;
  }
  if (error)
    error->clear();
  return model;
}

SceneNode* ModelFile::findNode(const std::string& name) const{
  return mRoot->findNode(name);
}

bool ModelFile::buildBuffers(Gfx::RenderDevice& device){
  bool ok = true;
  for (auto& mesh : mMeshes){
    if (!mesh->getSubMeshes().empty())
      ok &= mesh->buildBuffers(device);
  }
  return ok;
}

void ModelFile::render(Gfx::DrawContext& ctx, Mesh::DrawPart part){
  mRoot->render(ctx, part);
}

namespace{

void addBounds(const SceneNode* node, const Matrix& parent, BBox& box){
  Matrix trafo = parent*node->getTrafo();
  if (node->getType() == SceneNode::MESH){
    Mesh* mesh = static_cast<const Model*>(node)->getMesh();
    if (mesh && mesh->getNumVertices() > 0){
      BBox extents = mesh->getExtents();
      Vec3f lo = extents.getMin(), hi = extents.getMax();
      for (int i = 0; i < 8; ++i){
        Vec3f corner(i&1 ? hi.x : lo.x, i&2 ? hi.y : lo.y, i&4 ? hi.z : lo.z);
        box.addPoint(trafo*corner);
      }
    }
  }
  for (const SceneNode* child : node->getChildren())
    addBounds(child, trafo, box);
}

}

BBox ModelFile::getBounds() const{
  BBox box;
  addBounds(mRoot.get(), Matrix(Matrix::Identity), box);
  return box;
}

}
