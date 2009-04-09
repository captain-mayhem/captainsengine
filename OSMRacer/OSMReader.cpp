
#include "OSMReader.h"
#include <float.h>
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
  mMap->buildAccelerationStructures();
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
        case NODE:
          ignoreChildren = processNode(node);
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
  //CGE::Log << node->Value() << "\n";
  TiXmlElement* elem = node->ToElement();
  if (std::strcmp(node->Value(),"osm") == 0){
    mState = OSM;
    return false;
  }
  if (std::strcmp(node->Value(),"bounds") == 0){
    //info is not usable for octree
    double minlat, maxlat, minlon, maxlon;
    readAttribute(elem, "minlat", minlat);
    readAttribute(elem, "maxlat", maxlat);
    readAttribute(elem, "minlon", minlon);
    readAttribute(elem, "maxlon", maxlon);
    CGE::Vec3d minP = Utility::polarToCartesian(minlat, minlon)*Utility::SCALE;
    CGE::Vec3d maxP = Utility::polarToCartesian(maxlat, maxlon)*Utility::SCALE;
    mMap->setExtent(minP,maxP);
    /*CGE::Vec3d minBox = CGE::Vec3d(DBL_MAX,DBL_MAX,DBL_MAX);
    CGE::Vec3d maxBox = CGE::Vec3d(-DBL_MAX,-DBL_MAX,-DBL_MAX);
    box_extent(minBox,maxBox,(Utility::polarToCartesian(minlat, minlon)*Utility::SCALE));
    box_extent(minBox,maxBox,(Utility::polarToCartesian(maxlat, maxlon)*Utility::SCALE));
    box_extent(minBox,maxBox,(Utility::polarToCartesian(minlat, maxlon)*Utility::SCALE));
    box_extent(minBox,maxBox,(Utility::polarToCartesian(maxlat, minlon)*Utility::SCALE));*/
    //CGE::Vec3d minBox = Utility::polarToCartesian(minlat, minlon)*Utility::SCALE;
    //CGE::Vec3d maxBox = Utility::polarToCartesian(maxlat, maxlon)*Utility::SCALE;
    /*if (minBox.x > maxBox.x){
      double tmp = minBox.x; minBox.x = maxBox.x; maxBox.x = tmp;
    }
    if (minBox.y > maxBox.y){
      double tmp = minBox.y; minBox.y = maxBox.y; maxBox.y = tmp;
    }
    if (minBox.z > maxBox.z){
      double tmp = minBox.z; minBox.z = maxBox.z; maxBox.z = tmp;
    }*//*
    CGE::Vec3d center = (minBox+maxBox)/2;
    CGE::Vec3d extreme = center.normalized()*Utility::SCALE;
    minBox.x = extreme.x < minBox.x ? extreme.x : minBox.x;
    minBox.y = extreme.y < minBox.y ? extreme.y : minBox.y;
    minBox.z = extreme.z < minBox.z ? extreme.z : minBox.z;
    maxBox.x = extreme.x > maxBox.x ? extreme.x : maxBox.x;
    maxBox.y = extreme.y > maxBox.y ? extreme.y : maxBox.y;
    maxBox.z = extreme.z > maxBox.z ? extreme.z : maxBox.z;*/
    //recalculate new center
    //CGE::Vec3d center = (minBox+maxBox)/2;
    //mMap->setDimensions(center, (maxBox-minBox)/2.0f);
    return true;
  }
  if (std::strcmp(node->Value(),"node") == 0){
    int id;
    double lat, lon;
    readAttribute(elem, "id", id);
    readAttribute(elem, "lat", lat);
    readAttribute(elem, "lon", lon);
    CGE::Vec3d v = Utility::polarToCartesian(lat, lon)*Utility::SCALE;
    mState = NODE;
    for(TiXmlNode* child = node->FirstChild(); child != NULL; child = child->NextSibling()){
      readNode(child);
    }
    mState = OSM;
    mMap->addNode(v,id);
    return true;
  }
  if (std::strcmp(node->Value(),"way") == 0){
    readAttribute(elem, "id", mCurrStreet);
    mLastNode = 0;
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
  CGE::Log << "Unexpected xml value " << node->Value() << "\n";
  return true;
}

bool OSMReader::processNode(TiXmlNode* node){
  TiXmlElement* elem = node->ToElement();
  if (std::strcmp(node->Value(),"tag") == 0){
    std::pair<std::string,std::string> tag = readTag(elem);
    if (tag.first == "created_by" || tag.first == "converted_by" ||
      tag.first == "source" || tag.first == "class"){
      //not interesting
    }
    /*else if (tag.first == "amenity"){

    }
    else if (tag.first == "name"){

    }
    else if (tag.first == "is_in"){

    }
    else if (tag.first == "place"){

    }*/
    //CGE::Log << "Unexpected tag in xml file: " << tag.first << " - " << tag.second << "\n";
    return true;
  }
  CGE::Log << "Unexpected xml value " << node->Value() << "\n";
  return true;
}

bool OSMReader::processWay(TiXmlNode* node){
  TiXmlElement* elem = node->ToElement();
  if (std::strcmp(node->Value(),"nd") == 0){
    int ref;
    readAttribute(elem, "ref", ref);
    if (mLastNode != 0){
      mMap->addStreet(mCurrStreet,mLastNode,ref);
    }
    mLastNode = ref;
    return true;
  }
  if (std::strcmp(node->Value(),"tag") == 0){
    std::pair<std::string,std::string> tag = readTag(elem);
    if (tag.first == "created_by"){
      //uninteresting
    }
    else if(tag.first == "highway"){
      //we have really a street
    }
    else if(tag.first == "name"){

    }
    CGE::Log << "Unexpected tag in xml file: " << tag.first << " - " << tag.second << "\n";
    return true;
  }
  CGE::Log << "Unexpected xml value " << node->Value() << "\n";
  return true;
}

template<typename T> void OSMReader::readAttribute(TiXmlElement* element, const char* name, T& value){
  if (!element || !name)
    return;
  element->Attribute(name, &value);
}

std::pair<std::string,std::string> OSMReader::readTag(TiXmlElement* element){
  std::pair<std::string,std::string> res;
  res.first = element->Attribute("k");
  res.second = element->Attribute("v");
  return res;
}

