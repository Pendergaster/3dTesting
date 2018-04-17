#include "gameHeader\game.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#define MATH_IMPLEMENTATION
#include <mathutil.h>
#include <CommonEngine.h>

static int MEMTRACK = 0;

inline void* DEBUG_MALLOC(int size)
{
	MEMTRACK++;
	return malloc(size);
}

inline void* DEBUG_CALLOC(int COUNT, int SIZE)
{
	MEMTRACK++;
	return calloc(COUNT, SIZE);
}

#define MEM_DEBUG
#ifdef  MEM_DEBUG
#define free(PTR) do{ free(PTR); MEMTRACK--;}while(0)
#define malloc(SIZE) DEBUG_MALLOC(SIZE)
#define calloc(COUNT,SIZE) DEBUG_CALLOC(COUNT,SIZE)
#endif //  MEM_DEBUG

#include <smallGenericDynArray.h>

CREATEDYNAMICARRAY(renderData, renderArray)

typedef struct
{
	renderArray rend;
} Game;

EXPORT void init_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = eng->userdata;
	init_engine(eng);
	printf("game inited!");
	Game* game = p;
	INITARRAY(game->rend);
}

EXPORT void update_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = eng->userdata;
	//printf("game updated!");
	if(is_key_activated(&eng->inputs,KEY_A))
	{
		printf("JELLO!\n");
	}
	if (is_key_activated(&eng->inputs, KEY_Y))
	{
		printf("BELLO!\n");
	}	
	float camSpeed = 0.1f;


	if (is_key_down(KEY_W,&eng->inputs))
	{
		vec3 addvec;
		scale_vec3(&addvec, &eng->camera.cameraDir, camSpeed);
		add_vec3(&eng->camera.cameraPos, &eng->camera.cameraPos, &addvec);
	}
	if (is_key_down(KEY_A, &eng->inputs))
	{
		vec3 addvec;
		cross_product(&addvec, &eng->camera.cameraDir, &eng->camera.camUp);
		normalize_vec3(&addvec);
		scale_vec3(&addvec, &addvec, camSpeed);
		neg_vec3(&eng->camera.cameraPos, &eng->camera.cameraPos, &addvec);
	}
	if (is_key_down(KEY_D, &eng->inputs))
	{
		vec3 addvec;
		cross_product(&addvec, &eng->camera.cameraDir, &eng->camera.camUp);
		normalize_vec3(&addvec);
		scale_vec3(&addvec, &addvec, camSpeed);
		add_vec3(&eng->camera.cameraPos, &eng->camera.cameraPos, &addvec);
	}
	if (is_key_down(KEY_S, &eng->inputs))
	{
		vec3 addvec;
		scale_vec3(&addvec, &eng->camera.cameraDir, -camSpeed);
		add_vec3(&eng->camera.cameraPos, &eng->camera.cameraPos, &addvec);
	}

	if (/*!mouse_init ||*/ eng->inputs.inputsDisabled)
	{

		update_engine_camera(&eng->camera, eng->inputs.mousePos, eng->inputs.mousePos);
	}
	else
	{
		update_engine_camera(&eng->camera, eng->inputs.mousePos, eng->inputs.lastMousepos);
	}


	eng->renderArray = game->rend.buff;
	eng->sizeOfRenderArray = game->rend.num;
}

EXPORT void dispose_game(void* p)
{
	Engine* eng = p;
	Game* game = eng->userdata;
	DISPOSE_ARRAY(game->rend);
	printf("game disposed!");
	printf("MEMTRACK = %d", MEMTRACK);
	assert(MEMTRACK == 0);
}