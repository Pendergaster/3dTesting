#ifndef YCM_HELPER
#define YCM_HELPER
#ifdef YCM
#include <stdio.h>
#include <stdint.h>
#include<math.h>
#include <glad/glad.h>
#include <ycm_helper.h>
#include <stdlib.h>
#include<assert.h>
typedef uint8_t		ubyte;
typedef int8_t		byte;
typedef uint32_t	uint;
#define FATALERROR	assert(0)


typedef struct
{
	union {
		struct { float x, y; };
		struct { float i, j; };
	};

} vec2;

typedef struct
{
	union {
		struct { float x, y, z; };
		struct { float i, j, k; };
	};


} vec3;

typedef struct
{
	union {
		struct { float x, y, z, w; };
		struct { float scalar, i, j, k; };
		struct { float s, t, p, q; }; // texture coordinates

	};
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


#endif // YCM
#endif // YCM_HELPER
