#ifndef HQMEXPORT_H
#define HQMEXPORT_H

#include <string>

namespace Graphics{
class Scene;
};

class HQMExport{
public:
  HQMExport();
  ~HQMExport();
  bool exportHQM(Graphics::Scene& scn, const std::string& filename);
private:
};
  

#endif
