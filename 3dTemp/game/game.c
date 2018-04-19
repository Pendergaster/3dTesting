#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

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
#define MATH_IMPLEMENTATION
#include <mathutil.h>
#define GAME_SIDE
#include <CommonEngine.h>

CREATEDYNAMICARRAY(renderData, renderArray)

#include "AABBtree.c"

typedef struct
{
	renderArray rend;
	AABBtree	tree;
} Game;

EXPORT void init_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = calloc(1, sizeof(Game));
	init_engine(eng);
	eng->userdata = game;
	printf("game inited!");
	printf("game initedREEEEEEEEEEEEEEE!");
	INITARRAY(game->rend);

	renderData* planet = NULL;
	GET_NEW_OBJ(game->rend, planet);
	*planet = DEFAULT_RENDERDATA;

	planet->material.diffuse = MoonTexture;
	planet->modelId = Planet1;


	GET_NEW_OBJ(game->rend, planet);
	*planet = DEFAULT_RENDERDATA;
	planet->material.diffuse = MoonTexture;
	planet->modelId = Planet1;
	planet->position.x = 5.f;

	for (int i = game->rend.num; i < 50; i++)
	{
		GET_NEW_OBJ(game->rend, planet);
		*planet = DEFAULT_RENDERDATA;
		planet->material.diffuse = MoonTexture;
		planet->modelId = Planet1;
		planet->position.x = 5.f * i;
		// for(int i2 = 0, i2 < game->rend.num;i2++)
	
	}

	eng->renderArray = game->rend.buff;
	eng->sizeOfRenderArray = game->rend.num;
	init_tree(&game->tree);
	Object* objects = calloc(50,sizeof(Object));
	for(int i = 0; i < game->rend.num; i++)
	{
		objects[i].base = &game->rend.buff[i];
		objects[i].dims = eng->model_cache[Planet1].nativeScale;
	}
	for(int i = 0; i < 50; i++)
	{
		insert_to_tree(&game->tree,&objects[i]);
	}
}
EXPORT void update_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = eng->userdata;

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

	draw_box(&eng->drend, game->rend.buff[0].position, eng->model_cache[Planet1].nativeScale);
	eng->renderArray = game->rend.buff;
	eng->sizeOfRenderArray = game->rend.num;
}

EXPORT void dispose_game(void* p)
{
	Engine* eng = p;
	dispose_engine(eng);
	Game* game = eng->userdata;
	DISPOSE_ARRAY(game->rend);
	free(game);
	printf("game disposed!");
	printf("MEMTRACK = %d", MEMTRACK);
	assert(MEMTRACK == 0);
}