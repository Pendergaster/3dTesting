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

static mat4 inline orthomat(float left,float right,float bottom, float top,float Near,float Far)
{
	// NOTE IT IS [X][Y]
	mat4 ortho = {0};

	ortho.mat[0][0] = (2.f / (right - left));
	ortho.mat[1][1] = (2.f / (top - bottom));
	ortho.mat[2][2] = (-2.f / (Far - Near));
	ortho.mat[3][3] = 1.f;


	ortho.mat[3][0] = -((right + left) / (right - left));
	ortho.mat[3][1] = -((top + bottom) / (top - bottom));
	ortho.mat[3][2] = -((Far + Near) / (Far - Near));
}
static mat4 inline perspective(float y_fov, float aspect, float n, float f)// fov widht of frustrum,aspect ratio of scene,near plane, far plane
{
	float const a = 1.f / tan(y_fov / 2.f);
	mat4 per = { 0 };
	per.mat[0][0] = a / aspect;
	per.mat[0][1] = 0.f;
	per.mat[0][2] = 0.f;
	per.mat[0][3] = 0.f;

	per.mat[1][0] = 0.f;
	per.mat[1][1] = a;
	per.mat[1][2] = 0.f;
	per.mat[1][3] = 0.f;

	per.mat[2][0] = 0.f;
	per.mat[2][1] = 0.f;
	per.mat[2][2] = -((f + n) / (f - n));
	per.mat[2][3] = -1.f;

	per.mat[3][0] = 0.f;
	per.mat[3][1] = 0.f;
	per.mat[3][2] = -((2.f * f * n) / (f - n));
	per.mat[3][3] = 0.f;

}
const float pi = 3.14159265359;

static inline float deg_to_rad(float deg)
{
	return deg * (pi / 180);
}
static inline mat4 mult_mat4(const mat4* lhv, const mat4* rhv)
{
	mat4 temp;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			temp.mat[x][y] = 0;
			for (int n = 0; n < 4; n++)
			{
				temp.mat[x][y] += lhv->mat[n][y] * rhv->mat[x][n];
			}
		}
	}
	return temp;
}
static inline mat4 identity()
{
	mat4 temp = {0};
	temp.mat[0][0] = 1;
	temp.mat[1][1] = 1;
	temp.mat[2][2] = 1;
	temp.mat[3][3] = 1;
	return temp;
}


#undef MATH_IMPLEMENTATION
#endif // MATH_IMPLEMENTATION
