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

typedef struct
{
	renderData*	base;
	vec3		dims;
	uint		treeIndex;
} Object;

static const Object DEFAULT_OBJECT = {0};

CREATEDYNAMICARRAY(Object*,ObjectBuffer);
CREATEDYNAMICARRAY(renderData*,renderDataBuffer);

#include "AABBtree.c"

typedef struct
{
	ObjectBuffer		pools;
	renderDataBuffer	rendPools;
	uint				currentIndex;
	ObjectBuffer		freelist;
	renderDataBuffer	freeRendList;
} ObjectAllocator;

#define POOL_SIZE 1000

static inline void init_objectallocator(ObjectAllocator* al)
{
	INITARRAY(al->pools);
	INITARRAY(al->pools);
	INITARRAY(al->freelist);
	INITARRAY(al->freeRendList);
	al->currentIndex = 0;
	Object** ar = NULL;
	renderData** rd = NULL;
	GET_NEW_OBJ(al->pools,ar);
	(*ar) = malloc(POOL_SIZE * sizeof(Object) + POOL_SIZE * sizeof(renderData));
	GET_NEW_OBJ(al->rendPools,rd);
	(*rd) = (renderData*)((*ar)+POOL_SIZE);
	
}

static inline Object* new_object(ObjectAllocator* al)
{
	Object* ret = NULL;

	if(al->freelist.num == 0)
	{
		if(al->currentIndex < POOL_SIZE)
		{
			ret  = &BACK(al->pools)[al->currentIndex];
			ret->base =  &BACK(al->rendPools)[al->currentIndex++];
		}
		else
		{
			assert(0);
			al->currentIndex = 0;
			Object** ar = NULL;
			renderData** rd = NULL;
			GET_NEW_OBJ(al->pools,ar);
			(*ar) = malloc(POOL_SIZE * sizeof(Object) + POOL_SIZE * sizeof(renderData));
			GET_NEW_OBJ(al->rendPools,rd);
			(*rd) = (renderData*)((*ar)+POOL_SIZE);
			ret  = &BACK(al->pools)[al->currentIndex];
			ret->base =  &BACK(al->rendPools)[al->currentIndex++];
		}

	}
	else
	{
		assert(0);
		POP_ARRAY_COPY(al->freelist, ret);
		POP_ARRAY_COPY(al->freeRendList,ret->base);
	}
	*ret = DEFAULT_OBJECT;
	*(ret->base) = DEFAULT_RENDERDATA;
	return ret;
}

static inline void free_object(ObjectAllocator* al, Object* ob)
{
	PUSH_NEW_OBJ(al->freelist, ob);
	PUSH_NEW_OBJ(al->freeRendList, ob->base);
}

typedef struct
{
	renderDataBuffer 	rend;
	AABBtree			tree;
	ObjectAllocator		obAll;
	//Object*			objects;
	ObjectBuffer		objects;
} Game;

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}


#define NUM_OBJS 5
EXPORT void init_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = calloc(1, sizeof(Game));
	init_engine(eng);
	eng->userdata = game;
	printf("game inited!");
	printf("game initedREEEEEEE!");
	INITARRAY(game->rend);
	INITARRAY(game->objects);
	init_objectallocator(&game->obAll);








	renderData* planet = NULL;


	// for (int i = game->rend.num; i < NUM_OBJS; i++)
	// {
	// 	GET_NEW_OBJ(game->rend, planet);
	// 	*planet = DEFAULT_RENDERDATA;
	// 	planet->material.diffuse = MoonTexture;
	// 	planet->modelId = Planet1;
	// 	planet->position.x = i * 5.f;//rand_interval(0,15);
	// 	planet->position.y = 0;//rand_interval(0,15);
	// 	planet->position.z = 0;//rand_interval(0,15);
	// 	// for(int i2 = 0, i2 < game->rend.num;i2++)
	
	// }

	eng->renderArray = game->rend.buff;
	eng->sizeOfRenderArray = game->rend.num;
	init_tree(&game->tree);
	//Object* objects = calloc(NUM_OBJS,sizeof(Object));


	// for(int i = 0; i < game->rend.num; i++)
	// {
	// 	objects[i].base = &game->rend.buff[i];
	// 	objects[i].dims = eng->model_cache[Planet1].nativeScale;
	// }
	// uint inds[NUM_OBJS];
	// for(int i = 0; i < NUM_OBJS; i++)
	// {
	// 	inds[i] = insert_to_tree(&game->tree,&objects[i]);
	// 	objects[i].treeIndex = inds[i];
	// }
	// printf("INDEXES\n");
	// for(int i = 0; i < NUM_OBJS; i++)
	// {
	// 	printf("%d\n",inds[i]);
	// 	update_object_in_tree(&game->tree,inds[i]);
	// }
	// game->objects = objects;
	//force_fit_parent(&game->tree, &(game->tree.allocator[game->tree.rootIndex]));
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
	// struct Node* current = &game->tree.allocator[game->tree.allocator[game->tree.NodeArrayallocator.num].childIndexes[0]];
	
	// for(int i = 0; i < game->tree.NodeArrayallocator.num - game->tree.freelist.num; i++)
	// {	
	// 	//if(/*i == 4 || i == 3 || i == 6 || i == 1 || i == 5*/ game->tree.allocator[i].type == Branch)
	// 	draw_abbREEE(&game->tree.allocator[i], &eng->drend);
	// }
	// if(is_key_activated(&eng->inputs,KEY_P))
	// {
	// 	remove_node(&game->tree,7);
	//  	game->rend.num--;
	// }
	// game->objects[NUM_OBJS - 1].base->position.x += 0.05f;
	// for(int i = 0; i < NUM_OBJS; i++)
	// {
	// 	//printf()
	// 	game->objects[i].treeIndex = update_object_in_tree(&game->tree,game->objects[i].treeIndex);
	// }
	//BACK(game->rend).position.x += 0.1f;

	if(is_key_activated(&eng->inputs,KEY_P))
	{
		Object* nob = new_object(&game->obAll);
		renderData* planet = nob->base;
		//GET_NEW_OBJ(game->rend, planet);
		// printf("ALLOCATOR POOLS %d INDEX %d \n,",game->obAll.pools.num,game->obAll.currentIndex);
		planet->material.diffuse = MoonTexture;
		planet->modelId = Planet1;
		planet->position.x = rand_interval(0,60.f);
		planet->position.y = rand_interval(0,60.f);
		planet->position.z = rand_interval(0,60.f);	

		PUSH_NEW_OBJ(game->objects,nob);
		PUSH_NEW_OBJ(game->rend,planet);
		nob->dims = eng->model_cache[Planet1].nativeScale;
		nob->treeIndex = insert_to_tree(&game->tree,nob);
	}
	if(is_key_activated(&eng->inputs,KEY_O))
	{
		remove_node(&game->tree,BACK(game->objects)->treeIndex);
		game->objects.num--;
		game->rend.num--;
	}

	ObjectBuffer temp;
	INITARRAY(temp);

	vec3 potemp = {0};
	if(game->objects.num > 2)
	query_area(&game->tree,potemp,5,&temp);
	for(int i = 0; i < temp.num; i++)
	{
		draw_box(&eng->drend,temp.buff[i]->base->position,temp.buff[i]->dims);
	}
	vec3 di = {10,10,10};
	draw_box(&eng->drend,potemp,di);
	DISPOSE_ARRAY(temp);
	if(game->objects.num)
	draw_tree(&game->tree,&eng->drend);

	eng->renderArray = game->rend.buff;
	eng->sizeOfRenderArray = game->rend.num;
}

EXPORT void dispose_game(void* p)
{
	Engine* eng = p;
	dispose_engine(eng);
	Game* game = eng->userdata;
	dispose_tree(&game->tree);
	DISPOSE_ARRAY(game->rend);
	//DISPOSE_ARRAY(&game->obAll.pools);
	//DISPOSE_ARRAY(&game->obAll.freelist);
	//DISPOSE_ARRAY(&game->objects);
	free(game);
	printf("game disposed!");
	printf("MEMTRACK = %d", MEMTRACK);
	//assert(MEMTRACK == 0);
}