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
	float w;
} vec3;

typedef struct
{
	float x;
	float y;
	float w;
	float z;
} vec4;

typedef struct
{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte a;
} Color;

#endif //MATHUTIL_H 

#ifdef MATH_IMPLEMENTATION

#undef MATH_IMPLEMENTATION
#endif // MATH_IMPLEMENTATION
