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
#include "vector.hh"

using std::ostream;

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
  //! Destructor
	~Matrix();
  //! Matrix addition
	Matrix operator+(const Matrix& mat);
  //! Matrix subtraction
	Matrix operator-(const Matrix& mat);
  //! Matrix multiplication
	Matrix operator*(const Matrix& mat);
  //! Multiply a vector to a matrix
	Vector3D operator*(const Vector3D& vec);
  //! Multiply the matrix with a scalar
	Matrix operator*(float number);
  //! Multiply the matrix with a scalar
	friend Matrix operator*(float number, const Matrix& mat);
  //! returns the matrix as float array
	float* operator*();
  //! write matrix to a stream
	friend ostream& operator<<(ostream& stream, const Matrix& mat);
  //! get matrix from OpenGL
	void fromOpenGL();
  //! multiply matrix with the OpenGL matrix stack
	void toOpenGL();
  //! transpose a matrix
	Matrix transpose();
  //! invert a matrix
	Matrix inverse();
private:
  //! the matrix entries
	float data_[16];
};

Matrix operator*(float number, const Matrix& mat);
ostream& operator<<(ostream& stream, const Matrix& mat);

#endif
