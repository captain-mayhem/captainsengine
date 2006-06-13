#include <GL/gl.h>
#include "common.h"

void Forms::drawCube(float* texCoords){
#ifdef _CLIENT_
// The vertex array
  float vertices[] = {
    1, -1, 1,
    -1, -1, 1,
    -1, -1, -1,
    1, -1, -1,
    1, 1, 1,
    -1, 1, 1,
    -1, 1, -1,
    1, 1, -1,
  };
  // the index array
  unsigned int indices[] = {
    //BOTTOM
    0, 1, 2, 3,
    //FRONT
    4, 5, 1, 0,
    //LEFT
    5, 6, 2, 1,
    //BACK
    6, 7, 3, 2,
    //RIGHT
    7, 4, 0, 3,
    //TOP
    7, 6, 5, 4
  };
  //texture coordinates
  float textureCoords[] = {
    0,1,
    1,1,
    0,1,
    1,1,
    0,0,
    1,0,
    0,0,
    1,0
  };
  //set pointer
  glVertexPointer(3, GL_FLOAT, 3*sizeof(float), vertices);
  if (texCoords != NULL)
    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(float), texCoords);
  else
    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(float), textureCoords);
  //and draw
  glDrawElements(GL_QUADS,4*6, GL_UNSIGNED_INT, indices);
#endif
}

void Forms::drawCube2(float* texCoords){
#ifdef _CLIENT_
// The vertex array
  float vertices[] = {
    1, 0, 1,
    0, 0, 1,
    0, 0, 0,
    1, 0, 0,
    1, 1, 1,
    0, 1, 1,
    0, 1, 0,
    1, 1, 0,
  };
  // the index array
  unsigned int indices[] = {
    //BOTTOM
    0, 1, 2, 3,
    //FRONT
    4, 5, 1, 0,
    //LEFT
    5, 6, 2, 1,
    //BACK
    6, 7, 3, 2,
    //RIGHT
    7, 4, 0, 3,
    //TOP
    7, 6, 5, 4
  };
  //texture coordinates
  float textureCoords[] = {
    0,1,
    1,1,
    0,1,
    1,1,
    0,0,
    1,0,
    0,0,
    1,0
  };
  //set pointer
  glVertexPointer(3, GL_FLOAT, 3*sizeof(float), vertices);
  if (texCoords != NULL)
    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(float), texCoords);
  else
    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(float), textureCoords);
  //and draw
  glDrawElements(GL_QUADS,4*6, GL_UNSIGNED_INT, indices);
#endif
}
