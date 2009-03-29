//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Stefan Ruff and Marco Staginski                                  |
// | stefan.ruff@gmx.net and captain@captain-online.de                |
// |------------------------------------------------------------------|
// | File: matrix.hh                                                  |
//  ==================================================================

#ifndef MATRIX_HH
#define MATRIX_HH

#include <iostream>
#include "vector.h"
#include "quaternion.h"

using std::ostream;

namespace CGE{

//! This class is a 4x4-matrix
/*! It provides some standard matrix operations
 */
class Matrix{
public:
  //! the type of the matrix
	enum Type{
		Rotation,
		Translation,
		Scale,
		Identity
	};
  //! Default constructor
	Matrix();
  //! Rotation matrix
  /* \param t has to be Rotation
   * \param axis the axis to rotate around
   * \param angle the rotation angle specified in radians
   */
	Matrix(Type t, Vector3D axis, float angle);
  //! Set matrix from another matrix
  /* \param t the type of the new matrix
   * \param mat the matrix
   */
  Matrix(Type t, const Matrix& mat);
  //! Translation or Scale matrix
  /*! for Translation, data has to hold the translation vector
   *  for Scale, data is a vector with the scale factors
   */
	Matrix(Type t, Vector3D data);
  //! Identity Matrix
  /* \param t has to be Identity
   */
	Matrix(Type t);
  //! Constructor that initializes the matrix with the given values
	/*! Be careful: The values must be specified columnwise like in OpenGL
   */
  Matrix(float entries[16]);
  //! Builds a matrix from a quaternion
  Matrix(const Quaternion& quat);
  //! Init a matrix with four vectors
  Matrix(const Vec3f& r1, const Vec3f& r2, const Vec3f& r3, const Vec3f& r4);
  //! Destructor
	~Matrix();
  //! Matrix addition
	Matrix operator+(const Matrix& mat);
  //! Matrix subtraction
	Matrix operator-(const Matrix& mat);
  //! Matrix multiplication
	Matrix operator*(const Matrix& mat) const;
  //! Multiply a vector to a matrix
	Vector3D operator*(const Vector3D& vec) const;
  //! Multiply the matrix with a scalar
	Matrix operator*(float number) const;
  //! Multiply the matrix with a scalar
  Matrix& operator*=(float number);
  //! Multiply the matrix with a scalar
	friend Matrix operator*(float number, const Matrix& mat);
  //! returns the matrix as float array
	float* operator*();
  //! write matrix to a stream
	friend ostream& operator<<(ostream& stream, const Matrix& mat);
  //! access a row of the matrix
  CGE::Vec3<float*> operator[](int i);
  const float& at(int i) const {return data_[i];}
  float& at(int i) {return data_[i];}
  //! get matrix from OpenGL
	//void fromOpenGL();
  //! multiply matrix with the OpenGL matrix stack
	//void toOpenGL();
  //! transpose a matrix
	Matrix transpose() const;
  //! invert a matrix
	Matrix inverse() const;
  //! perform SVD
  /* \param
   * \return scale factor*/
  float SVD() const;
  //! get the matrix data
  const float* getData() const {return data_;}
  //! get the translation vector
  Vector3D getTranslation() const {return Vector3D(data_[12], data_[13], data_[14]);}
private:
  //! the matrix entries
	float data_[16];
};

Matrix operator*(float number, const Matrix& mat);
ostream& operator<<(ostream& stream, const Matrix& mat);

}

#endif
