
#include "math/vector.h"

namespace Graphics{
  class Camera;
}

class Vehicle{
public:
  void render(const Graphics::Camera& cam);
protected:
  Math::Vec3f mPosition;
};
