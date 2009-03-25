//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: math.hh                                                    |
//  ==================================================================

#ifndef MATH_HH
#define MATH_HH

#include "math/vector.h"

#define BEHIND		0
#define INTERSECTS	1
#define FRONT		2

using CGE::Vector3D;

//! provides some standard math and special math for 3d environments
/*! In this class, everything is static because instanciating a math object 
 *  seems somehow strange to me. The things like sinus or absolute are there
 *  to get an abstraction layer from the standard C++ math library. 
 *  The main purpose of this class is however to do collision detection.
 */
class Maths
{
public:
  //! Constructor
  Maths();  
  //! Destructor
  ~Maths();
  //! returns the absolute value of num
  static float Absolute(float num);
  //! the normal of a planar polygon
  static Vector3D Normal(const Vector3D polygon[]);
  //! returns the point on the line segment [a b] that is closest to point
  static Vector3D ClosestPointOnLine(Vector3D a, Vector3D b, Vector3D point);
  //! returns the distance of the plane from the origin
  /*! \param normal the normal of the plane
   *  \param point any arbitrary point on the plane
   */
  static float PlaneDistance(Vector3D normal, Vector3D point);
  //! tests, if a polygon and a line intersect
  /*! \param polygon the polygon
   *  \param line the line
   *  \param normal the normal of the polygon is returned in here
   *  \param originDistance the distance of the plane in which the polygon lies to the origin is returned in here
   */
  static bool IntersectedPlane(Vector3D polygon[], Vector3D line[], Vector3D &normal, float &originDistance);
  //! returns an intersection point of a plane and a line
  /*! the method assumes, that there exists an intersection
   *  \param normal plane normal
   *  \param line the line
   *  \param distance the plane distance from the origion
   *  \returns Intersection point
   */
  static Vector3D IntersectionPoint(Vector3D normal, Vector3D line[], double distance);
  //! returns true if the intersection point is inside the polygon
  static bool InsidePolygon(Vector3D intersection, const Vector3D polygon[], long verticesCount);
  //! tests collision between a line and polygon
  static bool IntersectedPolygon(Vector3D polygon[], Vector3D line[], int vertexCount);
  //! classifies a sphere according to a plane
  /*! \param center the center of the sphere
   * \param normal the plane normal
   * \param point any point on the plane
   * \param radius the radius of the sphere
   * \param distance the distance between the sphere's middle point and the plane is returned in here
   * \returns BEHIND, INTERSECTS or FRONT
   */
  static int ClassifySphere(Vector3D &center, Vector3D &normal, const Vector3D &point, float radius, float &distance);
  //! Tests, if the sphere collided with an edge of the polygon
  static bool EdgeSphereCollision(Vector3D &center, const Vector3D polygon[], int vertexCount, float radius);
  //! returns true if the sphere intersects the polygon.
  static bool SpherePolygonCollision(Vector3D polygon[], Vector3D &center, int vertexCount, float radius);
  //! returns the offset that the sphere needs to move that it does not intersect the plane any longer
  /*! \param normal the plane's normal
   *  \param radius the radius of the sphere
   *  \param distance the plane's distance from the origin
   */
  static Vector3D GetCollisionOffset(Vector3D &normal, float radius, float distance);
  //! Calculates cosinus
  inline static float Cosinus(float x){return cos(x);}
  //! Calculates sinus
  inline static float Sinus(float x){return sin(x);}
};

#endif
