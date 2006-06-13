//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Stefan Ruff and Marco Staginski                                  |
// | stefan.ruff@gmx.net and captain@captain-online.de                |
// |------------------------------------------------------------------|
// | File: matrix.cpp                                                 |
//  ==================================================================

#ifdef WIN32
	#include <windows.h>
#endif

#include <GL/gl.h>
#include "matrix.h"

using namespace Math;

//Pattern:
// 0 4 8  12
// 1 5 9  13
// 2 6 10 14
// 3 7 11 15

using namespace std;

//zero
Matrix::Matrix(){
	memset(data_, 0, 16*sizeof(float));
}

//rotate
Matrix::Matrix(Type t, Vector3D axis, float angle){
	
	memset(data_, 0, 16*sizeof(float));
	
	if (t == Rotation){
		double c = cos(angle);
		double s = sin(angle);
		double i = 1 - c;
		
		data_[0] = i*axis.x*axis.x + c;
		data_[1] = i*axis.x*axis.y + s*axis.z;
		data_[2] = i*axis.x*axis.z - s*axis.y;
		
		data_[4] = i*axis.x*axis.y - s*axis.z;
		data_[5] = i*axis.y*axis.y + c;
		data_[6] = i*axis.y*axis.z + s*axis.x;

		data_[8] = i*axis.x*axis.z + s*axis.y;
		data_[9] = i*axis.y*axis.z - s*axis.x;
		data_[10]= i*axis.z*axis.z + c;

		data_[15]= 1;
	}
	else
	{
		cerr<<"invalid matrix type"<<endl;
	}
}

//scale and translate
Matrix::Matrix(Type t, Vector3D data)
{
	memset(data_, 0, 16*sizeof(float));
	//scale matrix
	if(t == Scale)
	{
		data_[0] = data.x;			
		data_[5] = data.y;			
		data_[10] = data.z;
		data_[15] = 1;
	}else if (t == Translation)
	//translation matrix
	{
		data_[0] = 1;
		data_[5] = 1;
		data_[10] = 1;
		data_[15] = 1;
		data_[12] = data.x;
		data_[13] = data.y;
		data_[14] = data.z;
	}
	else
	{
		cerr<<"invalid matrix type"<<endl;
	}
}

//identity
Matrix::Matrix(Type t){
	memset(data_, 0, 16*sizeof(float));
	if (t == Identity){
		data_[0] = 1;
		data_[5] = 1;
		data_[10] = 1;
		data_[15] = 1;
	}
	else
	{
		cerr<<"invalid matrix type"<<endl;
	}
}

//user defined
Matrix::Matrix(float entries[16]){
	for (short i = 0; i < 16; i++){
		data_[i] = entries[i];
	}
}

Matrix::~Matrix(){}

Matrix Matrix::operator+(const Matrix& mat)
{
	Matrix result;
	for(short i=0; i<16; i++)
	{
		result.data_[i] = mat.data_[i] + data_[i];
	}
	return result;
}

Matrix Matrix::operator-(const Matrix& mat)
{
	Matrix result;
	for(short i=0; i<16; i++)
	{
		result.data_[i] = data_[i] - mat.data_[i];
	}
	return result;
}

Matrix Matrix::operator*(const Matrix& mat)
{
	Matrix result;
	for(short j=0; j<4; j++)
	{
		for(short i=0; i<4; i++)
		{
			result.data_[j+i*4] = data_[j] * mat.data_[4*i] 
													+ data_[j+4] * mat.data_[4*i+1]
													+ data_[j+8] * mat.data_[4*i+2]
													+ data_[j+12] * mat.data_[4*i+3];
		}
	}
	return result;
}

Vector3D Matrix::operator*(const Vector3D& vec)
{
	Vector3D result;
	result.x = vec.x*data_[0] + vec.y*data_[4] + vec.z*data_[8] + data_[12];
	result.y = vec.x*data_[1] + vec.y*data_[5] + vec.z*data_[9] + data_[13];
	result.z = vec.x*data_[2] + vec.y*data_[6] + vec.z*data_[10] + data_[14];
	return result;
}

Matrix Math::Matrix::operator*(float number){
	Matrix ret = *this;
	for (int i = 0; i < 16; i++){
		ret.data_[i] *= number;
	}
	return ret;
}

float* Matrix::operator*(){
	return data_;
}

ostream& Math::operator<<(ostream& stream, const Matrix& mat)
{
	stream<<"|-----------|"<<std::endl;
	for(short i=0; i<4; i++)
	{
		stream<<"| "<<mat.data_[i]<<" "<<mat.data_[i+4]<<" "<<mat.data_[i+8]<<" "<<mat.data_[i+12]<<" |"<<std::endl;
	}
	stream<<"|-----------|"<<std::endl;
  return stream;
}

void Matrix::fromOpenGL()
{
	glGetFloatv(GL_MODELVIEW_MATRIX, data_);
}

void Matrix::toOpenGL()
{
	glMultMatrixf(data_);
}

Matrix operator*(float number, const Matrix& mat){
	Matrix tmp = mat;
	return tmp*number;
}

Matrix Matrix::transpose()
{
	Matrix result;
	for(short i=0; i<4; i++)
	{
		for(short j=0; j<4; j++)
		{
			result.data_[i*4+j] = data_[i+4*j];		
		}
	}
	return result;
}

Matrix Matrix::inverse(){
#define SWAP_ROWS(a, b) { float *_tmp_ = a; a=b; b=_tmp_;}
  float tmp[4][8];
  float* m = data_;
  float* r0 = tmp[0]; float* r1 = tmp[1]; float* r2 = tmp[2]; float* r3 = tmp[3];
 /* 
  r0[0] = m[(0<<2)+0]; r0[1] = m[1<<2+0];
  r0[2] = m[(2<<2)+0]; r0[3] = m[3<<2+0];
  r0[4] = 1.0; r0[5] = r0[6] = r0[7] = 0.0;

  r1[0] = m[(0<<2)+1]; r1[1] = m[1<<2+1];
  r1[2] = m[(2<<2)+1]; r1[3] = m[3<<2+1];
  r1[5] = 1.0; r1[4] = r1[6] = r1[7] = 0.0;
  
  r2[0] = m[(0<<2)+2]; r2[1] = m[1<<2+2];
  r2[2] = m[(2<<2)+2]; r2[3] = m[3<<2+2];
  r2[6] = 1.0; r2[4] = r2[5] = r2[7] = 0.0;
  
  r3[0] = m[(0<<2)+3]; r3[1] = m[1<<2+3];
  r3[2] = m[(2<<2)+3]; r3[3] = m[3<<2+3];
  r3[7] = 1.0; r3[4] = r3[5] = r3[6] = 0.0;
*/
  r0[0] = m[0]; r0[1] = m[1];
  r0[2] = m[2]; r0[3] = m[3];
  r0[4] = 1.0; r0[5] = r0[6] = r0[7] = 0.0;

  r1[0] = m[4]; r1[1] = m[5];
  r1[2] = m[6]; r1[3] = m[7];
  r1[5] = 1.0; r1[4] = r1[6] = r1[7] = 0.0;
  
  r2[0] = m[8]; r2[1] = m[9];
  r2[2] = m[10]; r2[3] = m[11];
  r2[6] = 1.0; r2[4] = r2[5] = r2[7] = 0.0;
  
  r3[0] = m[12]; r3[1] = m[13];
  r3[2] = m[14]; r3[3] = m[15];
  r3[7] = 1.0; r3[4] = r3[5] = r3[6] = 0.0;
  //pivot
  if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
  if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
  if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
  if (r0[0] == 0)
    return Matrix(Identity);
  
  //first variable
  float m1 = r1[0]/r0[0]; float m2 = r2[0]/r0[0]; float m3 = r3[0]/r0[0];
  float s;
  s = r0[1]; r1[1] -= m1*s; r2[1] -= m2*s; r3[1] -= m3*s;
  s = r0[2]; r1[2] -= m1*s; r2[2] -= m2*s; r3[2] -= m3*s;
  s = r0[3]; r1[3] -= m1*s; r2[3] -= m2*s; r3[3] -= m3*s;
  s = r0[4];
  if (s != 0.0){
    r1[4] -= m1*s; r2[4] -= m2*s; r3[4] -= m3*s;
  }
  s = r0[5];
  if (s != 0.0){
    r1[5] -= m1*s; r2[5] -= m2*s; r3[5] -= m3*s;
  }
  s = r0[6];
  if (s != 0.0){
    r1[6] -= m1*s; r2[6] -= m2*s; r3[6] -= m3*s;
  }
  s = r0[7];
  if (s != 0.0){
    r1[7] -= m1*s; r2[7] -= m2*s; r3[7] -= m3*s;
  }
  
  //pivot
  if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
  if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
  if (r1[1] == 0)
    return Matrix(Identity);
  //second variable
  m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
  r2[2] -= m2*r1[2]; r3[2] -= m3*r1[2];
  r2[3] -= m2*r1[3]; r3[3] -= m3*r1[3];
  s = r1[4];
  if (s != 0.0){
    r2[4] -= m2*s; r3[4] -= m3*s;
  }
  s = r1[5];
  if (s != 0.0){
    r2[5] -= m2*s; r3[5] -= m3*s;
  }
  s = r1[6];
  if (s != 0.0){
    r2[6] -= m2*s; r3[6] -= m3*s;
  }
  s = r1[7];
  if (s != 0.0){
    r2[7] -= m2*s; r3[7] -= m3*s;
  }
  
  //pivot
  if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
  if (r2[2] == 0)
    return Matrix(Identity);
  //third variable
  m3 = r3[2]/r2[2];
  r3[3] -= m3*r2[3]; r3[4] -= m3*r2[4];
  r3[5] -= m3*r2[5]; r3[6] -= m3*r2[6];
  r3[7] -= m3*r2[7];

  if (r3[3] == 0.0)
    return Matrix(Identity);

  //back substitution
  //row 3
  s = 1.0/r3[3];
  r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;
  //row 2
  m2 = r2[3];
  s = 1.0/r2[2];
  r2[4] = s*(r2[4]-r3[4]*m2); r2[5] = s*(r2[5]-r3[5]*m2);
  r2[6] = s*(r2[6]-r3[6]*m2); r2[7] = s*(r2[7]-r3[7]*m2);
  m1 = r1[3];
  r1[4] -= r3[4]*m1; r1[5] -= r3[5]*m1;
  r1[6] -= r3[6]*m1; r1[7] -= r3[7]*m1;
  float m0 = r0[3];
  r0[4] -= r3[4]*m0; r0[5] -= r3[5]*m0;
  r0[6] -= r3[6]*m0; r0[7] -= r3[7]*m0;
  //row 1
  m1 = r1[2];
  s = 1.0/r1[1];
  r1[4] = s*(r1[4]-r2[4]*m1); r1[5] = s*(r1[5]-r2[5]*m1);
  r1[6] = s*(r1[6]-r2[6]*m1); r1[7] = s*(r1[7]-r2[7]*m1);
  m0 = r0[2];
  r0[4] -= r2[4]*m0; r0[5] -= r2[5]*m0;
  r0[6] -= r2[6]*m0; r0[7] -= r2[7]*m0;
  //row 0
  m0 = r0[1];
  s = 1.0/r0[0];
  r0[4] = s*(r0[4]-r1[4]*m0); r0[5] = s*(r0[5]-r1[5]*m0);
  r0[6] = s*(r0[6]-r1[6]*m0); r0[7] = s*(r0[7]-r1[7]*m0);

  Matrix ret;
  float* mat = ret.data_;
  /*
  mat[(0<<2)+0] = r0[4]; mat[(1<<2)+0] = r0[5];
  mat[(2<<2)+0] = r0[6]; mat[(3<<2)+0] = r0[7];
  mat[(0<<2)+1] = r1[4]; mat[(1<<2)+1] = r1[5];
  mat[(2<<2)+1] = r1[6]; mat[(3<<2)+1] = r1[7];
  mat[(0<<2)+2] = r2[4]; mat[(1<<2)+2] = r2[5];
  mat[(2<<2)+2] = r2[6]; mat[(3<<2)+2] = r2[7];
  mat[(0<<2)+3] = r3[4]; mat[(1<<2)+3] = r3[5];
  mat[(2<<2)+3] = r3[6]; mat[(3<<2)+3] = r3[7];
  */
  mat[0] = r0[4]; mat[1] = r0[5];
  mat[2] = r0[6]; mat[3] = r0[7];
  mat[4] = r1[4]; mat[5] = r1[5];
  mat[6] = r1[6]; mat[7] = r1[7];
  mat[8] = r2[4]; mat[9] = r2[5];
  mat[10] = r2[6]; mat[11] = r2[7];
  mat[12] = r3[4]; mat[13] = r3[5];
  mat[14] = r3[6]; mat[15] = r3[7];
  return ret;
#undef SWAP_ROWS
}
