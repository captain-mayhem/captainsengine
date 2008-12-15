
#include "OSMReader.h"
#include "system/engine.h"
#include "MapChunk.h"
#include "Utilities.h"

OSMReader::OSMReader(const std::string& filename) : mState(START){
  mDoc.LoadFile(filename.c_str());
}

OSMReader::~OSMReader(){

}

void OSMReader::read(MapChunk* map){
  mMap = map;
  readNode(&mDoc);
}

void OSMReader::readNode(TiXmlNode* node){
  if (!node)
    return;
  bool ignoreChildren = false;
  int t = node->Type();
  switch(t){
    case TiXmlNode::ELEMENT:
      switch(mState){
        case START:
        case OSM:
          ignoreChildren = processElement(node);
          break;
        case WAY:
          ignoreChildren = processWay(node);
          break;
      }
      break;
    default:
      break;
  }
  if (ignoreChildren)
    return;
  for(TiXmlNode* child = node->FirstChild(); child != NULL; child = child->NextSibling()){
    readNode(child);
  }
}

bool OSMReader::processElement(TiXmlNode* node){
  //System::Log << node->Value() << "\n";
  TiXmlElement* elem = node->ToElement();
  if (std::strcmp(node->Value(),"osm") == 0){
    mState = OSM;
    return false;
  }
  if (std::strcmp(node->Value(),"bounds") == 0){
    double minlat, maxlat, minlon, maxlon;
    readAttribute(elem, "minlat", minlat);
    readAttribute(elem, "maxlat", maxlat);
    readAttribute(elem, "minlon", minlon);
    readAttribute(elem, "maxlon", maxlon);
    Math::Vec3d minBox = Utility::polarToCartesian(minlat, minlon)*Utility::SCALE;
    Math::Vec3d maxBox = Utility::polarToCartesian(maxlat, maxlon)*Utility::SCALE;
    Math::Vec3d center = (minBox+maxBox)/2;
    Math::Vec3d extreme = center.normalized()*Utility::SCALE;
    mMap->addStreetNode(10,2,3,1);
    mMap->addStreetNode(10,2.5,3,2);
    return true;
  }
  if (std::strcmp(node->Value(),"node") == 0){
    int id;
    double lat, lon;
    readAttribute(elem, "id", id);
    readAttribute(elem, "lat", lat);
    readAttribute(elem, "lon", lon);
    return true;
  }
  if (std::strcmp(node->Value(),"way") == 0){
    int id;
    readAttribute(elem, "id", id);
    mState = WAY;
    for(TiXmlNode* child = node->FirstChild(); child != NULL; child = child->NextSibling()){
      readNode(child);
    }
    mState = OSM;
    return true;
  }
  if (std::strcmp(node->Value(),"relation") == 0){
    return true;
  }
  System::Log << "Unexpected xml value " << node->Value() << "\n";
  return true;
}

bool OSMReader::processWay(TiXmlNode* node){
  TiXmlElement* elem = node->ToElement();
  if (std::strcmp(node->Value(),"nd") == 0){
    int ref;
    readAttribute(elem, "ref", ref);
    return true;
  }
  if (std::strcmp(node->Value(),"tag") == 0){
    return true;
  }
  System::Log << "Unexpected xml value " << node->Value() << "\n";
  return true;
}

template<typename T> void OSMReader::readAttribute(TiXmlElement* element, const char* name, T& value){
  if (!element || !name)
    return;
  element->Attribute(name, &value);
}
