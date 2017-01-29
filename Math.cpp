
#include "StdAfx.h"

#pragma warning(disable: 4244)

long RandomLong(long min, long max)
{
	float fraction = ((float)rand() / RAND_MAX);
	fraction *= ((max - min) + 1);
	long value = fraction + min;
	if (value > max)
		value = max;
	return value;
}

float RandomFloat(float min, float max)
{
	float fraction = ((float)rand() / RAND_MAX);
	fraction *= (max - min);
	float value = fraction + min;
	return value;
}

float FindVectorZ(const Vector& p1, const Vector& p2, const Vector& p3, float x, float y )
{
	Vector v1 = p3-p1;
	Vector v2 = p2-p1;
	Vector normal = CrossProduct( v1, v2 ).Normalize();

	float poo = -( (normal.x * p1.x) + (normal.y * p1.y) + (normal.z * p1.z) );
	float z = (-( (normal.x * x) + (normal.y * y) + poo) ) / normal.z; 

	return z;
}

/* Matrix courtesy of Asriel */
matrix::matrix() {
	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	data[2][3] = 0.0f;
	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = 0.0f;
	data[3][3] = 1.0f;
}

/* General definition */
void matrix::define(float xa, float xb, float xc, float xd, float ya, float yb, float yc, float yd, 
					float za, float zb, float zc, float zd) {
	data[0][0] = xa;
	data[0][1] = xb;
	data[0][2] = xc;
	data[0][3] = xd;
	data[1][0] = ya;
	data[1][1] = yb;
	data[1][2] = yc;
	data[1][3] = yd;
	data[2][0] = za;
	data[2][1] = zb;
	data[2][2] = zc;
	data[2][3] = zd;
}

/* Definition by Quaternion */
void matrix::defineByQuaternion(float qw, float qx, float qy, float qz) {
	data[0][0] = 1 - (2 * (qy * qy)) - (2 * (qz * qz));
	data[1][0] = (2 * qx * qy) - (2 * qw * qz);
	data[2][0] = (2 * qx * qz) + (2 * qw * qy);

	data[0][1] = (2 * qx * qy) + (2 * qw * qz);
	data[1][1] = 1 - (2 * (qx * qx)) - (2 * (qz * qz));
	data[2][1] = (2 * qy * qz) - (2 * qw * qx);

	data[0][2] = (2 * qx * qz) - (2 * qw * qy);
	data[1][2] = (2 * qy * qz) + (2 * qw * qx);
	data[2][2] = 1 - (2 * (qx * qx)) - (2 * (qy * qy));
}

/* Definition by Eulerian angular rotation */

#pragma warning(disable : 4244)

void matrix::defineByRotation(float roll, float pitch, float yaw) {
	double sr,sp,sy,cr,cp,cy;
	matrix mp, my;

	sr = sin(DEG2RAD(roll));
	cr = cos(DEG2RAD(roll));
	sp = sin(DEG2RAD(pitch));
	cp = cos(DEG2RAD(pitch));
	sy = sin(DEG2RAD(yaw));
	cy = cos(DEG2RAD(yaw));
                              
	data[0][0] = data[1][1] = cr;
	data[1][0] = -(data[0][1] = sr);

	mp.data[1][1] = mp.data[2][2] = cp;
	mp.data[1][2] = -(mp.data[2][1] = sp);

	my.data[0][0] = my.data[2][2] = cy;
	my.data[0][2] = -(my.data[2][0] = sy);

	multiply(my);
	multiply(mp);
}

void matrix::applyRotation(float roll, float pitch, float yaw) {
	matrix mat;

	mat.defineByRotation(roll, pitch, yaw);
	multiply(mat);
}

void matrix::applyTranslation(float x, float y, float z) {
	data[0][3] = x;
	data[1][3] = y;
	data[2][3] = z;
}

/* Apply this matrix to a vector */
void matrix::applyToVector(Vector &vect) {
	float xo, yo, zo, wo;
	xo = vect.x;
	yo = vect.y;
	zo = vect.z;
	wo = 1.0f;

	vect.x = xo * data[0][0] + yo * data[1][0] + zo * data[2][0] + wo * data[3][0];
	vect.y = xo * data[0][1] + yo * data[1][1] + zo * data[2][1] + wo * data[3][0];
	vect.z = xo * data[0][2] + yo * data[1][2] + zo * data[2][2] + wo * data[3][0];
}

void matrix::copy(matrix &dest) {
	dest.define(data[0][0], data[0][1], data[0][2], data[0][3],
				data[1][0], data[1][1], data[1][2], data[1][3],
				data[2][0], data[2][1], data[2][2], data[2][3]);
}

void matrix::multiply(matrix second)
{
	matrix temp;
	copy(temp);

	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 4; i++)
			data[i][j] = temp.data[i][0] * second.data[0][j] + 
						 temp.data[i][1] * second.data[1][j] + 
						 temp.data[i][2] * second.data[2][j] +
						 temp.data[i][3] * second.data[3][j];
}

// Checks if the sum of two 32-bit values will overflow.
void inline WillOF( DWORD v1, DWORD v2, BOOL *bResult )
{
#ifdef _WIN32
	__asm {
		mov eax, DWORD PTR [v1]
		add eax, DWORD PTR [v2]
		xor eax, eax
		adc eax, eax

		mov DWORD PTR [bResult], eax
	}
#else
	*bResult = TRUE;
#endif
}
