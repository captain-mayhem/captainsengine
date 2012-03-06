#include "PostProcessing.h"

#include "Renderer.h"

using namespace adv;

static const char vertexshadersource[] =
"precision mediump float;\n"
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"\n"
"uniform mat4 matrices[3];\n"
"\n"
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*0.625, (0.0+position.y)*0.9375);\n"
"  tex_coord2 = vec2(position.x*2.0-1.0, (0.0+position.y)*2.0-1.0)*vec2(0.0015625*2, 0.002083*2);\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
"";

static const char fragmentshadersource[] =
"precision mediump float;\n"
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform int textureEnabled;\n"
"uniform vec4 constColor;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
//"  color.rgb = color.rgb*color.rgb*vec3(2.0);\n"
"  vec4 intensity = vec4(0.8,0.8,0.8,0.0)/vec4(6.0);\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st)*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(2.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(3.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(4.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(5.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(6.0))*intensity;\n"
"  gl_FragColor = color;\n"
"  gl_FragDepth = gl_FragCoord.z;\n"
"}\n"
"";

PostProcessor::Effect::Effect(const char* vertexsource, const char* fragmentsource){
  mShader.addShader(GL_VERTEX_SHADER, vertexshadersource);
  mShader.addShader(GL_FRAGMENT_SHADER, fragmentshadersource);
  mShader.linkShaders();
}

PostProcessor::Effect::~Effect(){
}

PostProcessor::PostProcessor(int width, int height, int depth) : mResult(width, height, depth){
  Effect* darkbloom = new Effect(vertexshadersource, fragmentshadersource);
  GLint tex = darkbloom->getShader().getUniformLocation("texture");
  darkbloom->getShader().uniform(tex, 0);
  mEffects["darkbloom"] = darkbloom;
}

PostProcessor::~PostProcessor(){
  for (std::map<std::string,Effect*>::iterator iter = mEffects.begin(); iter != mEffects.end(); ++iter){
    delete iter->second;
  }
}

BlitObject* PostProcessor::process(BlitObject* input){
  mResult.bind();
  glBindTexture(GL_TEXTURE_2D, input->getTexture());
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  mEffects["darkbloom"]->activate();
  //GLint pos = mEffects["darkbloom"]->getShader().getAttribLocation("position");
  //glVertexAttribPointer(pos, size, type, GL_FALSE, 0, Engine::instance()->getVertices());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  mEffects["darkbloom"]->deactivate();
  mResult.unbind();
  //return &mResult;
  return input;
}
