#include <string>
#include <vector>

namespace Graphics{

class VertexBuffer;
  
struct VerTexNorm{
  float v[3];
  float t[2];
  float n[3];
};
  
class Model{
public:
  Model();
  ~Model();
  void loadFromHMB(const std::string& filename);
  inline void activate() {vb_->activate();}
  inline void draw() {vb_->draw(VB_TRIANGLES);}
private:
  std::vector<VerTexNorm> vertices_;
  std::vector<unsigned short> indices_;
  VertexBuffer* vb_;
};

}

