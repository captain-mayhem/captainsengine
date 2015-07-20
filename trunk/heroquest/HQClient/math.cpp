//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: math.cpp                                                   |
//  ==================================================================

//is needed by Windows to know M_PI and some other math constants.
#define _USE_MATH_DEFINES

#include <math.h>
#include "math.h"

Maths::Maths(){
}

Maths::~Maths(){
}

//absolute value
float Maths::Absolute(float num){
  if(num < 0)
    return (0 - num);
  return num;
}


//returns the normal of a polygon
Vector3D Maths::Normal(const Vector3D polygon[]){
  Vector3D v1 = polygon[2] - polygon[0];
  Vector3D v2 = polygon[1] - polygon[0];

  Vector3D normal = v1.cross(v2);
  normal = normal.normalized();
  
  return normal;
}


//calculates closest point on line
Vector3D Maths::ClosestPointOnLine(Vector3D a, Vector3D b, Vector3D point){
  
  Vector3D v1 = point - a;
  Vector3D v2 = (b - a).normalized();
  
  float d = a.distance(b);
  
  //project v1 on v2 for distance
  float t = v2.dot(v1);
  
  //a is closest
  if (t <= 0)
    return a;

  //b is closest
  if (t >= d)
    return b;
  
  Vector3D v3 = v2 * t;
  
  Vector3D closestPoint = a + v3;
  return closestPoint;
}


//returns the distance between a plane and the origin
float Maths::PlaneDistance(Vector3D normal, Vector3D point){
  //plane: Ax + By + Cz + D = 0
  //D = -(Ax + By + Cz)
  float distance = - ((normal.x * point.x) + (normal.y * point.y) + (normal.z * point.z));
  return distance;
}


//checks if a line intersects a plane
bool Maths::IntersectedPlane(Vector3D polygon[], Vector3D line[], Vector3D &normal, float &originDistance){
  normal = Normal(polygon);
  originDistance = PlaneDistance(normal, polygon[0]);

  float distance1 = ((normal.x * line[0].x)+(normal.y * line[0].y)+(normal.z * line[0].z)) + originDistance;
  float distance2 = ((normal.x * line[1].x)+(normal.y * line[1].y)+(normal.z * line[1].z)) + originDistance;

  // Line points are on the same plane side ==> no intersection
  if(distance1 * distance2 >= 0)
    return false;

  return true;
}


//returns the intersection point of the line intersecting the plane
Vector3D Maths::IntersectionPoint(Vector3D normal, Vector3D line[], double distance){
  Vector3D point;

  //vector of the line
  Vector3D lineDir = line[1] - line[0];
  lineDir = lineDir.normalized();

  //clac plane distance
  double numerator = - (normal.x * line[0].x + normal.y * line[0].y + normal.z * line[0].z + distance);
  double denominator = normal.dot(lineDir);

  //the line is parallel to the plane, return arbitrary point
  if( denominator == 0.0)
    return line[0];

  double dist = numerator / denominator;
  
  point.x = (float)(line[0].x + (lineDir.x * dist));
  point.y = (float)(line[0].y + (lineDir.y * dist));
  point.z = (float)(line[0].z + (lineDir.z * dist));

  return point;
}

// checks, if a point is inside of a polygon
bool Maths::InsidePolygon(Vector3D intersection, const Vector3D polygon[], long verticeCount){
  const double MATCH_FACTOR = 0.99;
  double angle = 0.0;
  
  //sum up all angles of the polygon-vertex-to-point vectors
  for (int i = 0; i < verticeCount; i++){
    Vector3D a = polygon[i] - intersection;
    Vector3D b = polygon[(i + 1) % verticeCount] - intersection;
    angle += a.angle(b);
  }
  
  //angle > 2 Pi ==> inside polygon
  if(angle >= (MATCH_FACTOR*2*M_PI))
    return true;
  
  return false;
}

//checks if a line intersects a polygon
bool Maths::IntersectedPolygon(Vector3D polygon[], Vector3D line[], int vertexCount){
  Vector3D normal;
  float originDistance = 0;
	
  //intersect with plane first
  if(!IntersectedPlane(polygon, line,   normal,   originDistance))
    return false;

  //calculate intersection point
  Vector3D intersection = IntersectionPoint(normal, line, originDistance);

  // Test, if intersection point is within the polygon
  if(InsidePolygon(intersection, polygon, vertexCount))
    return true;
  
  return false;
}

// classifies a sphere according to a plane
int Maths::ClassifySphere(Vector3D &center, Vector3D &normal, const Vector3D &point, float radius, float &distance){
  // distance of the polygon from the origin
  float d = (float)PlaneDistance(normal, point);

  //distance of the sphere from the polygon
  distance = (normal.x * center.x + normal.y * center.y + normal.z * center.z + d);

  if(Absolute(distance) < radius)
    return INTERSECTS;
  else if(distance >= radius)
    return FRONT;
  return BEHIND;
}

//returns true if the sphere intersects edges of the polygon
bool Maths::EdgeSphereCollision(Vector3D &center, const Vector3D polygon[], int vertexCount, float radius){
	
  for(int i = 0; i < vertexCount; i++){
    // closest point on edge to the sphere's center
    Vector3D point = ClosestPointOnLine(polygon[i], polygon[(i + 1) % vertexCount], center);
    
    float distance = point.distance(center);

    if(distance < radius)
      return true;
  }
  return false;
}

//returns true if the sphere collides with the polygon
bool Maths::SpherePolygonCollision(Vector3D polygon[], Vector3D &center, int vertexCount, float radius){
  //find sphere classification
  Vector3D normal = Normal(polygon);
  float distance = 0;
  int classification = ClassifySphere(center, normal, polygon[0], radius, distance);

  if(classification == INTERSECTS){
    //find intersection point
    Vector3D offset = normal * distance;
    Vector3D position = center - offset;
    //inside?
    if(InsidePolygon(position, polygon, 3))
      return true;
    //no intersection so far ==> search edges
    else{
      if(EdgeSphereCollision(center, polygon, vertexCount, radius)){
	return true;
      }
    }
  }
  //no collision
  return false;
}

//returns the offset the sphere has to move to correct collision
Vector3D Maths::GetCollisionOffset(Vector3D &normal, float radius, float distance){
  Vector3D offset = Vector3D();

  if(distance > 0){
    //how much sphere and polygon overlap
    float distanceOver = radius - distance;
    offset = normal * distanceOver;
  }
  else{
    float distanceOver = radius + distance;
    offset = normal * -distanceOver;
  }
  return offset;
}

