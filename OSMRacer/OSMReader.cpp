
#include "OSMReader.h"
#include <float.h>
#include "system/engine.h"
#include "OSMMapAdapter.h"
#include "Utilities.h"

OSMReader::OSMReader(const std::string& filename) : mState(START){
  mDoc.LoadFile(filename.c_str());
}

OSMReader::~OSMReader(){

}

void OSMReader::read(OSMMapAdapter* map){
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
    double minlat = readDoubleAttribute(elem, "minlat");
    double maxlat = readDoubleAttribute(elem, "maxlat");
    double minlon = readDoubleAttribute(elem, "minlon");
    double maxlon = readDoubleAttribute(elem, "maxlon");
    CGE::Vec3d minP = Utility::polarToCartesian(minlat, minlon)*Utility::SCALE;
    CGE::Vec3d maxP = Utility::polarToCartesian(maxlat, maxlon)*Utility::SCALE;
    mMap->setExtent(minP,maxP);
    return true;
  }
  if (std::strcmp(node->Value(),"node") == 0){
    uint64 id = readULongAttribute(elem, "id");
    double lat = readDoubleAttribute(elem, "lat");
    double lon = readDoubleAttribute(elem, "lon");
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
    mState = WAY;
    mCurrWay = WayInfo();
    mCurrWay.wayid = readULongAttribute(elem, "id");

    for(TiXmlNode* child = node->FirstChild(); child != NULL; child = child->NextSibling()){
      readNode(child);
    }

    if (mCurrWay.waytype == WayInfo::STREET){
      mMap->addStreet(mCurrWay.wayid, mCurrWay.waynodes, mCurrWay.street);
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
    uint64 ref = readULongAttribute(elem, "ref");
    mCurrWay.waynodes.push_back(ref);
    return true;
  }
  if (std::strcmp(node->Value(),"tag") == 0){
    std::pair<std::string,std::string> tag = readTag(elem);
    if (tag.first == "created_by" || tag.first == "source"){
      //uninteresting
    }
    else if(tag.first == "highway"){
      //we have really a street
      mCurrWay.waytype = WayInfo::STREET;
      //translate size
      if (tag.second == "primary"){
        mCurrWay.street.streettype = StreetInfo::FEDERAL_STREET;
      }
      else if (tag.second == "secondary"){
        mCurrWay.street.streettype = StreetInfo::COUNTY_STREET;
      }
      else if (tag.second == "residential"){
        mCurrWay.street.streettype = StreetInfo::MEDIUM_LOCAL_STREET;
      }
      else{
        CGE::Log << "Unexpected street qualifier: " << tag.first << " - " << tag.second << "\n";
      }
    }
    else if(tag.first == "name"){
      mCurrWay.street.name = tag.second;
    }
    else if (tag.first == "oneway"){
      if (tag.second == "yes")
        mCurrWay.street.oneway = true;
      else
        mCurrWay.street.oneway = false;
    }
    else{
      CGE::Log << "Unexpected street tag in xml file: " << tag.first << " - " << tag.second << "\n";
    }
    return true;
  }
  CGE::Log << "Unexpected xml value " << node->Value() << "\n";
  return true;
}

double OSMReader::readDoubleAttribute(TiXmlElement* element, const char* name){
  double value = 0;
  if (!element || !name)
    return value;
  element->Attribute(name, &value);
  return value;
}

uint64 OSMReader::readULongAttribute(TiXmlElement* element, const char* name){
  uint64 value = 0;
  if (!element || !name)
    return value;
  value = _atoi64(element->Attribute(name));
  return value;
}

std::pair<std::string,std::string> OSMReader::readTag(TiXmlElement* element){
  std::pair<std::string,std::string> res;
  res.first = element->Attribute("k");
  res.second = element->Attribute("v");
  return res;
}

