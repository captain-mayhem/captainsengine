#define _USE_MATH_DEFINES
#include "Utilities.h"

#include <math.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

const double Utility::SCALE=6371.6009;

CGE::Vec3<double> Utility::polarToCartesian(double latitude, double longitude){
  double lat = latitude/360*M_PI;
  double lon = longitude/360*M_PI;
  double x = sin(lat)*sin(lon);
  double y = cos(lat);
  double z = -sin(lat)*cos(lon);
  return CGE::Vec3<double>(x,y,z);
}