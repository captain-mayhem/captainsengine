
#include "MapChunk.h"
#include "GeoGen.h"
#include <mesh/SimpleMesh.h>
#include <renderer/drawcontext.h>
#include <renderer/screentext.h>

using namespace CGE;

MapChunk::MapChunk(){
  mStreets.setTraversalCB(renderCB);
}

MapChunk::~MapChunk(){

}

MapChunk::NodeRef MapChunk::addNode(const CGE::Vec3f& position){
  NodeRef node = Ptr<Node>(new Node(position));
  mGraph.addSingleNode(node);
  return node;
}

MapChunk::StreetRef MapChunk::addStreet(const StreetInfo& info){
  StreetRef street = Ptr<StreetInfo>(new StreetInfo(info));
  return street;
}

void MapChunk::addStreetSegment(MapChunk::NodeRef from, MapChunk::NodeRef to, StreetRef street){
  if (mGraph.connect(from,to)){
    from->mStreets.push_back(street);
  }
  //if (street->oneway)
  //  return;
  if (mGraph.connect(to,from)){
    to->mStreets.push_back(street);
  }
}

int drawCount;
namespace{
struct Traversal{
  CGE::Gfx::DrawContext* ctx;
  const CGE::Camera* cam;
};
}

void MapChunk::render(CGE::Gfx::DrawContext& ctx, const CGE::Camera* cam){
  //mTree.renderDebug();
  drawCount = 0;
  //mTree.render(cam);s
  CGE::Material mat("street");
  mat.setDiffuse(CGE::Color(0.2f,0.2f,0.2f,1.0f));
  mat.setAmbient(CGE::Color(0.1f,0.1f,0.1f,1.0f));
  ctx.setMaterial(mat);
  Traversal traversal = {&ctx, cam};
  mStreets.traverse(&traversal);
  char tmp[32];
  sprintf(tmp,"Map nodes rendered: %i", drawCount);
  CGE::Engine::instance()->getScreenText(0)->print(20,700,tmp,0);
}

CGE::OctreeStatic<SimpleMesh*>::TraversalState MapChunk::renderCB(const std::vector<SimpleMesh*>& values, const CGE::BBox& box, uint8 flags, void* data){
  Traversal* traversal = (Traversal*)data;
  const CGE::Camera* cam = traversal->cam;
  OctreeStatic<SimpleMesh*>::TraversalState state = OctreeStatic<SimpleMesh*>::RECURSE_TO_END;

  //check if we do not pass through
  if (!(flags & OctreeStatic<SimpleMesh*>::PASS_THROUGH)){
    Frustum::Result res = cam->getFrustum().checkBox(box);
    if (res == Frustum::OUTSIDE)
      return OctreeStatic<SimpleMesh*>::STOP_RECURSION;
    if (res == Frustum::INTERSECT)
      state = OctreeStatic<SimpleMesh*>::RECURSE_FURTHER;
  }

  for (uint32 i = 0; i < values.size(); ++i){
    values[i]->render(*traversal->ctx);
    ++drawCount;
  }

  return state;
}

