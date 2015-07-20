#ifndef QUATERNION_H
#define QUATERNION_H

class Quaternion{
public:
  Quaternion(){x=0.0f;y=0.0f;z=0.0f;w=0.0f;}
  union{
    struct{
      float x;
      float y;
      float z;
      float w;
    };
    float data[4];
  };
};

#endif

