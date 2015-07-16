#pragma once

#include <math.h>
#include "aflStd.h"
#ifdef __ARM_NEON__
	#include <arm_neon.h>
#else
	#include <xmmintrin.h>
#endif

#define NPI 3.141592654f

#ifdef __ANDROID__
	struct SIZE
	{
		int cx,cy;
	};
	struct POINT
	{
		INT x,y;
	};

#endif

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// NVector2
// 2次元座標用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct NVector2
{
	float x,y;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// NVector3
// 3次元座標用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct NVector;
struct NVector3
{
	union
	{
		float vector4_f32[3];
		unsigned int vector4_u32[3];
		struct
		{
			float x,y,z;
		};
	};
	inline bool operator<(FLOAT f) const;
	inline bool operator>(FLOAT f) const;
	inline NVector3& operator=(const NVector f);
	inline NVector3& operator=(const NVector3 f);
	inline bool operator!=(const NVector3 v) const;
	inline NVector3 operator+(const NVector3& vect) const;
	inline NVector3 operator-(const NVector3& vect) const;
	inline NVector3& operator+=(NVector3& vect);
	inline NVector3& operator-=(NVector3& vect);
	inline bool operator < ( const NVector3& vector) const;
	inline NVector3 abs() const;
	inline NVector3 minimum(const NVector3 vect) const;
	inline NVector3 maximum(const NVector3& vect) const;
	inline NVector3 normal() const;
	inline FLOAT length() const;
};

typedef struct __vector4
{
	union
	{
		float vector4_f32[4];
		unsigned int vector4_u32[4];
	};
} __vector4;

#ifndef __ARM_NEON__
	typedef __vector4 float32x4_t;
	typedef float float32_t;
#endif

struct NVector;
inline NVector operator/(const float,const NVector& v);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// NVector
// 4次元座標用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct NVector
{
	union
	{
		float32x4_t v;
		struct
		{
			float x,y,z,w;
		};
		float p[4];
	};
	inline static NVector set(FLOAT x,FLOAT y,FLOAT z,FLOAT w);
	#ifdef __ARM_NEON__
	inline NVector& operator=(const uint32x4_t f);
	#endif
	inline NVector abs() const;
	inline bool operator==(const NVector& v) const;
	inline bool operator!=(const NVector& v) const;
	inline bool inBounds(const NVector& v) const;
	inline NVector& operator=(const NVector3& f);
	inline NVector less(const NVector& v) const;
	inline bool operator<(FLOAT f) const;
	inline bool operator>(FLOAT f) const;
	
	inline bool operator<(const NVector& v) const;
	inline bool operator>(const NVector& v) const;
	inline bool operator<=(const NVector& v) const;
	inline bool operator>=(const NVector& v) const;

#ifndef __ARM_NEON__
	inline NVector& operator=(const __m128 f);
	inline NVector& operator=(const __m128i f);
	inline operator __m128() const;
#else
	inline operator uint32x4_t() const;
#endif
	inline NVector& operator=(const float32x4_t f);
	inline NVector operator-() const;

	inline NVector operator/(float f) const;
	inline NVector operator*(float f) const;
	inline NVector operator+(float f) const;
	inline NVector operator-(float f) const;
	inline NVector operator+(const NVector& vect) const;
	inline NVector operator-(const NVector& vect) const;
	inline NVector operator*(const NVector& vect) const;
	inline NVector operator/(const NVector& vect) const;
	inline NVector& operator+=(const FLOAT f);
	inline NVector& operator-=(float f);
	inline NVector& operator/=(const FLOAT f);
	inline NVector& operator*=(float f);

	inline NVector& operator+=(const NVector& vect);
	inline NVector& operator-=(const NVector& vect);
	inline NVector& operator*=(const NVector& vect);
	inline NVector& operator/=(const NVector& vect);
	inline operator NVector3();
	inline operator float32x4_t() const;
	inline operator float*();

	inline NVector normal() const;
	inline FLOAT length() const;
	inline FLOAT length3() const;
	inline NVector normal3() const;

	inline void setZero();
	inline NVector sin() const;
	inline NVector cos() const;
	inline NVector minimum(const NVector& vect) const;
	inline NVector maximum(const NVector& vect) const;
	inline NVector quotanion() const;
	inline static NVector splatSignMask();

	inline NVector shiftRight1() const;
	inline NVector shiftRight2() const;
	inline NVector shiftRight3() const;

	inline NVector shiftLeft1() const;
	inline NVector shiftLeft2() const;
	inline NVector shiftLeft3() const;
	inline NVector operator ^(const NVector& vect) const;

	inline NVector slerpQuaternion(NVector& vect,float t) const;
	inline NVector splatX();
	inline NVector splatY();

	inline NVector select(const NVector& v1,const NVector& v2);

	inline NVector sqrt() const;
	inline NVector atan2(NVector v) const;

	inline NVector lerp(const NVector& vect,FLOAT f) const;
	inline NVector& operator=(FLOAT f);
	inline NVector cross(const NVector& vect) const;
	inline FLOAT dot(const NVector& vect) const;
	inline NVector negate() const;
	inline NVector transformNormal(struct NMatrix& matrix);
	inline NVector transformCoord(struct NMatrix& matrix);
	inline NVector project(FLOAT ViewportX,FLOAT ViewportY,FLOAT ViewportWidth,FLOAT ViewportHeight,
		FLOAT ViewportMinZ,FLOAT ViewportMaxZ,NMatrix Projection,struct NMatrix View,struct NMatrix World);
};
inline NVector operator/(const float f,const NVector& v)
{
	NVector result;
	result = f;
	result = result / v;
	return result;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// NMatrix
// 行列管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct NMatrix
{
	union
	{
		NVector r[4];
		float f[16];
		float m[4][4];
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};

	};
	inline bool operator==(const NMatrix& v) const;
	inline NMatrix operator+(const NMatrix& matrix) const;
	inline NMatrix operator*(const NMatrix& matrix) const;
	inline NMatrix& operator+=(const NMatrix& matrix);
	inline NMatrix& operator*=(NMatrix& matrix);
	inline NMatrix operator*(FLOAT f) const;
	static inline NMatrix multiply(const NMatrix& M1,const NMatrix& M2);

	inline FLOAT determinant() const;
	inline NVector quaternionRotationMatrix() const;
	inline void decompMatrix(NVector& p,NVector& s,NVector& q) const;
	inline NMatrix& setIdentity();
	inline NMatrix& setScaling(FLOAT x,FLOAT y,FLOAT z);
	inline NMatrix& setRotationRollPitchYaw(FLOAT x,FLOAT y,FLOAT z);
	inline NMatrix& setRotationX(float angle);
	inline NMatrix& setRotationY(float angle);
	inline NMatrix& setRotationZ(float angle);

	inline operator float*();
	inline NMatrix& setRotationQuaternion(const NVector& q);
	inline void compMatrix(const NVector& p,const NVector& s,const NVector& q);
	inline void changeMatrixCoordinate();
	inline NMatrix& setTranslation(FLOAT x,FLOAT y,FLOAT z);
	inline NMatrix& setTranspose();
	inline NMatrix& setInverse();
	inline NMatrix& setLookAtLH(const NVector& EyePosition,const NVector& FocusPosition,const NVector& UpDirection);
	inline NMatrix& setLookToLH(const NVector& EyePosition,const NVector& EyeDirection,const NVector& UpDirection);
	inline NMatrix& setPerspectiveFovLH(FLOAT FovAngleY,FLOAT AspectRatio,FLOAT NearZ,FLOAT FarZ);
	inline NMatrix& setLookAtRH(const NVector& EyePosition, const NVector& FocusPosition, const NVector& UpDirection);
	inline NMatrix& setLookToRH(const NVector& EyePosition, const NVector& EyeDirection, const NVector& UpDirection);
	inline NMatrix& setPerspectiveFovRH(FLOAT FovAngleY, FLOAT AspectRatio, FLOAT NearZ, FLOAT FarZ);
};



#include "aflMath.inl"
