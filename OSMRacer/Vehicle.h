
#include "math/vector.h"
#include <physics/Body.h>

namespace Graphics{
  class Camera;
}

class Vehicle : public CGE::Body{
public:
  Vehicle();
  ~Vehicle();
  void render(const Graphics::Camera& cam);
protected:
  CGE::Body* mAutobody;
  CGE::Body* mWheels[4];
};
