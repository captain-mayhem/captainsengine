#include <math/vector.h>

class Utility{
public:
  static const double SCALE;
  static CGE::Vec3<double> polarToCartesian(double longitude, double latitude);
};

