#if defined(_WIN32) | defined(_WIN32_WCE)
	#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "aflMd5.h"

namespace AFL
{

struct SHA1_Context {

	UINT8  Block[0x40];
	UINT32 State[5 + 1];
	UINT64 Count;

}; /* SHA1_Context */

typedef struct SHA1_Context SHA1_Context_t;

void SHA1cc_Init(SHA1_Context_t* t);
void SHA1cc_Update(SHA1_Context_t* t, const VOID* pv, SIZE_T cb);
void SHA1cc_Finalize(SHA1_Context_t* t, UINT8 digest[20]);


#define ROL(v, b) _rotl((v), (b))
#define BSW(v)    _byteswap_ulong((v))

	/* */

#define B_(i) (b[i] = BSW(b[i])) /* LE */
#define BL(i) (b[i&0xf] = ROL(b[(i+0xd)&0xf] ^ b[(i+0x8)&0xf] ^ b[(i+0x2)&0xf] ^ b[i&0xf], 1))

#define R_(v,w,x,y,z,i) r##z += ((r##w & (r##x ^ r##y)) ^ r##y)          + B_(i) + 0x5a827999 + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R0(v,w,x,y,z,i) r##z += ((r##w & (r##x ^ r##y)) ^ r##y)          + BL(i) + 0x5a827999 + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R1(v,w,x,y,z,i) r##z += (r##w ^ r##x ^ r##y)                     + BL(i) + 0x6ed9eba1 + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R2(v,w,x,y,z,i) r##z += (((r##w | r##x) & r##y) | (r##w & r##x)) + BL(i) + 0x8f1bbcdc + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R3(v,w,x,y,z,i) r##z += (r##w ^ r##x ^ r##y)                     + BL(i) + 0xca62c1d6 + ROL(r##v, 5); r##w = ROL(r##w, 30);

	/* */

void SHA1cc_Transform(SHA1_Context_t* t)
{
	UINT32 r0 = t->State[0];
	UINT32 r1 = t->State[1];
	UINT32 r2 = t->State[2];
	UINT32 r3 = t->State[3];
	UINT32 r4 = t->State[4];

	UINT32* b = (UINT32*)(t->Block);

	R_(0, 1, 2, 3, 4, 0)
		R_(4, 0, 1, 2, 3, 1)
		R_(3, 4, 0, 1, 2, 2)
		R_(2, 3, 4, 0, 1, 3)
		R_(1, 2, 3, 4, 0, 4)
		R_(0, 1, 2, 3, 4, 5)
		R_(4, 0, 1, 2, 3, 6)
		R_(3, 4, 0, 1, 2, 7)
		R_(2, 3, 4, 0, 1, 8)
		R_(1, 2, 3, 4, 0, 9)
		R_(0, 1, 2, 3, 4, 10)
		R_(4, 0, 1, 2, 3, 11)
		R_(3, 4, 0, 1, 2, 12)
		R_(2, 3, 4, 0, 1, 13)
		R_(1, 2, 3, 4, 0, 14)
		R_(0, 1, 2, 3, 4, 15)
		R0(4, 0, 1, 2, 3, 16)
		R0(3, 4, 0, 1, 2, 17)
		R0(2, 3, 4, 0, 1, 18)
		R0(1, 2, 3, 4, 0, 19)

		R1(0, 1, 2, 3, 4, 20)
		R1(4, 0, 1, 2, 3, 21)
		R1(3, 4, 0, 1, 2, 22)
		R1(2, 3, 4, 0, 1, 23)
		R1(1, 2, 3, 4, 0, 24)
		R1(0, 1, 2, 3, 4, 25)
		R1(4, 0, 1, 2, 3, 26)
		R1(3, 4, 0, 1, 2, 27)
		R1(2, 3, 4, 0, 1, 28)
		R1(1, 2, 3, 4, 0, 29)
		R1(0, 1, 2, 3, 4, 30)
		R1(4, 0, 1, 2, 3, 31)
		R1(3, 4, 0, 1, 2, 32)
		R1(2, 3, 4, 0, 1, 33)
		R1(1, 2, 3, 4, 0, 34)
		R1(0, 1, 2, 3, 4, 35)
		R1(4, 0, 1, 2, 3, 36)
		R1(3, 4, 0, 1, 2, 37)
		R1(2, 3, 4, 0, 1, 38)
		R1(1, 2, 3, 4, 0, 39)

		R2(0, 1, 2, 3, 4, 40)
		R2(4, 0, 1, 2, 3, 41)
		R2(3, 4, 0, 1, 2, 42)
		R2(2, 3, 4, 0, 1, 43)
		R2(1, 2, 3, 4, 0, 44)
		R2(0, 1, 2, 3, 4, 45)
		R2(4, 0, 1, 2, 3, 46)
		R2(3, 4, 0, 1, 2, 47)
		R2(2, 3, 4, 0, 1, 48)
		R2(1, 2, 3, 4, 0, 49)
		R2(0, 1, 2, 3, 4, 50)
		R2(4, 0, 1, 2, 3, 51)
		R2(3, 4, 0, 1, 2, 52)
		R2(2, 3, 4, 0, 1, 53)
		R2(1, 2, 3, 4, 0, 54)
		R2(0, 1, 2, 3, 4, 55)
		R2(4, 0, 1, 2, 3, 56)
		R2(3, 4, 0, 1, 2, 57)
		R2(2, 3, 4, 0, 1, 58)
		R2(1, 2, 3, 4, 0, 59)

		R3(0, 1, 2, 3, 4, 60)
		R3(4, 0, 1, 2, 3, 61)
		R3(3, 4, 0, 1, 2, 62)
		R3(2, 3, 4, 0, 1, 63)
		R3(1, 2, 3, 4, 0, 64)
		R3(0, 1, 2, 3, 4, 65)
		R3(4, 0, 1, 2, 3, 66)
		R3(3, 4, 0, 1, 2, 67)
		R3(2, 3, 4, 0, 1, 68)
		R3(1, 2, 3, 4, 0, 69)
		R3(0, 1, 2, 3, 4, 70)
		R3(4, 0, 1, 2, 3, 71)
		R3(3, 4, 0, 1, 2, 72)
		R3(2, 3, 4, 0, 1, 73)
		R3(1, 2, 3, 4, 0, 74)
		R3(0, 1, 2, 3, 4, 75)
		R3(4, 0, 1, 2, 3, 76)
		R3(3, 4, 0, 1, 2, 77)
		R3(2, 3, 4, 0, 1, 78)
		R3(1, 2, 3, 4, 0, 79)

		t->State[0] += r0;
	t->State[1] += r1;
	t->State[2] += r2;
	t->State[3] += r3;
	t->State[4] += r4;
}

/* */

void SHA1cc_Init(SHA1_Context_t* t)
{
	memset(t, 0, sizeof(SHA1_Context_t));

	t->State[0] = 0x67452301;
	t->State[1] = 0xefcdab89;
	t->State[2] = 0x98badcfe;
	t->State[3] = 0x10325476;
	t->State[4] = 0xc3d2e1f0;
}

void SHA1cc_Update(
	SHA1_Context_t* t,
	const VOID*     pv,
	SIZE_T          cb)
{
	const UINT8* ss = (const UINT8*)pv;
	const UINT8* se = ss + cb;

	UINT32 i = (UINT32)(t->Count) & 0x3f;
	if (i > 0) {
		SIZE_T s_sz = se - ss;
		SIZE_T d_sz = 0x40 - i;

		if (s_sz < d_sz) {
			memcpy(t->Block + i, ss, s_sz);
			t->Count += s_sz;
			return;
		}

		memcpy(t->Block + i, ss, d_sz);
		t->Count += d_sz;
		ss += d_sz;

		SHA1cc_Transform(t);
	}

	for (;;) {
		SIZE_T s_sz = se - ss;
		if (s_sz < 0x40) {
			memcpy(t->Block, ss, s_sz);
			t->Count += s_sz;
			return;
		}

		memcpy(t->Block, ss, 0x40);
		t->Count += 0x40;
		ss += 0x40;

		SHA1cc_Transform(t);
	}
}

#define UI64_0(v) BSW((UINT32)((v) >> 32)) /* LE */
#define UI64_1(v) BSW((UINT32) (v)       ) /* LE */
#define UI32(v)   BSW((v))                 /* LE */

void SHA1cc_Finalize(
	SHA1_Context_t* t,
	UINT8           digest[20])
{
	UINT64 bit_count = t->Count << 3;

	UINT32 i = (UINT32)(t->Count) & 0x3f;

	t->Block[i++] = 0x80;
	memset(t->Block + i, 0, 0x40 - i);

	if (i > 0x38) {
		SHA1cc_Transform(t);

		memset(t->Block, 0, 0x40);
	}

	*((UINT32*)(t->Block + 0x38)) = UI64_0(bit_count);
	*((UINT32*)(t->Block + 0x3c)) = UI64_1(bit_count);

	SHA1cc_Transform(t);

	*((UINT32*)(digest + 0x00)) = UI32(t->State[0]);
	*((UINT32*)(digest + 0x04)) = UI32(t->State[1]);
	*((UINT32*)(digest + 0x08)) = UI32(t->State[2]);
	*((UINT32*)(digest + 0x0c)) = UI32(t->State[3]);
	*((UINT32*)(digest + 0x10)) = UI32(t->State[4]);
}


typedef struct {
  UINT i[2];                   /* number of _bits_ handled mod 2^64 */
  UINT buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;



static void Transform (UINT* buf, UINT* in);

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z))) 

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

void MD5Init (MD5_CTX* mdContext)
{
  mdContext->i[0] = mdContext->i[1] = (UINT)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT)0x67452301;
  mdContext->buf[1] = (UINT)0xefcdab89;
  mdContext->buf[2] = (UINT)0x98badcfe;
  mdContext->buf[3] = (UINT)0x10325476;
}

void MD5Update (MD5_CTX* mdContext,const unsigned char* inBuf,unsigned int inLen)
{
  UINT in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT)inLen << 3);
  mdContext->i[1] += ((UINT)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT)mdContext->in[ii+3]) << 24) |
                (((UINT)mdContext->in[ii+2]) << 16) |
                (((UINT)mdContext->in[ii+1]) << 8) |
                ((UINT)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

void MD5Final (MD5_CTX* mdContext)
{
  UINT in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT)mdContext->in[ii+3]) << 24) |
            (((UINT)mdContext->in[ii+2]) << 16) |
            (((UINT)mdContext->in[ii+1]) << 8) |
            ((UINT)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}

/* Basic MD5 step. Transform buf based on in.
 */
static void Transform (UINT* buf, UINT* in)
{
  UINT a = buf[0], b = buf[1], c = buf[2], d = buf[3];
  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360U); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710U); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819U); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966U); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399U); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426U); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955U); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313U); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416U); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879U); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233U); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134U); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682U); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195U); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006U); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329U); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786U); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664U); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713U); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994U); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605U); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083U); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961U); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448U); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438U); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606U); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335U); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501U); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829U); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512U); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473U); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562U); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738U); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833U); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562U); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740U); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236U); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353U); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664U); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656U); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174U); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074U); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317U); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189U); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809U); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461U); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520U); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645U); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452U); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415U); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391U); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241U); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571U); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690U); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773U); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497U); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359U); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552U); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916U); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649U); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226U); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917U); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259U); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745U); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
//--------------------------------------------------------------

void SHA1String(std::string& dest, MD5_CTX* mdContext)
{
	CHAR buff[33];
	int i;
	for (i = 0; i < 16; i++)
		sprintf(&buff[i * 2], "%02x", mdContext->digest[i]);
	dest = buff;
}

bool SHA1::parse(UINT8* digest, LPCSTR value)
{
	SHA1_Context_t ctx;
	INT length = (INT)strlen(value);

	SHA1cc_Init(&ctx);
	SHA1cc_Update(&ctx, (const unsigned char*)value, length);
	SHA1cc_Finalize(&ctx, digest);
	return true;
}
bool SHA1::String(std::string& dest, LPCSTR value)
{
	SHA1_Context_t ctx;
	INT length = (INT)strlen(value);

	SHA1cc_Init(&ctx);
	SHA1cc_Update(&ctx, (const unsigned char*)value, length);
	UINT8 digest[20];
	SHA1cc_Finalize(&ctx, digest);

	static const char* HEX = "0123456789abcdef";

	int i;
	for (i = 0; i < 20; i++)
	{
		dest += HEX[digest[i] >> 4];
		dest += HEX[digest[i] & 0xf];
	}
	return true;
}
bool SHA1::File(std::string& dest, FILE* file)
{
	if (!file)
		return false;

	SHA1_Context_t ctx;
	SHA1cc_Init(&ctx);

	CHAR buff[4096];
	INT size;
	fseek(file, 0, SEEK_SET);
	while (size = fread(buff, 1, sizeof(buff), file))
	{
		SHA1cc_Update(&ctx, (const unsigned char*)buff, size);
	}
	static const char* HEX = "0123456789abcdef";
	UINT8 digest[20];
	SHA1cc_Finalize(&ctx, digest);
	int i;
	for (i = 0; i < 20; i++)
	{
		dest += HEX[digest[i] >> 4];
		dest += HEX[digest[i] & 0xf];
	}
	return true;
}

bool SHA1::File(std::string& dest, LPCSTR fileName)
{
	FILE* file = fopen(fileName, "rb");
	if (!file)
		return false;
	bool flag = File(dest, file);
	fclose(file);
	return flag;
}
//--------------------------------------------------------------
void MDString(std::string& dest,MD5_CTX* mdContext)
{
	CHAR buff[33];
	int i;
	for (i = 0; i < 16; i++)
		sprintf (&buff[i*2],"%02x", mdContext->digest[i]);
	dest = buff;
}

bool MD5::Data(std::string& dest, LPCVOID data, INT length)
{
	MD5_CTX mdContext;
	MD5Init(&mdContext);
	MD5Update(&mdContext, (const unsigned char*)data, length);
	MD5Final(&mdContext);
	MDString(dest, &mdContext);
	return true;
}

bool MD5::String(std::string& dest,LPCSTR value)
{
	MD5_CTX mdContext;
	INT length = (INT)strlen(value);

	MD5Init (&mdContext);
	MD5Update (&mdContext, (const unsigned char*)value, length);
	MD5Final (&mdContext);
	MDString(dest,&mdContext);
	return true;
}
bool MD5::File(std::string& dest,FILE* file)
{
	if(!file)
		return false;

	MD5_CTX mdContext;
	MD5Init (&mdContext);

	CHAR buff[4096];
	INT size;
	fseek(file,0,SEEK_SET);
	while(size = fread (buff, 1, sizeof(buff), file))
	{
		MD5Update(&mdContext, (UCHAR*)buff, (int)size);
	}
	MD5Final (&mdContext);
	MDString(dest,&mdContext);
	return true;
}

bool MD5::File(std::string& dest,LPCSTR fileName)
{
	FILE* file = fopen(fileName,"rb");
	if(!file)
		return false;

	MD5_CTX mdContext;
	MD5Init (&mdContext);

	CHAR buff[4096];
	size_t size;
	while(size = fread (buff, 1, sizeof(buff), file))
	{
		MD5Update(&mdContext, (UCHAR*)buff, (int)size);
	}
	MD5Final (&mdContext);

	MDString(dest,&mdContext);
	fclose(file);
	return true;
}

}
