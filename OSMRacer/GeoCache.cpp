
#include "GeoCache.h"

#include <cstdlib>

GeoCache* GeoCache::mCache = NULL;

void GeoCache::init(){
  mCache = new GeoCache();
}

void GeoCache::deinit(){
  delete mCache;
  mCache = NULL;
}

GeoCache::GeoCache(){
  
}

GeoCache::~GeoCache(){

}