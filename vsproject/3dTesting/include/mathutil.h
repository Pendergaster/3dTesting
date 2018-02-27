#ifndef MATHUTIL_H
#define MATHUTIL_H
#include <stdint.h>

typedef uint8_t		ubyte;
typedef int8_t		byte;
typedef uint32_t	uint;

typedef struct
{
	float x;
	float y;
} vec2;

typedef struct
{
	float x;
	float y;
	float z;
} vec3;

typedef struct
{
	float x;
	float y;
	float z;
	float w;
} vec4;

typedef struct
{
	float mat[4][4];
} mat4;

typedef struct
{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte a;
} Color;

#endif //MATHUTIL_H 

#ifdef MATH_IMPLEMENTATION

mat4 inline orthomat(float left,float right,float bottom, float top,float Near,float Far)
{
	mat4 ortho = {0};

	ortho.mat[0][0] = (2.f / (right - left));
	ortho.mat[0][3] = -((right + left) / (right - left));
	ortho.mat[1][1] = (2.f / (top - bottom));
	ortho.mat[1][3] = -((top + bottom) / (top - bottom));
	ortho.mat[2][2] = (-2.f / (Far - Near));
	ortho.mat[2][3] = -((Far + Near) / (Far - Near));
	ortho.mat[3][3] = 1.f;
}

#undef MATH_IMPLEMENTATION
#endif // MATH_IMPLEMENTATION
