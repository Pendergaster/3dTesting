#ifndef MATHUTIL_H
#define MATHUTIL_H
#include <stdint.h>
#include<math.h>

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

static void inline orthomat(mat4* ortho,float left,float right,float bottom, float top,float Near,float Far)
{
	// NOTE IT IS [X][Y]
	ortho->mat[0][0] = (2.f / (right - left));
	ortho->mat[0][1] = 0.f;
	ortho->mat[0][2] = 0.f;
	ortho->mat[0][3] = 0.f;

	ortho->mat[1][1] = (2.f / (top - bottom));
	ortho->mat[1][0] = 0.f;
	ortho->mat[1][2] = 0.f;
	ortho->mat[1][3] = 0.f;

	ortho->mat[2][2] = (-2.f / (Far - Near));
	ortho->mat[2][0] = 0.f;
	ortho->mat[2][1] = 0.f;
	ortho->mat[2][3] = 0.f;


	ortho->mat[3][0] = -((right + left) / (right - left));
	ortho->mat[3][1] = -((top + bottom) / (top - bottom));
	ortho->mat[3][2] = -((Far + Near) / (Far - Near));
	ortho->mat[3][3] = 1.f;

}
static void inline perspective(mat4* per,float y_fov, float aspect, float n, float f)// fov widht of frustrum,aspect ratio of scene,near plane, far plane
{
	float const a = (float)(1.f / tanf(y_fov / 2.f));
	per->mat[0][0] = a / aspect;
	per->mat[0][1] = 0.f;
	per->mat[0][2] = 0.f;
	per->mat[0][3] = 0.f;

	per->mat[1][0] = 0.f;
	per->mat[1][1] = a;
	per->mat[1][2] = 0.f;
	per->mat[1][3] = 0.f;

	per->mat[2][0] = 0.f;
	per->mat[2][1] = 0.f;
	per->mat[2][2] = -((f + n) / (f - n));
	per->mat[2][3] = -1.f;

	per->mat[3][0] = 0.f;
	per->mat[3][1] = 0.f;
	per->mat[3][2] = -((2.f * f * n) / (f - n));
	per->mat[3][3] = 0.f;
}
static const float pi = 3.141592653f;

static inline float deg_to_rad(float deg)
{
	return deg * (pi / 180);
}
static inline void mult_mat4(mat4* result,const mat4* lhv, const mat4* rhv)
{
	/*assume that result might be same as lhv*/
	mat4 temp = *lhv;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			result->mat[x][y] = 0;
			for (int n = 0; n < 4; n++)
			{
				result->mat[x][y] += temp.mat[n][y] * rhv->mat[x][n];
			}
		}
	}
}
static inline void identity(mat4* identity)
{
	for(int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			identity->mat[x][y] = x == y ? 1.f : 0.f;
		}
	}

}
static inline float vec3_lenght(const vec3* v) 
{
	return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
static inline void normalize_vec3(vec3* v)
{
	float div = vec3_lenght(v);
	if (div == 0) return;
	v->x /= div;
	v->y /= div;
	v->z /= div;
}
static inline void create_rotate_mat4(mat4* Result,vec3 axis,float angle)
{
	float co = cosf(angle);
	float si = sinf(angle);
	float k = 1 - co;

	normalize_vec3(&axis);


	Result->mat[0][0] = axis.x * axis.x * k + co;		Result->mat[0][1] = axis.x *axis.y * k + axis.z*si	; Result->mat[0][2] = axis.x * axis.z * k - axis.y*si;	Result->mat[0][3] = 0;
	Result->mat[1][0] = axis.x*axis.y*k - axis.z * si;	Result->mat[1][1] = axis.y * axis.y * k + co		; Result->mat[1][2] = axis.y * axis.z * k + axis.x*si;	Result->mat[1][3] = 0;
	Result->mat[2][0] = axis.x *axis.z * k + axis.y*si; Result->mat[2][1] = axis.y * axis.z * k - axis.x*si	; Result->mat[2][2] = axis.z * axis.z * k + co;			Result->mat[2][3] = 0;
	Result->mat[3][0] = 0;								Result->mat[3][1] = 0;								Result->mat[3][2] = 0;									Result->mat[3][3] = 1;

	/*mat4 temp =
	{
		{
			{ axis.x * axis.x * k + co,			axis.x *axis.y * k + axis.z*si,		axis.x * axis.z * k - axis.y*si	,0 },

			{ axis.x*axis.y*k - axis.z * si,	axis.y * axis.y * k + co ,			axis.y * axis.z * k + axis.x*si	,0 },

			{ axis.x *axis.z * k + axis.y*si,	axis.y * axis.z * k - axis.x*si,	axis.z * axis.z * k + co		,0 },

			{ 0									,0								 ,0									,1 }
		}
	};

	*Result = temp;*/
}
static inline void rotate_mat4(mat4* Result,const mat4* lhv, vec3 axis, float angle)
{
	mat4 rotater = { 0 };
	create_rotate_mat4(&rotater, axis, angle);

	mult_mat4(Result, lhv, &rotater);
}

static inline void mat4_mult_vec4(vec4* Result,const mat4* lhv,const vec4* rhv)
{

		Result->x = lhv->mat[0][0] * rhv->x + lhv->mat[1][0] * rhv->y + lhv->mat[2][0] * rhv->z + lhv->mat[3][0] * rhv->w;
		Result->y = lhv->mat[0][1] * rhv->x + lhv->mat[1][1] * rhv->y + lhv->mat[2][1] * rhv->z + lhv->mat[3][1] * rhv->w;
		Result->z = lhv->mat[0][2] * rhv->x + lhv->mat[1][2] * rhv->y + lhv->mat[2][2] * rhv->z + lhv->mat[3][2] * rhv->w;
		Result->w = lhv->mat[0][3] * rhv->x + lhv->mat[1][3] * rhv->y + lhv->mat[2][3] * rhv->z + lhv->mat[3][3] * rhv->w;
}
static inline void create_translation_matrix(mat4* result,const vec3 v)
{
	result->mat[0][0] = 1; result->mat[1][0] = 0; result->mat[2][0] = 0; result->mat[3][0] = v.x;
	result->mat[0][1] = 0; result->mat[1][1] = 1; result->mat[2][1] = 0; result->mat[3][1] = v.y;
	result->mat[0][2] = 0; result->mat[1][2] = 0; result->mat[2][2] = 1; result->mat[3][2] = v.z;
	result->mat[0][3] = 0; result->mat[1][3] = 0; result->mat[2][3] = 0; result->mat[3][3] = 1;
}
static inline void translate_mat4(mat4* Result, const mat4* t,const vec3 v)
{
	mat4 trans = { 0 };
	create_translation_matrix(&trans, v);
	mult_mat4(Result, t, &trans);
}
static inline void sub_vec3(vec3* res,const vec3* lhv,const vec3* rhv)
{
	res->x = lhv->x - rhv->x;
	res->y = lhv->y - rhv->y;
	res->z = lhv->z - rhv->z;
}
static inline void cross_product(vec3* result,const vec3* lhv, const vec3*rhv)
{
	vec3 left = *lhv;
	result->x = left.y * rhv->z - rhv->y * left.z;
	result->y = -1 * (left.x * rhv->z - rhv->x * left.z);
	result->z = left.x * rhv->y - rhv->x * left.y;
}

static inline void scale_vec3(vec3* res,const vec3* v,float scale)
{
	res->x = v->x * scale;
	res->y = v->y * scale;
	res->z = v->z * scale;
}
static inline void neg_vec3(vec3* res,const vec3* lhv,  const vec3* rhv)
{
	res->x = lhv->x - rhv->x;
	res->y = lhv->y - rhv->y;
	res->z = lhv->z - rhv->z;
}
static inline void add_vec3(vec3* res, const vec3* lhv, const vec3* rhv)
{
	res->x = lhv->x + rhv->x;
	res->y = lhv->y + rhv->y;
	res->z = lhv->z + rhv->z;
}
static inline void create_lookat_mat4(mat4* Result,const vec3* eye,const vec3* target,const vec3* up)
{
	//vec3 D = { target->x - eye->x,target->y - eye->y ,target->z - eye->z };

	vec3 D = { eye->x - target->x,eye->y - target->y ,eye->z - target->z };
	normalize_vec3(&D); // needed?

	vec3 R = { 0 };
	cross_product(&R, up, &D);

	/*vec3 camUP = { 0 };
	cross_product(&camUP, &D, &R);*/
 
	mat4 trans = { 0 };
	vec3 camPos = { -eye->x,-eye->y , -eye->z };
	create_translation_matrix(&trans, camPos);


	Result->mat[0][0] = R.x; Result->mat[0][1] = up->x; Result->mat[0][2] = D.x; Result->mat[0][3] = 0.f;
	Result->mat[1][0] = R.y; Result->mat[1][1] = up->y; Result->mat[1][2] = D.y; Result->mat[1][3] = 0.f;
	Result->mat[2][0] = R.z; Result->mat[2][1] = up->z; Result->mat[2][2] = D.z; Result->mat[2][3] = 0.f;
	Result->mat[3][0] = 0.f; Result->mat[3][1] = 0.f; Result->mat[3][2] = 0.f; Result->mat[3][3]  = 1.f;


	mult_mat4(Result, Result, &trans);
	//vec3 p;
	//scale_vec3(&p, pos, -1.f);

	//mat4 trans = { 0 };
	//create_translation_matrix(&trans, p);	
	//mult_mat4(Result, Result, &trans);
}
static inline void rotate_mat4_X(mat4* m,float angle)
{
	if (angle == 0) return;
	float s = sinf(angle);
	float c = cosf(angle);

	mat4 r =
	{
		1.f, 0.f, 0.f, 0.f,
		 0.f,   c,   s, 0.f ,
		0.f,  -s,   c, 0.f ,
		0.f, 0.f, 0.f, 1.f 
	};

	mult_mat4(m, m, &r);
}
static inline void rotate_mat4_Y(mat4* m, float angle)
{
	if (angle == 0) return;
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 r =
	{
		 c, 0.f,   s, 0.f ,
		 0.f, 1.f, 0.f, 0.f ,
		 -s, 0.f,   c, 0.f ,
		 0.f, 0.f, 0.f, 1.f
	};
	mult_mat4(m, m, &r);
}
static inline void rotate_mat4_Z(mat4* m, float angle)
{
	if (angle == 0) return;
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 r =
	{
		 c,   s, 0.f, 0.f ,
		 -s,   c, 0.f, 0.f ,
		 0.f, 0.f, 1.f, 0.f ,
		 0.f, 0.f, 0.f, 1.f 
	};
	mult_mat4(m, m, &r);
}
static inline void scale_mat4(mat4* m,const float s)
{
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++)
			m->mat[x][y] *= s;

}

#undef MATH_IMPLEMENTATION
#endif // MATH_IMPLEMENTATION
