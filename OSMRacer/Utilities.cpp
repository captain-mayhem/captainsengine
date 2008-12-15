#define _USE_MATH_DEFINES
#include "Utilities.h"

#include <math.h>

const double Utility::SCALE=6371.009;

Math::Vec3<double> Utility::polarToCartesian(double latitude, double longitude){
  double lat = latitude/360*M_PI;
  double lon = longitude/360*M_PI;
  double x = cos(lat)*sin(lon);
  double y = sin(lat);
  double z = cos(lat)*cos(lon);
  return Math::Vec3<double>(x,y,z);
}