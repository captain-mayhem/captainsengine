
#include "Simulator.h"

#include <cstdlib>

using namespace CGE;

Simulator::Simulator() : mSimulationCB(NULL){

}

Simulator::~Simulator(){

}

void Simulator::doSimulationStep(double time){
  if (mSimulationCB)
    mSimulationCB(time);
}