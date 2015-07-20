#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <set>
#include <string>
#include "glm.h"

using std::ofstream;
using std::ios;
using std::list;
using std::set;
using std::pair;
using std::vector;
using std::string;

struct vtn{
  long v;
  long t;
  long n;
  long idx;
  bool operator<(vtn const & test) const{
    /*
    if (v == test.v){
      if (t == test.t){
        return n < test.n;
      }
      return t < test.t;
    }
    return v < test.v;
    */
    return v < test.v || t < test.t || n < test.n;
  }
};

struct VerTexNorm{
  float v[3];
  float t[2];
  float n[3];
};

int main(int argc, char** argv){
  GLMmodel* model = glmReadOBJ(argv[2]);
  glmVertexNormals(model, 90.0, GL_TRUE);
  //glmWriteOBJ(model, "test.obj", GLM_SMOOTH | GLM_TEXTURE);
  bool flip;
  if (string(argv[1]) == "-f")
    flip = true;
  else
    flip = false;
 
  //if (flip)
    //std::cerr << "Flipped\n";
  string output(argv[2]);
  output[output.size()-3] = 'h';
  output[output.size()-2] = 'm';
  output[output.size()-1] = 'b';
  ofstream out(output.c_str(), ios::binary);

  vector<VerTexNorm> points;
  vector<unsigned short> indices;
  set<vtn> tripels;
  int count = 0;
  
  GLMgroup* group = model->groups;
  while(group){
    for (int i = 0; i < group->numtriangles; i++){
      if (flip){
        for (int j = 2; j >= 0; j--){
          vtn tripel;
          tripel.v = model->triangles[ group->triangles[i] ].vindices[j];
          tripel.t = model->triangles[ group->triangles[i] ].tindices[j];
          tripel.n = model->triangles[ group->triangles[i] ].nindices[j];
          tripel.idx = count;
          //insert into set
          pair<set<vtn>::iterator,bool> result;
          result = tripels.insert(tripel);
          //std::cerr << i << " " << j << "\n";
          //std::cerr << tripel.v << " " << tripel.t << " " << tripel.n << "\n";
          //tripel already present
          if (!result.second){
            vtn orig = *result.first;
            indices.push_back(orig.idx);
          }
          else{
            indices.push_back(tripel.idx);
            VerTexNorm point;
            point.v[0] = model->vertices[3*tripel.v+0];
            point.v[1] = model->vertices[3*tripel.v+1];
            point.v[2] = model->vertices[3*tripel.v+2];
            point.t[0] = model->texcoords[2*tripel.t+0];
            point.t[1] = model->texcoords[2*tripel.t+1];
            point.n[0] = model->normals[3*tripel.n+0];
            point.n[1] = model->normals[3*tripel.n+1];
            point.n[2] = model->normals[3*tripel.n+2];
            points.push_back(point);
            count++;
          }
        }
      }
      else{
        for (int j = 0; j < 3; j++){
          vtn tripel;
          tripel.v = model->triangles[ group->triangles[i] ].vindices[j];
          tripel.t = model->triangles[ group->triangles[i] ].tindices[j];
          tripel.n = model->triangles[ group->triangles[i] ].nindices[j];
          tripel.idx = count;
          //insert into set
          pair<set<vtn>::iterator,bool> result;
          result = tripels.insert(tripel);
          //std::cerr << i << " " << j << "\n";
          //std::cerr << tripel.v << " " << tripel.t << " " << tripel.n << "\n";
          //tripel already present
          if (!result.second){
            vtn orig = *result.first;
            indices.push_back(orig.idx);
          }
          else{
            indices.push_back(tripel.idx);
            VerTexNorm point;
            point.v[0] = model->vertices[3*tripel.v+0];
            point.v[1] = model->vertices[3*tripel.v+1];
            point.v[2] = model->vertices[3*tripel.v+2];
            point.t[0] = model->texcoords[2*tripel.t+0];
            point.t[1] = model->texcoords[2*tripel.t+1];
            point.n[0] = model->normals[3*tripel.n+0];
            point.n[1] = model->normals[3*tripel.n+1];
            point.n[2] = model->normals[3*tripel.n+2];
            points.push_back(point);
            count++;
          }
        }
      }
    }
    
    group = group->next;
  }

  unsigned size;
  size = points.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < points.size(); i++){
    VerTexNorm p = points[i];
    //std::cerr << p.v[0] << " " << p.v[1] << " " << p.v[2] << " / " << p.t[0] << " " << p.t[1] << " / " << p.n[0] << " " << p.n[1] << " " << p.n[2] << "\n";
    out.write((char*)&p, sizeof(p));
  }
  
  size = indices.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < indices.size(); i++){
    //std::cerr << indices[i] << " ";
    out.write((char*)&indices[i], sizeof(indices[i]));
  }

  //std::cerr << "\n";
  /*
  //vertices
  out.write((char*)&model->numvertices, sizeof(model->numvertices));
  for (int i = 1; i <= model->numvertices; i++){
    out.write((char*)&model->vertices[3*i+0], sizeof(model->vertices[0]));
    out.write((char*)&model->vertices[3*i+1], sizeof(model->vertices[0]));
    out.write((char*)&model->vertices[3*i+2], sizeof(model->vertices[0]));
  }
  
  //texcoords
  out.write((char*)&model->numtexcoords, sizeof(model->numtexcoords));
  for (int i = 1; i <= model->numtexcoords; i++){
    out.write((char*)&model->texcoords[2*i+0], sizeof(model->texcoords[0]));
    out.write((char*)&model->texcoords[2*i+1], sizeof(model->texcoords[0]));
  }
 
  //normals
  out.write((char*)&model->numnormals, sizeof(model->numnormals));
  for (int i = 1; i <= model->numnormals; i++){
    out.write((char*)&model->normals[3*i+0], sizeof(model->normals[0]));
    out.write((char*)&model->normals[3*i+1], sizeof(model->normals[0]));
    out.write((char*)&model->normals[3*i+2], sizeof(model->normals[0]));
  }
  */
  out.close();
  
  return 0;
}
