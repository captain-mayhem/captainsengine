#include "state.h"

#include <cstdlib>

VRState::VRState() : mSurface(NULL){
  for (int i = 0; i < NUM_VARYING; ++i)
    mVertexAttribArrays[i] = false;
}
