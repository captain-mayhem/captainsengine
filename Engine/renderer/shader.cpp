#include "shader.h"

#include <cstdlib>

#include <system/engine.h>

using namespace CGE;

Shader* Shader::mCurrShader = NULL;

Shader::Shader() : mOldProg(NULL), mApplyLight(NULL), mApplyLightData(NULL) {

}

void Shader::syncMatrix(char const* name, MatrixType type){
  int loc = getUniformLocation(VERTEX_SHADER, name);
  if (loc < 0)
    return;
  mSyncMat[loc] = type;
}

void Shader::applyEngineUniforms(){
  lockUniforms(VERTEX_SHADER);
  for (std::map<int, MatrixType>::iterator iter = mSyncMat.begin(); iter != mSyncMat.end(); ++iter){
    uniform(iter->first, Engine::instance()->getRenderer()->getMatrix(iter->second));
  }
  unlockUniforms(VERTEX_SHADER);
}

bool Shader::applyMaterial(Material const& mat){
  return false;
}
