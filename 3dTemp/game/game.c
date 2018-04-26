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

enum
{
	Flocker,
	Player
};

typedef struct
{
	uint		type;
	renderData*	base;
	vec3		dims;
	uint		treeIndex;
	vec3		velocity;
	vec3		acceleration;
} Object;
static const Object DEFAULT_OBJECT = {0};

CREATEDYNAMICARRAY(Object*,ObjectBuffer);
CREATEDYNAMICARRAY(renderData*,renderDataBuffer);

#define NUM_OBJS 2000
#include "AABBtree.c"

typedef struct
{
	ObjectBuffer		pools;
	renderDataBuffer	rendPools;
	uint				currentIndex;
	ObjectBuffer		freelist;
	renderDataBuffer	freeRendList;
} ObjectAllocator;

#define POOL_SIZE 5000

static inline void init_objectallocator(ObjectAllocator* al)
{
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
static void inline dispose_object_allocator(ObjectAllocator* al)
{
	while(al->pools.num)
	{
		Object* del = NULL;
		POP_ARRAY_COPY(al->pools,del); 
		free(del);
	}
	DISPOSE_ARRAY(al->pools);
	DISPOSE_ARRAY(al->freelist);
	DISPOSE_ARRAY(al->freeRendList);
}


// asdfasdf 
// g; g,
// ctrl-o ctrl-i
static inline Object* new_object(ObjectAllocator* al)
{
	Object* ret = NULL;

	if(al->freelist.num == 0)
	{
		if(al->currentIndex < POOL_SIZE)
		{
			
			ret  = &BACK(al->pools)[al->currentIndex];
			*ret = DEFAULT_OBJECT;
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
			*ret = DEFAULT_OBJECT;
			ret->base =  &BACK(al->rendPools)[al->currentIndex++];
		}

	}
	else
	{
		assert(0);
		POP_ARRAY_COPY(al->freelist, ret);
		POP_ARRAY_COPY(al->freeRendList,ret->base);
	}
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
	ObjectBuffer		raybuffer;
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
    do{
    
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}


#define MAX_FLOCK_VEL 0.3f

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
	INITARRAY(game->raybuffer);
	init_objectallocator(&game->obAll);


	renderData* planet = NULL;


	eng->renderArray = game->rend.buff;
	eng->sizeOfRenderArray = game->rend.num;
	init_tree(&game->tree);

	for(int i = 0; i < NUM_OBJS; i++)
	{

		Object* nob = new_object(&game->obAll);
		renderData* planet = nob->base;
		planet->material.diffuse = MoonTexture;
		planet->modelId = Planet1;
		planet->position.x = (float)rand_interval(0,400) - 200.f;
		planet->position.y = (float)rand_interval(0,400) - 200.f;
		planet->position.z = (float)rand_interval(0,400) - 200.f;	

		nob->velocity.x = ((float)rand_interval(0,30) - 15.f) * 0.1f;
		nob->velocity.y = ((float)rand_interval(0,30) - 15.f) * 0.1f;
		nob->velocity.z = ((float)rand_interval(0,30) - 15.f) * 0.1f;

		reduce_vec3_inplace(&nob->velocity,MAX_FLOCK_VEL);

		PUSH_NEW_OBJ(game->objects,nob);
		PUSH_NEW_OBJ(game->rend,planet);
		nob->dims = eng->model_cache[Planet1].nativeScale;
		nob->treeIndex = insert_to_tree(&game->tree,nob);
	}

	// if(is_key_activated(&eng->inputs,KEY_O))
	// {
	// 	remove_node(&game->tree,BACK(game->objects)->treeIndex);
	// 	game->objects.num--;
	// 	game->rend.num--;
	// }

	
}
void update_objects(ObjectBuffer* objs,float dt,ObjectBuffer* buffer,AABBtree* tree);
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
	update_objects(&game->objects,eng->DT,&game->raybuffer,&game->tree);
	

	// ObjectBuffer temp;
	// INITARRAY(temp);

	static float pp = 0.f;
	pp += 0.02f;
	vec3 potemp = {pp,pp,pp};
	//if(game->objects.num > 2)
	// query_area(&game->tree,potemp,10,&temp);
	// for(int i = 0; i < temp.num; i++)
	// {
	// 	draw_box(&eng->drend,temp.buff[i]->base->position,temp.buff[i]->dims);
	// }
//	vec3 di = {5,5,5};
	//draw_box(&eng->drend,potemp,di);
	//DISPOSE_ARRAY(temp);
//	if(game->objects.num)
	//draw_tree(&game->tree,&eng->drend);

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
	DISPOSE_ARRAY(game->objects);
	DISPOSE_ARRAY(game->raybuffer)
	dispose_object_allocator(&game->obAll);
	//DISPOSE_ARRAY(&game->obAll.pools);
	//DISPOSE_ARRAY(&game->obAll.freelist);
	//DISPOSE_ARRAY(&game->objects);
	free(game);
	printf("game disposed!\n");
	printf("MEMTRACK = %d \n", MEMTRACK);
	assert(MEMTRACK == 0);
}

#define SEPARATION_MULTPLIER 1.5f
#define ALINGMENT_MULTPLIER 1.f
#define COHESION_MULTPLIER 1.f
#define RAY_AREA 5.f
#define MAX_FORCE 0.1f
#define MAX_SPEED 4.f


uint update = 1;
void update_objects(ObjectBuffer* objs,float dt,ObjectBuffer* buffer,AABBtree* tree)
{
	if (update)
	{

	for(int i = 0;i < objs->num; i++)
	{
		query_area(tree,objs->buff[i]->base->position,RAY_AREA,buffer);
		//SEPARATION
		
		//ALINGMENT
		vec3 alingvel = {0};
		if(buffer->num > 1) // not myself
		{
			for(int i = 0; i < buffer->num; i++)
			{
				if(objs->buff[i] != buffer->buff[i])
				add_vec3(&alingvel,&alingvel,&buffer->buff[i]->velocity);
			}
			alingvel.x /= buffer->num;
			alingvel.y /= buffer->num;
			alingvel.z /= buffer->num;
			normalize_vec3(&alingvel);
			scale_vec3(&alingvel,&alingvel,MAX_SPEED);
			sub_vec3(&alingvel,&alingvel,&objs->buff[i]->velocity);
			reduce_vec3_inplace(&alingvel,MAX_FORCE);
		}
		//COHEESIO
		// vec3 cohesionvel = {0};
		// if(buffer->num > 1)
		// {
		// 	for(int i = 0; i < buffer->num; i++)
		// 	{
		// 		if(objs->buff[i] != buffer->buff[i])
		// 		add_vec3(&cohesionvel,&cohesionvel,&buffer->buff[i]->base->position);
		// 	}
		// 	cohesionvel.x /= buffer->num;
		// 	cohesionvel.y /= buffer->num;
		// 	cohesionvel.z /= buffer->num;
		// 	normalize_vec3(&alingvel);
		// 	scale_vec3(&alingvel,&alingvel,MAX_SPEED);
		// 	sub_vec3(&alingvel,&alingvel,&objs->buff[i]->velocity);
		// 	reduce_vec3_inplace(&alingvel,MAX_FORCE);
		//}





		add_vec3(&objs->buff[i]->acceleration,&objs->buff[i]->acceleration,&alingvel);
		objs->buff[i]->velocity.x += objs->buff[i]->acceleration.x * dt;
		objs->buff[i]->velocity.y += objs->buff[i]->acceleration.y * dt;
		objs->buff[i]->velocity.z += objs->buff[i]->acceleration.z * dt;

		objs->buff[i]->base->position.x += objs->buff[i]->velocity.x * dt;
		objs->buff[i]->base->position.y += objs->buff[i]->velocity.y * dt;
		objs->buff[i]->base->position.z += objs->buff[i]->velocity.z * dt;


		objs->buff[i]->treeIndex = update_object_in_tree(tree,objs->buff[i]->treeIndex);

		objs->buff[i]->acceleration.x = 0;
		objs->buff[i]->acceleration.y = 0;
		objs->buff[i]->acceleration.z = 0;
		buffer->num = 0;
		update = 0;
	}	
	}
	else
	{
		update = 1;
	}
	
}