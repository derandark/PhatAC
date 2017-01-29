

#include "StdAfx.h"
#include "MathLib.h"
#include "Frame.h"

ULONG Vec2D::pack_size()
{
	return(sizeof(float) * 2);
}

ULONG Vec2D::Pack(BYTE** ppData, ULONG iSize)
{
	ULONG PackSize = pack_size();

	if (iSize >= PackSize)
	{
		PACK(float, x);
		PACK(float, y);
	}

	return PackSize;
}

BOOL Vec2D::UnPack(BYTE** ppData, ULONG iSize)
{
	UNPACK(float, x);
	UNPACK(float, y);

	return TRUE;
}

Vector cross_product(const Vector& v1, const Vector& v2)
{
	return Vector(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

BOOL Vector::is_zero() const
{
	if ((fabs(x) >= F_EPSILON) || (fabs(y) >= F_EPSILON) || (fabs(z) >= F_EPSILON))
		return FALSE;

	return TRUE;
}

float Vector::dot_product(const Vector& v) const
{
	return((x * v.x) + (y * v.y) + (z * v.z));
}

BOOL Vector::normalize_check_small()
{
	float nfactor = magnitude();

	if (nfactor < F_EPSILON)
		return TRUE; // Too small.

	nfactor = 1 / nfactor;

	x *= nfactor;
	y *= nfactor;
	z *= nfactor;

	return FALSE;
}

Vector& Vector::normalize()
{
	float nfactor = 1 / magnitude();

	x *= nfactor;
	y *= nfactor;
	z *= nfactor;

	return *this;
}

BOOL Vector::IsValid() const
{
	if (_isnan(x) || _isnan(y) || _isnan(z))
		return FALSE;

	return TRUE;
}

BOOL Quaternion::IsValid() const
{
	if (_isnan(w) || _isnan(x) || _isnan(y) || _isnan(z))
		return FALSE;

	float magn = (w * w) + (x * x) + (y * y) + (z * z);

	if (_isnan(magn))
		return FALSE;

	if ((F_EPSILON * 5.0f) < fabs(magn - 1.0f))
		return FALSE;

	return TRUE;
}

void Quaternion::normalize()
{
	float magn = 1 / magnitude();

	w *= magn;
	x *= magn;
	y *= magn;
	z *= magn;
}

float Quaternion::dot_product(const Quaternion& q) const
{
	return((w * q.w) + (x * q.x) + (y * q.y) + (z * q.z));
}

Plane::Plane()
{
}

Plane::Plane(Vector& Vect1, Vector& Vect2)
{
	// Finish me
	__asm int 3;
}

float Plane::dot_product(const Vector& point)
{
	return m_normal.dot_product(point) + m_dist;
}

ULONG Plane::pack_size()
{
	return (m_normal.pack_size() + sizeof(float));
}

BOOL Plane::UnPack(BYTE** ppData, ULONG iSize)
{
	if (iSize < pack_size())
		return FALSE;

	// Plane Normal
	UNPACK_OBJ(m_normal);

	// Plane Distance
	UNPACK(float, m_dist);

	return TRUE;
}

BOOL Plane::compute_time_of_intersection(const Ray& ray, float *time)
{
	float dot = m_normal.dot_product(ray.m_direction);

	if (F_EPSILON > abs(dot))
		return FALSE;

	float depth = dot_product(ray.m_origin) * (-1.0 / dot);
	*time = depth;

	if (depth < 0.0f)
		return FALSE;

	return TRUE;
}

int Plane::which_side(const Vector& point, float near_dist)
{
	float dp = dot_product(point);

	if (dp > near_dist)
		return 0;

	if (dp < -near_dist)
		return 1;

	return 2;
}

