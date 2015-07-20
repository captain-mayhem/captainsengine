
#ifndef GEOCACHE_H
#define GEOCACHE_H

enum PathType{
  UNKNOWN=0,
  STREET_FOOTPATH,
  STREET_PATH,
  STREET_SMALL,
  STREET_MEDIUM,
  STREET_BIG
};

class GeoCache{
public:
  static void init();
  static void deinit();
  static GeoCache* instance() {return mCache;}

  void setPathType(int streetid, PathType type);
protected:
  GeoCache();
  ~GeoCache();
  static GeoCache* mCache;
};

#endif

