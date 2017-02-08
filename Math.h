
#pragma once
#include <math.h>

#pragma pack(push, 1)
typedef struct loc_s
{
	loc_s() {
		landcell = 0;
		x = 0;
		y = 0;
		z = 0;
	}
	loc_s(DWORD _landcell, float _x, float _y, float _z) {
		landcell = _landcell;
		x = _x;
		y = _y;
		z = _z;
	}

	//inline Vector2D operator/(vec_t fl) const { return Vector2D(x/fl, y/fl); }
	DWORD landcell;
	float x;
	float y;
	float z;
} loc_t;
#pragma pack(pop)

inline bool operator !(const loc_t &v) {
	return !v.landcell ? true : false;
}

#pragma pack(push, 1)
typedef struct heading_s
{
	heading_s() {
		w = 1;
		x = 0;
		y = 0;
		z = 0;
	}
	heading_s(float _w, float _x, float _y, float _z) {
		w = _w;
		x = _x;
		y = _y;
		z = _z;
	}
	float w;
	float x;
	float y;
	float z;
} heading_t;
#pragma pack(pop)

typedef struct placement_s
{
	loc_t origin;
	heading_t angles;
} placement_t;

extern DWORD RandomDWORD(DWORD min, DWORD max);
extern long RandomLong(long min, long max);
extern double RandomDouble(double min, double max);
extern float RandomFloat(float min, float max);
extern BOOL WillOF(unsigned long value_one, unsigned long value_two);

//Credits to Valve for a lot of this.
typedef double vec_t;
enum {
	PITCH = 0,
	YAW,
	ROLL
};

class Vector2D;
class Vector;

#define PI 3.1415926535897f
#define RAD2DEG( x ) ( (x) * (180.f / PI) )
#define DEG2RAD( x ) ( (x) * (PI / 180.f) )

class matrix {
public:
	matrix();
	void define(float xa, float xb, float xc, float xd, float ya, float yb, float yc, float yd,
		float za, float zb, float zc, float zd);
	void defineByRotation(float roll, float pitch, float yaw);
	void defineByQuaternion(float qw, float qx, float qy, float qz);
	void applyRotation(float roll, float pitch, float yaw);
	void applyTranslation(float x, float y, float z);
	void applyToVector(Vector &vect);
	void multiply(matrix second);
	void copy(matrix &dest);
	float data[4][4];
};

class Vector2D
{
public:
	inline Vector2D(void) { }
	inline Vector2D(vec_t X, vec_t Y) { x = X; y = Y; }
	inline Vector2D operator+(const Vector2D& v) const { return Vector2D(x + v.x, y + v.y); }
	inline Vector2D operator-(const Vector2D& v) const { return Vector2D(x - v.x, y - v.y); }
	inline Vector2D operator*(vec_t fl) const { return Vector2D(x*fl, y*fl); }
	inline Vector2D operator/(vec_t fl) const { return Vector2D(x / fl, y / fl); }

	inline vec_t Length(void) const { return sqrt(x*x + y*y); }

	inline Vector2D Normalize(void) const
	{
		Vector2D vec2;

		vec_t flLen = Length();
		if (flLen == 0)
		{
			return Vector2D(0, 0);
		}
		else
		{
			flLen = 1 / flLen;
			return Vector2D(x * flLen, y * flLen);
		}
	}

	vec_t x, y;
};

inline vec_t DotProduct(const Vector2D& a, const Vector2D& b)
{
	return(a.x*b.x + a.y*b.y);
}

inline Vector2D operator*(vec_t fl, const Vector2D& v)
{
	return v * fl;
}

class Vector
{
public:
	// Construction/destruction
	inline Vector(void) { }
	inline Vector(vec_t X, vec_t Y, vec_t Z) { x = X; y = Y; z = Z; }
	//inline Vector(double X, double Y, double Z) { x = (float)X; y = (float)Y; z = (float)Z; }
	//inline Vector(int X, int Y, int Z) { x = (float)X; y = (float)Y; z = (float)Z; }
	inline Vector(const Vector& v) { x = v.x; y = v.y; z = v.z; }
	inline Vector(vec_t rgfl[3]) { x = rgfl[0]; y = rgfl[1]; z = rgfl[2]; }
	inline Vector(loc_t origin)
	{
		x = ((origin.landcell >> 24) * 192.0f) + origin.x;
		y = (((origin.landcell >> 16) & 0xFF) * 192.0f) + origin.y;
		z = origin.z;
	}
	inline Vector(float X, float Y, float Z, float W)
	{
		//matrix poop;
		//poop.defineByQuaternion( X, Y, Z, W );
		//poop.applyToVector(this);
	}

	// Operators
	inline Vector operator-(void) const { return Vector(-x, -y, -z); }
	inline int operator==(const Vector& v) const { return x == v.x && y == v.y && z == v.z; }
	inline int operator!=(const Vector& v) const { return !(*this == v); }
	inline Vector operator+(const Vector& v) const { return Vector(x + v.x, y + v.y, z + v.z); }
	inline Vector operator-(const Vector& v) const { return Vector(x - v.x, y - v.y, z - v.z); }
	inline Vector operator*(vec_t fl) const { return Vector(x*fl, y*fl, z*fl); }
	inline Vector operator/(vec_t fl) const { return Vector(x / fl, y / fl, z / fl); }

	// Methods
	inline void CopyToArray(vec_t* rgfl) const { rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; }
	inline vec_t Length(void) const { return sqrt(x*x + y*y + z*z); }
	operator vec_t *() { return &x; } // Vectors will now automatically convert to float * when needed
	operator const vec_t *() const { return &x; } // Vectors will now automatically convert to float * when needed
	inline Vector Normalize(void) const
	{
		vec_t flLen = Length();
		if (flLen == 0) return Vector(0, 0, 1); // ????
		flLen = 1 / flLen;
		return Vector(x * flLen, y * flLen, z * flLen);
	}

	inline Vector2D Make2D(void) const
	{
		Vector2D Vec2;

		Vec2.x = x;
		Vec2.y = y;

		return Vec2;
	}
	inline vec_t Length2D(void) const { return sqrt(x*x + y*y); }

	// Members
	vec_t x, y, z;
};

inline Vector operator*(vec_t fl, const Vector& v)
{
	return v * fl;
}

inline vec_t DotProduct(const Vector& a, const Vector& b)
{
	return(a.x*b.x + a.y*b.y + a.z*b.z);
}
inline Vector CrossProduct(const Vector& a, const Vector& b)
{
	return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

float FindVectorZ(const Vector& p1, const Vector& p2, const Vector& p3, float x, float y);

void inline SinCos(float radians, float *sine, float *cosine)
{
#ifdef _WIN32
	_asm
	{
		fld DWORD PTR[radians]
		fsincos

		mov edx, DWORD PTR[cosine]
		mov eax, DWORD PTR[sine]

		fstp DWORD PTR[edx]
		fstp DWORD PTR[eax]
	}
#elif _LINUX
	register double __cosr, __sinr;
	__asm __volatile__
	("fsincos"
		: "=t" (__cosr), "=u" (__sinr) : "0" (radians));

	*sine = __sinr;
	*cosine = __cosr;
#endif
}

inline void AngleVectors(const Vector &angles, Vector *forward)
{
	float sp, sy, cp, cy;

	SinCos((float)DEG2RAD(angles[YAW]), &sy, &cy);
	SinCos((float)DEG2RAD(angles[PITCH]), &sp, &cp);

	forward->x = cp*cy;
	forward->y = cp*sy;
	forward->z = -sp;
}
