#pragma once
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// NVector3
// 3次元座標用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool NVector3::operator<(FLOAT f) const
{
	if(x < f && y < f && z < f)
		return true;
	return false;
}
bool NVector3::operator>(FLOAT f) const
{
	if(x > f && y > f && z > f)
		return true;
	return false;
}
NVector3& NVector3::operator=(const NVector f)
{
	x = f.x;
	y = f.y;
	z = f.z;
	return *this;
}

NVector3& NVector3::operator=(const NVector3 f)
{
	x = f.x;
	y = f.y;
	z = f.z;
	return *this;
}
bool NVector3::operator!=(const NVector3 v) const
{
	if(x != v.x || y != v.y || z != v.z)
		return true;
	return false;
}
NVector3 NVector3::operator+(const NVector3& vect) const
{
	NVector3 result = {  x + vect.x, y + vect.y, z + vect.z };
	return result;
}
NVector3 NVector3::operator-(const NVector3& vect) const
{
	NVector3 result = {x-vect.x,y-vect.y,z-vect.z};
	return result;
}
NVector3& NVector3::operator+=(NVector3& vect)
{
	*this = *this + vect;
	return *this;
}
NVector3& NVector3::operator-=(NVector3& vect)
{
	*this = *this - vect;
	return *this;
}
bool NVector3::operator < ( const NVector3& vector) const
{
	if(x < vector.x)
		return true;
	if(x == vector.x)
	{
		if(y < vector.y)
			return true;
		if(y == vector.y && z < vector.z)
			return true;
	}
	return false;
}
NVector3 NVector3::abs() const
{
	NVector3 result = {::abs(x),::abs(y),::abs(z)};
	return result;
}
NVector3 NVector3::minimum(const NVector3 vect) const
{
	NVector3 result;
	NVector v1,v2;
	v1 = *this;
	v2 = vect;
	result = v1.minimum(v2);
	return result;
}
NVector3 NVector3::maximum(const NVector3& vect) const
{
	NVector3 result;
	NVector v1,v2;
	v1 = *this;
	v2 = vect;
	result = v1.maximum(v2);
	return result;
}
FLOAT NVector3::length() const
{
	return sqrtf(x*x + y*y + z*z);
}
NVector3 NVector3::normal() const
{
	NVector result;
	FLOAT len = length();
	result.x = x/len;
	result.y = y/len;
	result.z = z/len;
	return result;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// NVector
// 4次元座標用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
NVector NVector::set(FLOAT x,FLOAT y,FLOAT z,FLOAT w)
{
#ifdef __ARM_NEON__
	NVector result = { x, y, z, w };
	return result; 
#else
	NVector result;
	result = _mm_set_ps(w, z, y, x);
	return result;
#endif
}
NVector& NVector::operator=(const float32x4_t f)
{
	v = f;
	return *this;
}
#ifdef __ARM_NEON__
NVector& NVector::operator=(const uint32x4_t f)
{
	*(uint32x4_t*)&v = f;
	return *this;
}
#endif
NVector NVector::abs() const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vabsq_f32(*this);
	#else
		result = _mm_andnot_ps(_mm_set1_ps(-0.0f), *this);
	#endif
	return result;
}
#define _getUINT(a) (*(UINT*)&a)
#define _getINT(a) (*(INT*)&a)


bool NVector::operator==(const NVector& v) const
{
	#ifdef __ARM_NEON__
		uint32x4_t r = vceqq_f32( *this, v );
		if(((LPDWORD)&r)[0] & ((LPDWORD)&r)[1] & ((LPDWORD)&r)[2] & ((LPDWORD)&r)[3] != 0)
			return true;
		return false;
	#else
		INT p = _mm_movemask_epi8(_mm_castps_si128(_mm_cmpeq_ss(*this, v)));
		return p == 0xffff;
	#endif
}
bool NVector::operator!=(const NVector& v) const
{
	#ifdef __ARM_NEON__
		uint32x4_t r = vceqq_f32( *this, v );
		if(((LPDWORD)&r)[0] & ((LPDWORD)&r)[1] & ((LPDWORD)&r)[2] & ((LPDWORD)&r)[3] == 0)
			return true;
		return false;
	#else
		INT p = _mm_movemask_epi8(_mm_castps_si128(_mm_cmpneq_ss(*this, v)));
		return p == 0xffff;
	#endif
}
bool NVector::inBounds(const NVector& v) const
{
	if(*this >= -v && *this <= v)
		return true;
	return false;
}
bool NVector::operator<(const NVector& v) const
{
	#ifdef __ARM_NEON__
		uint32x4_t r = vcltq_f32( *this, v );
		if(((LPDWORD)&r)[0] & ((LPDWORD)&r)[1] & ((LPDWORD)&r)[2] & ((LPDWORD)&r)[3])
			return true;
		return false;
	#else
		INT p = _mm_movemask_epi8(_mm_castps_si128(_mm_cmplt_ps(*this, v)));
		return p == 0xffff;
	#endif

}
bool NVector::operator>(const NVector& v) const
{
	#ifdef __ARM_NEON__
		uint32x4_t r = vcgtq_f32( *this, v );
		if (((LPDWORD)&r)[0] & ((LPDWORD)&r)[1] & ((LPDWORD)&r)[2] & ((LPDWORD)&r)[3])
			return true;
		return false;
	#else
		INT p = _mm_movemask_epi8(_mm_castps_si128(_mm_cmpgt_ps(*this, v)));
		return p == 0xffff;
	#endif
}
bool NVector::operator<=(const NVector& v) const
{
	#ifdef __ARM_NEON__
		uint32x4_t r = vcleq_f32( *this, v );
	#else
		__m128 r = _mm_cmple_ps( *this, v );
	#endif
	if(((LPDWORD)&r)[0] & ((LPDWORD)&r)[1] & ((LPDWORD)&r)[2] & ((LPDWORD)&r)[3])
		return true;
	return false;
}
bool NVector::operator>=(const NVector& v) const
{
	#ifdef __ARM_NEON__
		uint32x4_t r = vcgeq_f32( *this, v );
	#else
		__m128 r = _mm_cmpge_ps( *this, v );
	#endif
	if(((LPDWORD)&r)[0] & ((LPDWORD)&r)[1] & ((LPDWORD)&r)[2] & ((LPDWORD)&r)[3])
		return true;
	return false;
}

NVector NVector::operator-() const
{
	return negate();
}
NVector& NVector::operator=(const NVector3& f)
{
	x = f.x;
	y = f.y;
	z = f.z;
	w = 0.0f;
	return *this;
}
NVector NVector::less(const NVector& v) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vcltq_f32( *this, v );
	#else
		result = _mm_cmplt_ps( *this, v );
	#endif
	return result;
}


bool NVector::operator<(FLOAT f) const
{
	NVector v;
	v = f;
	if(*this < v)
		return true;
	return false;
}
bool NVector::operator>(FLOAT f) const
{
	NVector v;
	v = f;
	if(*this > v)
		return true;
	return false;
}
#ifndef __ARM_NEON__
NVector& NVector::operator=(const __m128 f)
{
	_mm_storeu_ps((float*)&v,f);
	return *this;
}
NVector& NVector::operator=(const __m128i f)
{
	*this = *reinterpret_cast<const __m128*>(&f);
	return *this;
}
NVector::operator __m128() const
{
	return _mm_loadu_ps((float*)&v);
}
#else
NVector::operator uint32x4_t() const
{
	return *(uint32x4_t*)&v;
}
#endif

NVector NVector::operator/(float f) const
{
	NVector result;
	#ifdef __ARM_NEON__
		float32_t v2 = 1.0f / f;
		result =vmulq_n_f32(*this,v2);
	#else
		float v = 1.0f / f;
		__m128 v2 = _mm_load1_ps(&v);
		result = _mm_mul_ps(*this,v2);
	#endif
	return result;
}
NVector NVector::operator*(float f) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result =vmulq_n_f32(*this,f);
	#else
		__m128 v2 = _mm_load1_ps(&f);
		result = _mm_mul_ps(*this,v2);
	#endif
	return result;
}
NVector NVector::operator+(float f) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vmovq_n_f32(f);
		result = vaddq_f32(*this,result);
	#else
		__m128 v2 = _mm_load1_ps(&f);
		result = _mm_add_ps(*this,v2);
	#endif
	return result;
}
NVector NVector::operator-(float f) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vmovq_n_f32(f);
		result = vsubq_f32(*this,result);
	#else
		__m128 v2 = _mm_load1_ps(&f);
		result = _mm_sub_ps(*this,v2);
	#endif
	return result;
}
NVector NVector::operator+(const NVector& vect) const
{
	NVector result;
#ifdef __ARM_NEON__
		result = vaddq_f32(v,vect.v);
#else
		result = _mm_add_ps(*this,vect);
#endif
	return result;
}

NVector NVector::operator-(const NVector& vect) const
{
	NVector result;
	#ifdef __ARM_NEON__
			result = vsubq_f32(v,vect.v);
	#else
			result = _mm_sub_ps(*this,vect);
	#endif
	return result;
}

NVector NVector::operator*(const NVector& vect) const
{
	NVector result;
	#ifdef __ARM_NEON__
			result = vmulq_f32(v,vect.v);
	#else
			result = _mm_mul_ps(*this,vect);
	#endif
	return result;
}

NVector NVector::operator/(const NVector& vect) const
{
	NVector result;
	#ifdef __ARM_NEON__
		float32x4_t v2 = vrecpeq_f32(vect);
		result = vmulq_f32(v,v2);
	#else
		result = _mm_div_ps(*this,vect);
	#endif
	return result;
}
NVector& NVector::operator+=(const FLOAT f)
{
	*this = *this + f;
	return *this;
}
NVector& NVector::operator-=(float f)
{
	*this = *this - f;
	return *this;
}
NVector& NVector::operator/=(const FLOAT f)
{
	*this = *this / f;
	return *this;
}
NVector& NVector::operator*=(float f)
{
	*this = *this * f;
	return *this;
}

NVector& NVector::operator+=(const NVector& vect)
{
	*this = *this + vect;
	return *this;
}
NVector& NVector::operator-=(const NVector& vect)
{
	*this = *this - vect;
	return *this;
}
NVector& NVector::operator*=(const NVector& vect)
{
	*this = *this * vect;
	return *this;
}
NVector& NVector::operator/=(const NVector& vect)
{
	*this = *this / vect;
	return *this;
}
NVector::operator NVector3()
{
	return *(NVector3*)this;
}
NVector::operator float32x4_t() const
{
	return v;
}
NVector::operator float*()
{
	return (float*)&v;
}

NVector NVector::normal() const
{
	NVector result;
	FLOAT len = length();
	result = *this/len;
	return result;
}
FLOAT NVector::length() const
{
	return sqrtf(x*x + y*y + z*z + w*w);
}
FLOAT NVector::length3() const
{
	return sqrtf(x*x + y*y + z*z);
}
NVector NVector::normal3() const
{
	NVector result;
	FLOAT len = length3();
	result.x = x/len;
	result.y = y/len;
	result.z = z/len;
	result.w = w;
	return result;
}

void NVector::setZero()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}
NVector NVector::sin() const
{
	NVector v;
	v.x = sinf(x);
	v.y = sinf(y);
	v.z = sinf(z);
	v.w = sinf(w);
	return v;
}
NVector NVector::cos() const
{
	NVector v;
	v.x = cosf(x);
	v.y = cosf(y);
	v.z = cosf(z);
	v.w = cosf(w);
	return v;
}
NVector NVector::minimum(const NVector& vect) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vminq_f32(*this,vect);
	#else
		result = _mm_min_ps(*this,vect);
	#endif
	return result;
}
NVector NVector::maximum(const NVector& vect) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vmaxq_f32(*this,vect);
	#else
		result = _mm_max_ps(*this,vect);
	#endif

	return result;
}

NVector NVector::quotanion() const
{
	NVector result;
	NVector angle = *this;
	NVector m = { 1.0f, -1.0f, -1.0f, 1.0f };
	angle *= 0.5f;
	NVector sin = angle.sin();
	NVector cos = angle.cos();
	NVector qx0 = { sin.x, cos.x, cos.x, cos.x};
	NVector qy0 = {cos.y,sin.y,cos.y,cos.y};
	NVector qz0 = {cos.z,cos.z,sin.z,cos.z};
	NVector qx1 = { cos.x, sin.x, sin.x, sin.x };
	NVector qy1 = { sin.y, cos.y, sin.y, sin.y };
	NVector qz1 = { sin.z, sin.z, cos.z, sin.z };
	
	NVector q1 = qx1 * m;
	NVector q0 = qx0 * qy0;
	q1 = q1 * qy1;
	q0 = q0 * qz0;

	q1 = q1*qz1;
	result = q0+q1;
	return result;
}
NVector NVector::splatSignMask()
{
	#ifdef __ARM_NEON__
		const UINT data[] = {0x80000000,0x80000000,0x80000000,0x80000000};
		NVector result;
		memcpy(&result,data,sizeof(data));
		return result;
	#else
		NVector result;
		__m128i V = _mm_set1_epi32( 0x80000000 );
		result = reinterpret_cast<__m128*>(&V)[0];
		return result;
	#endif
}
/*
NVector operator >>(const int count) const
{
	__m128 w = *this;
	NVector result;
	__m128i V = _mm_srli_epi32(*reinterpret_cast<__m128i*>(&w),count);
	result = reinterpret_cast<__m128*>(&V)[0];
	return result;
}
NVector operator <<(const int count) const
{
	__m128 w = *this;
	NVector result;
	__m128i V = _mm_slli_epi32(*reinterpret_cast<__m128i*>(&w),count);
	result = reinterpret_cast<__m128*>(&V)[0];
	return result;
}
*/
NVector NVector::shiftRight1() const
{
	NVector result = {0,x,y,z};
	return result;
}
NVector NVector::shiftRight2() const
{
	NVector result = {0,0,x,y};
	return result;
}
NVector NVector::shiftRight3() const
{
	NVector result = {0,0,0,x};
	return result;
}

NVector NVector::shiftLeft1() const
{
	NVector result = {y,z,w,0};
	return result;
}
NVector NVector::shiftLeft2() const
{
	NVector result = {z,w,0,0};
	return result;
}
NVector NVector::shiftLeft3() const
{
	NVector result = {w,0,0,0};
	return result;
}
NVector NVector::operator ^(const NVector& vect) const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = veorq_u32(*this,vect);
	#else
		__m128 v1 = *this;
		__m128 v2 = vect;
		result = _mm_xor_si128(*reinterpret_cast<__m128i*>(&v1),*reinterpret_cast<__m128i*>(&v2));
	#endif
	return result;
}

NVector NVector::slerpQuaternion(NVector& vect,float t) const
{
	NVector T;
	T = t;
	NVector result;

	static const NVector one = {1,1,1,1};
	static const NVector none = {-1,-1,-1,-1};
	static const NVector ome = {1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f};

	NVector co,so;
	NVector zero;
	NVector less;
	NVector s;
	NVector omega;

	zero.setZero();

	co = dot(vect);
	less = co.less(zero);
	s = less.select(one,none);
	co *= s;

	less = co.less(ome);
	so = one - co * co;
	so = so.sqrt();
	omega = so.atan2(co);

	NVector sm = NVector::splatSignMask();
	NVector c1000 = {1,0,0,0};


	NVector v01 = T.shiftLeft2();
	sm = sm.shiftLeft3();
	v01 = v01^sm;
	v01 += c1000;

	NVector iso;
	iso = 1.0f/so;

	NVector s1;
	NVector s0 = v01*omega;
	s0 = s0.sin();
	s0 = s0 * iso;
	s0 = less.select(v01,s0);

	s1 = s0.splatY();
	s0 = s0.splatX();

	s1 = s1*s;

	result = *this*s0 + vect * s1;

	return result;

}
NVector NVector::splatX()
{
	NVector result;
	#ifdef __ARM_NEON__
	result = vmovq_n_f32(x);
	#else
		result = _mm_shuffle_ps( *this, *this, _MM_SHUFFLE(0, 0, 0, 0) );
	#endif
	return result;
}
NVector NVector::splatY()
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vmovq_n_f32(y);
	#else
		result = _mm_shuffle_ps( *this, *this, _MM_SHUFFLE(1, 1, 1, 1) );
	#endif
	return result;
}

NVector NVector::select(const NVector& v1,const NVector& v2)
{
	NVector result;

	#ifdef __ARM_NEON__
		NVector t1;
		NVector t2;
	    t1 = vmvnq_u32(*this);
	    t1 = vandq_u32(t1,v1);
	    t2 = vandq_u32(v2,*this);
	    result = vorrq_u32(t1,t2);
	#else
	    __m128 t1 = _mm_andnot_ps(*this,v1);
	    __m128 t2 = _mm_and_ps(v2,*this);
	    result = _mm_or_ps(t1,t2);
	#endif
	return result;
}

NVector NVector::sqrt() const
{
	NVector result;
	#ifdef __ARM_NEON__
		result = vrsqrteq_f32(*this);
		result = vrecpeq_f32(result);
	#else
		result = _mm_sqrt_ps(*this);
	#endif
	return result;
}
NVector NVector::atan2(NVector v) const
{
	NVector result;
	result.x = atan2f(x,v.x);
	result.y = atan2f(y,v.y);
	result.z = atan2f(z,v.z);
	result.w = atan2f(w,v.w);

	return result;
}

NVector NVector::lerp(const NVector& vect,FLOAT f) const
{
	NVector result;
	result = *this + (vect-*this) * f;
	return result;
}
NVector& NVector::operator=(FLOAT f)
{
	x = f;
	y = f;
	z = f;
	w = f;
	return *this;
}
//外積
NVector NVector::cross(const NVector& vect) const
{
	NVector result =
	{
		y*vect.z - z * vect.y,
		z*vect.x - x * vect.z,
		x*vect.y - y * vect.x,
		0.0f
	};
	return result;
};

//内積
FLOAT NVector::dot(const NVector& vect) const
{
	NVector result = *this * vect;
	FLOAT f = result.x + result.y + result.z + result.w;
	return f;
}
NVector NVector::negate() const
{
	NVector result = {-x,-y,-z,-w};
	return result;
}

NVector NVector::transformNormal(struct NMatrix& matrix)
{
	NVector result;
	NVector tx,ty,tz;
	tx = x;
	ty = y;
	tz = z;
	result =  tx * matrix.r[0];
	result += ty * matrix.r[1];
	result += tz * matrix.r[2];

	return result;
}
 NVector NVector::transformCoord(struct NMatrix& matrix)
 {
	NVector result;
	NVector tx,ty,tz;
	tx = x;
	ty = y;
	tz = z;
	result = matrix.r[3];
	result += tx * matrix.r[0];
	result += ty * matrix.r[1];
	result += tz * matrix.r[2];
	result /= result.w;
	return result;

 }

NVector NVector::project(FLOAT ViewportX,FLOAT ViewportY,FLOAT ViewportWidth,FLOAT ViewportHeight,
	FLOAT ViewportMinZ,FLOAT ViewportMaxZ,NMatrix Projection,struct NMatrix View,struct NMatrix World)
{
	FLOAT    HalfViewportWidth = ViewportWidth * 0.5f;
	FLOAT    HalfViewportHeight = ViewportHeight * 0.5f;

	NVector Scale = {HalfViewportWidth,-HalfViewportHeight,ViewportMaxZ - ViewportMinZ,0.0f};
	NVector Offset = {ViewportX + HalfViewportWidth,ViewportY + HalfViewportHeight,ViewportMinZ,0.0f};

	NMatrix Transform = World*View;
	Transform *= Projection;

	NVector result = transformCoord(Transform);
	result += Scale * Offset;

	return result;
}


bool NMatrix::operator==(const NMatrix& v) const
{
	return memcmp(this,&v,sizeof(NMatrix)) == 0;
}

NMatrix NMatrix::operator+(const NMatrix& matrix) const
{
	NMatrix result;
	result.r[0] = r[0] + matrix.r[0];
	result.r[1] = r[1] + matrix.r[1];
	result.r[2] = r[2] + matrix.r[2];
	result.r[3] = r[3] + matrix.r[3];

	return result;
}
NMatrix NMatrix::operator*(const NMatrix& matrix) const
{
	return multiply(*this,matrix);
}
NMatrix& NMatrix::operator+=(const NMatrix& matrix)
{
	*this = *this + matrix;
	return *this;
}

NMatrix& NMatrix::operator*=(NMatrix& matrix)
{
	return *this = multiply(*this,matrix);
}


NMatrix NMatrix::operator*(FLOAT f) const
{
	NMatrix result;
	result.r[0] = r[0] * f;
	result.r[1] = r[1] * f;
	result.r[2] = r[2] * f;
	result.r[3] = r[3] * f;

	return result;
}
#ifdef __ARM_NEON__
NMatrix NMatrix::multiply(const NMatrix& M1,const NMatrix& M2)
{
	NMatrix work;

	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT w;

	x = M1.m[0][0];
	y = M1.m[0][1];
	z = M1.m[0][2];
	w = M1.m[0][3];
	work.r[0] = vmulq_n_f32(M2.r[0],x);
	work.r[0] = vmlaq_n_f32(work.r[0],M2.r[1],y);
	work.r[0] = vmlaq_n_f32(work.r[0],M2.r[2],z);
	work.r[0] = vmlaq_n_f32(work.r[0],M2.r[3],w);

	x = M1.m[1][0];
	y = M1.m[1][1];
	z = M1.m[1][2];
	w = M1.m[1][3];
	work.r[1] = vmulq_n_f32(M2.r[0],x);
	work.r[1] = vmlaq_n_f32(work.r[1],M2.r[1],y);
	work.r[1] = vmlaq_n_f32(work.r[1],M2.r[2],z);
	work.r[1] = vmlaq_n_f32(work.r[1],M2.r[3],w);

	x = M1.m[2][0];
	y = M1.m[2][1];
	z = M1.m[2][2];
	w = M1.m[2][3];
	work.r[2] = vmulq_n_f32(M2.r[0],x);
	work.r[2] = vmlaq_n_f32(work.r[2],M2.r[1],y);
	work.r[2] = vmlaq_n_f32(work.r[2],M2.r[2],z);
	work.r[2] = vmlaq_n_f32(work.r[2],M2.r[3],w);

	x = M1.m[3][0];
	y = M1.m[3][1];
	z = M1.m[3][2];
	w = M1.m[3][3];
	work.r[3] = vmulq_n_f32(M2.r[0],x);
	work.r[3] = vmlaq_n_f32(work.r[3],M2.r[1],y);
	work.r[3] = vmlaq_n_f32(work.r[3],M2.r[2],z);
	work.r[3] = vmlaq_n_f32(work.r[3],M2.r[3],w);

	return work;
}
#else
NMatrix NMatrix::multiply(const NMatrix& M1,const NMatrix& M2)
{
	NMatrix work;

	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT w;

	x = M1.m[0][0];
	y = M1.m[0][1];
	z = M1.m[0][2];
	w = M1.m[0][3];
	work.r[0] = M2.r[0] * x;
	work.r[0] += M2.r[1] * y;
	work.r[0] += M2.r[2] * z;
	work.r[0] += M2.r[3] * w;

	x = M1.m[1][0];
	y = M1.m[1][1];
	z = M1.m[1][2];
	w = M1.m[1][3];
	work.r[1] = M2.r[0] * x;
	work.r[1] += M2.r[1] * y;
	work.r[1] += M2.r[2] * z;
	work.r[1] += M2.r[3] * w;

	x = M1.m[2][0];
	y = M1.m[2][1];
	z = M1.m[2][2];
	w = M1.m[2][3];
	work.r[2] = M2.r[0] * x;
	work.r[2] += M2.r[1] * y;
	work.r[2] += M2.r[2] * z;
	work.r[2] += M2.r[3] * w;

	x = M1.m[3][0];
	y = M1.m[3][1];
	z = M1.m[3][2];
	w = M1.m[3][3];
	work.r[3] = M2.r[0] * x;
	work.r[3] += M2.r[1] * y;
	work.r[3] += M2.r[2] * z;
	work.r[3] += M2.r[3] * w;

	return work;
}
#endif
FLOAT NMatrix::determinant() const
{
	FLOAT f =
	+ m[0][0] * m[1][1] * m[2][2] * m[3][3]
	- m[0][0] * m[1][1] * m[2][3] * m[3][2]
	- m[0][0] * m[2][1] * m[1][2] * m[3][3]
	+ m[0][0] * m[2][1] * m[1][3] * m[3][2]
	+ m[0][0] * m[3][1] * m[1][2] * m[2][3]
	- m[0][0] * m[3][1] * m[1][3] * m[2][2]
	- m[1][0] * m[0][1] * m[2][2] * m[3][3]
	+ m[1][0] * m[0][1] * m[2][3] * m[3][2]
	+ m[1][0] * m[2][1] * m[0][2] * m[3][3]
	- m[1][0] * m[2][1] * m[0][3] * m[3][2]
	- m[1][0] * m[3][1] * m[0][2] * m[2][3]
	+ m[1][0] * m[3][1] * m[0][3] * m[2][2]
	+ m[2][0] * m[0][1] * m[1][2] * m[3][3]
	- m[2][0] * m[0][1] * m[1][3] * m[3][2]
	- m[2][0] * m[1][1] * m[0][2] * m[3][3]
	+ m[2][0] * m[1][1] * m[0][3] * m[3][2]
	+ m[2][0] * m[3][1] * m[0][2] * m[1][3]
	- m[2][0] * m[3][1] * m[0][3] * m[1][2]
	- m[3][0] * m[0][1] * m[1][2] * m[2][3]
	+ m[3][0] * m[0][1] * m[1][3] * m[2][2]
	+ m[3][0] * m[1][1] * m[0][2] * m[2][3]
	- m[3][0] * m[1][1] * m[0][3] * m[2][2]
	- m[3][0] * m[2][1] * m[0][2] * m[1][3]
	+ m[3][0] * m[2][1] * m[0][3] * m[1][2];
	return f;
}
NVector NMatrix::quaternionRotationMatrix() const
{
	NVector q;

	// 最大成分を検索
	float elem[ 4 ]; // 0:x, 1:y, 2:z, 3:w
	elem[ 0 ] =  _11 - _22 - _33 + 1.0f;
	elem[ 1 ] = -_11 + _22 - _33 + 1.0f;
	elem[ 2 ] = -_11 - _22 + _33 + 1.0f;
	elem[ 3 ] =  _11 + _22 + _33 + 1.0f;

	unsigned biggestIndex = 0;
	for ( int i = 1; i < 4; i++ )
	{
		if ( elem[i] > elem[biggestIndex] )
			biggestIndex = i;
	}


	// 最大要素の値を算出
	float v = sqrtf( elem[biggestIndex] ) * 0.5f;
	float mult = 0.25f / v;

	switch ( biggestIndex ) {
	case 0: // x
		q.x = v;
		q.y = (_12 + _21) * mult;
		q.z = (_31 + _13) * mult;
		q.w = -(_23 - _32) * mult;
		break;
	case 1: // y
		q.x = (_12 + _21) * mult;
		q.y = v;
		q.z = (_23 + _32) * mult;
		q.w = -(_31 - _13) * mult;
		break;
	case 2: // z
		q.x = (_31 + _13) * mult;
		q.y = (_23 + _32) * mult;
		q.z = v;
		q.w = -(_12 - _21) * mult;
		break;
	case 3: // w
		q.x = (_23 - _32) * mult;
		q.y = (_31 - _13) * mult;
		q.z = (_12 - _21) * mult;
		q.w = -v;
		break;
	}
	return q;
}
void NMatrix::decompMatrix(NVector& p,NVector& s,NVector& q) const
{
	p = r[3];
	s.x = sqrtf(_11*_11+_12*_12+_13*_13);
	s.y = sqrtf(_21*_21+_22*_22+_23*_23);
	s.z = sqrtf(_31*_31+_32*_32+_33*_33);
	s.w = 0.0f;

	if(determinant() < 0)
	{
		s.x = -s.x;
		s.y = -s.y;
		s.z = -s.z;
	}
	NMatrix mat;
	INT i;
	for(i=0;i<4;i++)
	{
		mat.r[0] = r[0] / s.x;
		mat.r[1] = r[1] / s.y;
		mat.r[2] = r[2] / s.z;
	}
	mat.r[3].x =0.0f;
	mat.r[3].y =0.0f;
	mat.r[3].z =0.0f;
	mat.r[3].w =1.0f;


	q = quaternionRotationMatrix();
}

NMatrix& NMatrix::setIdentity()
{
	static const NVector VectorIdentity[] = 
	{
		{1.0f,0.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f,0.0f},
		{0.0f,0.0f,1.0f,0.0f},
		{0.0f,0.0f,0.0f,1.0f}
	};
	r[0] = VectorIdentity[0];
	r[1] = VectorIdentity[1];
	r[2] = VectorIdentity[2];
	r[3] = VectorIdentity[3];
	return *this;
}
NMatrix& NMatrix::setScaling(FLOAT x,FLOAT y,FLOAT z)
{
	m[0][0] = x;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = y;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = z;
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	return *this;
}
NMatrix& NMatrix::setRotationRollPitchYaw(FLOAT x,FLOAT y,FLOAT z)
{
	NVector v = {x,y,z,0};
	setRotationQuaternion(v.quotanion());
	return *this;
}
NMatrix& NMatrix::setRotationX(float angle)
{
	float fSinAngle = sinf(angle);
	float fCosAngle = cosf(angle);

	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = fCosAngle;
	m[1][2] = fSinAngle;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = -fSinAngle;
	m[2][2] = fCosAngle;
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	return *this;
}
NMatrix& NMatrix::setRotationY(float angle)
{
	float fSinAngle = sinf(angle);
	float fCosAngle = cosf(angle);

	m[0][0] = fCosAngle;
	m[0][1] = 0.0f;
	m[0][2] = -fSinAngle;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = fSinAngle;
	m[2][1] = 0.0f;
	m[2][2] = fCosAngle;
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	return *this;
}
NMatrix& NMatrix::setRotationZ(float angle)
{
	float fSinAngle = sinf(angle);
	float fCosAngle = cosf(angle);

	m[0][0] = fCosAngle;
	m[0][1] = fSinAngle;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = -fSinAngle;
	m[1][1] = fCosAngle;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	return *this;
}

NMatrix::operator float*()
{
	return f;
}
NMatrix& NMatrix::setRotationQuaternion(const NVector& q)
{
	_11 = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
	_12 = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
	_13 = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
	_14 = 0.0f;

	_21 = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
	_22 = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
	_23 = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
	_24 = 0.0f;

	_31 = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
	_32 = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
	_33 = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
	_34 = 0.0f;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 1.0f;

	return *this;
}
void NMatrix::compMatrix(const NVector& p,const NVector& s,const NVector& q)
{
	NMatrix matrixKey =
	{
		s.x,0.0f,0.0f,0.0f,
		0.0f,s.y,0.0f,0.0f,
		0.0f,0.0f,s.z,0.0f,
		0.0f,0.0f,0.0f,1.0f
	};
	*this = matrixKey * NMatrix().setRotationQuaternion(q);
	r[3] = p;
}
void NMatrix::changeMatrixCoordinate()
{
	NVector p,s;
	NVector q;
	decompMatrix(p,s,q);
	q.z = -q.z;
	p.z = -p.z;
	q.w = -q.w;
	compMatrix(p,s,q);
}
NMatrix& NMatrix::setTranslation(FLOAT x,FLOAT y,FLOAT z)
{
	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;

	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	m[3][3] = 1.0f;
	return *this;
}
NMatrix& NMatrix::setTranspose()
{
	NMatrix temp;
	temp._11 = _11;
	temp._12 = _21;
	temp._13 = _31;
	temp._14 = _41;

	temp._21 = _12;
	temp._22 = _22;
	temp._23 = _32;
	temp._24 = _42;

	temp._31 = _13;
	temp._32 = _23;
	temp._33 = _33;
	temp._34 = _43;

	temp._41 = _14;
	temp._42 = _24;
	temp._43 = _34;
	temp._44 = _44;
	*this = temp;
	return *this;
}
NMatrix& NMatrix::setInverse()
{
	NMatrix result;
	FLOAT buf; //一時的なデータを蓄える
	int i,j,k; //カウンタ
	int n=4;  //配列の次数

	//単位行列を作る
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			result.m[i][j]=(i==j)?1.0f:0.0f;
		}
	}
	//掃き出し法
	for(i=0;i<n;i++)
	{
		buf=1/m[i][i];
		for(j=0;j<n;j++)
		{
			m[i][j] *= buf;
			result.m[i][j] *= buf;
		}
		for(j=0;j<n;j++)
		{
			if(i!=j)
			{
				buf=m[j][i];
				for(k=0;k<n;k++)
				{
					m[j][k]-=m[i][k]*buf;
					result.m[j][k]-=result.m[i][k]*buf;
				}
			}
		}
	}
	*this = result;
	return *this;
}
NMatrix& NMatrix::setLookAtLH(const NVector& EyePosition,const NVector& FocusPosition,const NVector& UpDirection)
{
	NVector EyeDirection = FocusPosition - EyePosition;
	return setLookToLH(EyePosition,EyeDirection,UpDirection);
}
NMatrix& NMatrix::setLookAtRH(const NVector& EyePosition, const NVector& FocusPosition, const NVector& UpDirection)
{
	NVector EyeDirection = EyePosition - FocusPosition;
	return setLookToLH(EyePosition, EyeDirection, UpDirection);
}
NMatrix& NMatrix::setLookToLH(const NVector& EyePosition,const NVector& EyeDirection,const NVector& UpDirection)
{
	NVector R2 = EyeDirection.normal();
	NVector R0 = UpDirection.cross(R2);
	R0 = R0.normal();
	NVector R1 = R2.cross(R0);

	NVector NegEyePosition = EyePosition.negate();

	NVector D0;
	D0 = R0.dot(NegEyePosition);
	NVector D1;
	D1 = R1.dot(NegEyePosition);
	NVector D2;
	D2 = R2.dot(NegEyePosition);

	r[0] = R0;
	r[1] = R1;
	r[2] = R2;

	r[0].w = D0.w;
	r[1].w = D1.w;
	r[2].w = D2.w;

	r[3].x = 0.0f;
	r[3].y = 0.0f;
	r[3].z = 0.0f;
	r[3].w = 1.0f;

	setTranspose();

	return *this;
}
NMatrix& NMatrix::setLookToRH(const NVector& EyePosition, const NVector& EyeDirection, const NVector& UpDirection)
{
	NVector negEyeDirection = EyeDirection.negate();
	setLookToLH(EyePosition, negEyeDirection, UpDirection);
	return *this;
}
NMatrix& NMatrix::setPerspectiveFovLH(FLOAT FovAngleY,FLOAT AspectRatio,FLOAT NearZ,FLOAT FarZ)
{
	FLOAT angle = FovAngleY * 0.5f;
	FLOAT fovSin = sinf(angle);
	FLOAT fovCos = cosf(angle);
	FLOAT height = fovCos / fovSin;
	FLOAT width = height / AspectRatio;
	r[0] = NVector::set(width,0.0f,0.0f,0.0f);
	r[1] = NVector::set(0.0f, height, 0.0f, 0.0f);
	r[2] = NVector::set(0.0f, 0.0f, FarZ / (FarZ - NearZ), 1.0f);
	r[3] = NVector::set(0.0f, 0.0f, -r[2].z * NearZ, 0.0f);

	return *this;
}
NMatrix& NMatrix::setPerspectiveFovRH(FLOAT FovAngleY, FLOAT AspectRatio, FLOAT NearZ, FLOAT FarZ)
{
	FLOAT angle = FovAngleY * 0.5f;
	FLOAT fovSin = sinf(angle);
	FLOAT fovCos = cosf(angle);
	FLOAT height = fovCos / fovSin;
	FLOAT width = height / AspectRatio;
	r[0] = NVector::set(width, 0.0f, 0.0f, 0.0f);
	r[1] = NVector::set(0.0f, height, 0.0f, 0.0f);
	r[2] = NVector::set(0.0f, 0.0f, FarZ / (NearZ - FarZ), -1.0f);
	r[3] = NVector::set(0.0f, 0.0f, r[2].z * NearZ, 0.0f);

	return *this;
}