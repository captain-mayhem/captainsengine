
namespace CGE{
class Simulator{
public:
  Simulator();
  ~Simulator();
  void setSimulationCallback(void (*func)(double)) {mSimulationCB = func;}
  void doSimulationStep(double time);
protected:
  void (*mSimulationCB)(double time);
};
}