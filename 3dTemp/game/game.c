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
    InvalidType,
	Flocker,
	Player,
	Bullet
};

#define MAX_NEIGHS 8
typedef struct
{
	renderData*	base;
	vec3		dims;
	uint		treeIndex;
	vec3		velocity;
	vec3		acceleration;
	uint		type;
	union
	{
		struct
		{
			float	curQueryTime;
			void*   neighbours[MAX_NEIGHS];
			uint 	numNeigh;
		};
		struct
		{
			float 	lastShot;
		};

	};
} Object;
static const Object DEFAULT_OBJECT = {0};

CREATEDYNAMICARRAY(Object*,ObjectBuffer);
CREATEDYNAMICARRAY(renderData*,renderDataBuffer);

#define NUM_OBJS 100
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

float map(float min,float max,float norm,float dmin,float dmax);
static inline void init_objectallocator(ObjectAllocator* al)
{
	float test = map(0,3,3.f/2.f,0,10);
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
		//assert(0);
		POP_ARRAY_COPY(al->freelist, ret);
		POP_ARRAY_COPY(al->freeRendList,ret->base);
		assert(ret->type == InvalidType);
	}
	*(ret->base) = DEFAULT_RENDERDATA;
	return ret;
}

static inline void free_object(ObjectAllocator* al, Object* ob)
{	
    ob->type = InvalidType;
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


#define MAX_FLOCK_VEL 2.f
#define NUM_QUERIES_PER_SEC 10
#define SPAWN_RANGE 150
EXPORT void init_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = calloc(1, sizeof(Game));
	init_engine(eng);
	eng->userdata = game;
	printf("game inited!");
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
		planet->material.diffuse = SpaceshipTexture;
		planet->modelId = SpaceShip;
		planet->position.x = (float)rand_interval(0,SPAWN_RANGE) - SPAWN_RANGE/2.f;
		planet->position.y = (float)rand_interval(0,SPAWN_RANGE) - SPAWN_RANGE/2.f;
		planet->position.z = (float)rand_interval(0,SPAWN_RANGE) - SPAWN_RANGE/2.f;	
		//printf("%f %f %f \n", planet->position.x , planet->position.y , planet->position.z);
		nob->velocity.x = ((float)rand_interval(0,30) - 15.f) * 1;//0.1f; 
		nob->velocity.z = ((float)rand_interval(0,30) - 15.f) * 1;//0.1f; 
		nob->velocity.y = ((float)rand_interval(0,30) - 15.f) * 1;//0.1f; 
		nob->type = Flocker;
		nob->curQueryTime = (float)(i % NUM_QUERIES_PER_SEC) * 0.01f;
		reduce_vec3_inplace(&nob->velocity,MAX_FLOCK_VEL);

		PUSH_NEW_OBJ(game->objects,nob);
		PUSH_NEW_OBJ(game->rend,planet);
		nob->dims = eng->model_cache[SpaceShip].nativeScale;
		nob->treeIndex = insert_to_tree(&game->tree,nob);
	}
	Object* player = new_object(&game->obAll);
	renderData* ship = player->base;
	ship->material.diffuse = SpaceshipTexture;
	ship->modelId = SpaceShip;
	ship->position.x = 0;
	ship->position.y = 0;
	ship->position.z = 0;
	player->type = Player;
	PUSH_NEW_OBJ(game->objects,player);
	PUSH_NEW_OBJ(game->rend,ship);
	player->dims = eng->model_cache[SpaceShip].nativeScale;
	player->treeIndex = insert_to_tree(&game->tree,player);
	player->velocity.x = -10.f;
	// if(is_key_activated(&eng->inputs,KEY_O))
	// {
	// 	remove_node(&game->tree,BACK(game->objects)->treeIndex);
	// 	game->objects.num--;
	// 	game->rend.num--;
	// }


}
void update_objects(ObjectBuffer* objs,float dt,ObjectBuffer* buffer,
		AABBtree* tree,DebugRend* drend,EngineCamera* camera,EngineInputs* inputs,
		ObjectAllocator* all, ModelHandle* models,renderDataBuffer* 	rend);

EXPORT void update_game(void* p)
{
	Engine* eng = (Engine*)p;
	Game* game = eng->userdata;

	float camSpeed = 0.3f;

	/*
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

	   if (/!mouse_init ||/ eng->inputs.inputsDisabled)
	   {

	   update_engine_camera(&eng->camera, eng->inputs.mousePos, eng->inputs.mousePos);
	   }
	   else
	   {
	   update_engine_camera(&eng->camera, eng->inputs.mousePos, eng->inputs.lastMousepos);
	   }
	   */	
	update_objects(&game->objects,eng->DT,&game->raybuffer,&game->tree,
				&eng->drend,&eng->camera, &eng->inputs,&game->obAll,eng->model_cache,&game->rend);


	// ObjectBuffer temp;
	// INITARRAY(temp);

	static float pp = 0.f;
	pp += 0.02f;
	vec3 potemp = {pp,pp,pp};
	draw_index(&game->tree,&eng->drend,game->objects.buff[1]->treeIndex);
	//	for(int i = 0; i < game->objects.num; i++)
	//	{
	//		assert( game->objects.buff[i]->base->modelId == Planet1);
	//		float posLen = vec3_lenght(& game->objects.buff[i]->base->position);
	//		assert(posLen < 500);
	//	}
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
	//assert(eng->sizeOfRenderArray == 10);
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
#define RAY_AREA 15.f
#define MAX_FORCE 5.f
#define MAX_SPEED 10.f
#define STEER_MULT 1.0f

const float QUERY_FREQ = 1.f/(float)NUM_QUERIES_PER_SEC;

float map(float min,float max,float norm,float dmin,float dmax)
{
	float n = (norm - min) / (max - min);
	return (dmax - dmin) * n + dmin; // lerp
}

void update_objects(ObjectBuffer* objs,float dt,ObjectBuffer* buffer,
		AABBtree* tree,DebugRend* drend,EngineCamera* camera,EngineInputs* inputs,
		ObjectAllocator* all,ModelHandle* models,renderDataBuffer* 	rend)
{

	vec3 target = {0};
	int updates = 0;
	float maxRange = RAY_AREA * RAY_AREA;
	static float temprot = 0.f;
	for(int i = 0;i < objs->num; i++)
	{

		Object* currentOb = objs->buff[i];
		if(currentOb->type == Flocker)
		{

			assert(currentOb->base->modelId == SpaceShip);
			if(currentOb->numNeigh < MAX_NEIGHS)
			{
				query_area(tree,currentOb->base->position,RAY_AREA,buffer);

				for(int i2 = currentOb->numNeigh;i2 < MAX_NEIGHS; i2++)
				{
					if(buffer->num == 0) break;
					if( BACK((*buffer)) == currentOb) continue;
					if(BACK((*buffer))->type != Flocker) continue;
					POP_ARRAY_COPY( (*buffer),currentOb->neighbours[i2]);
					currentOb->numNeigh++;
				}	
			}
			vec3 alingment = {0};
			vec3 cohesion = {0};
			vec3 separation = {0};

			//separation
			float numSeparations = 0;
			for(int i2 = 0; i2 < currentOb->numNeigh; i2++)
			{
				//check if we need to find new neighbour
				vec3 distVec = {0};
				neg_vec3(&distVec,&currentOb->base->position,&((Object*)currentOb->neighbours[i2])->base->position);
				float range = vec3_multi_lenght(&distVec); 
				/*currentOb->base->position.x - ((Object*)currentOb->neighbours[i2])->base->position.x * 
				  currentOb->base->position.x - ((Object*)currentOb->neighbours[i2])->base->position.x +
				  currentOb->base->position.y - ((Object*)currentOb->neighbours[i2])->base->position.y *
				  currentOb->base->position.y - ((Object*)currentOb->neighbours[i2])->base->position.y +
				  currentOb->base->position.z - ((Object*)currentOb->neighbours[i2])->base->position.z * 
				  currentOb->base->position.z - ((Object*)currentOb->neighbours[i2])->base->position.z;*/
				float separateRange = (currentOb->dims.x * currentOb->dims.x + currentOb->dims.y * currentOb->dims.y +
						currentOb->dims.z + currentOb->dims.z)*2.f * 1.5;

				//if(((Object*)currentOb->neighbours[i2])->type != Flocker)
				//{
					//assert(0);
					
				//}
				if(range > maxRange || ((Object*)currentOb->neighbours[i2])->type != Flocker)
				{
					Object* one = currentOb->neighbours[currentOb->numNeigh - 1];
					Object* two = currentOb->neighbours[i2];
					if(i2 == currentOb->numNeigh - 1)
					{
						currentOb->numNeigh--;
					}	
					else
					{
						currentOb->numNeigh--;
						currentOb->neighbours[i2] = currentOb->neighbours[currentOb->numNeigh];
						i2--;
					}
					continue;
				}
				if(range < separateRange)
				{
					numSeparations++;	
					normalize_vec3(&distVec);		
					range = sqrt(range);	
					distVec.x /= range + 0.001f;	
					distVec.y /= range + 0.001f;	
					distVec.z /= range + 0.001f;	
					add_vec3(&separation,&separation,&distVec);
				}		

			}
			if(numSeparations > 0)
			{
				separation.x /= numSeparations;	
				separation.y /= numSeparations;	
				separation.z /= numSeparations;	

				normalize_vec3(&separation);
				scale_vec3(&separation,&separation,MAX_SPEED);
				//steering
				neg_vec3(&separation,&separation,&currentOb->velocity);
				reduce_vec3_inplace(&separation,MAX_FORCE);
			}
			if(currentOb->numNeigh > 0)
			{

				//allingment
				for(int i2 = 0; i2 < currentOb->numNeigh;i2++)
				{
					add_vec3(&alingment,&alingment,&((Object*)currentOb->neighbours[i2])->velocity);
				}
				alingment.x /= currentOb->numNeigh;
				alingment.y /= currentOb->numNeigh;
				alingment.z /= currentOb->numNeigh;
				normalize_vec3(&alingment);
				scale_vec3(&alingment,&alingment,MAX_SPEED);
				sub_vec3(&alingment,&alingment,&currentOb->velocity);
				reduce_vec3_inplace(&alingment,MAX_FORCE);
				// cohesion
				for(int i2 = 0; i2 < currentOb->numNeigh;i2++)		
				{
					add_vec3(&cohesion,&cohesion,&((Object*)currentOb->neighbours[i2])->base->position);
					/*
					 *	 	cohesionvel.x /= buffer->num;
					 cohesionvel.y /= buffer->num;
					 cohesionvel.z /= buffer->num;
					/*
					 *PVector desired = PVector.sub(target,location);
					 desired.normalize();
					 desired.mult(maxspeed);

					 Create a new PVector to store the steering force.
					 PVector steer = PVector.sub(desired,velocity);
					 steer.limit(maxforce);
					 return steer;
					 *
					 * 	
					 vec3 desired = {0};
					 neg_vec3(&desired,&cohesionvel, &objs->buff[i]->base->position);
					 normalize_vec3(&desired);
					 scale_vec3(&desired,&desired,MAX_SPEED);
					 neg_vec3(&desired,&desired,&objs->buff[i]->velocity);
					 reduce_vec3_inplace(&desired,MAX_FORCE);

					 * */
				}
				cohesion.x /= currentOb->numNeigh;
				cohesion.y /= currentOb->numNeigh;
				cohesion.z /= currentOb->numNeigh;
				vec3 desired = {0};
				neg_vec3(&desired,&cohesion,&currentOb->base->position);
				normalize_vec3(&desired);
				scale_vec3(&desired,&desired,MAX_SPEED);
				neg_vec3(&desired,&desired,&currentOb->velocity);
				reduce_vec3_inplace(&desired,MAX_FORCE);
				cohesion = desired;


				scale_vec3(&cohesion,&cohesion,2.f);
				scale_vec3(&alingment,&alingment,5.f);
				scale_vec3(&separation,&separation,3.5f);
				add_vec3(&currentOb->acceleration,&currentOb->acceleration,&cohesion);
				add_vec3(&currentOb->acceleration,&currentOb->acceleration,&alingment);
				add_vec3(&currentOb->acceleration,&currentOb->acceleration,&separation);
			}
			float dist = vec3_lenght(&currentOb->base->position);
			//if (dist > 100)
			//{

			//steer
			vec3 desired = {0};
			neg_vec3(&desired,&target,&currentOb->base->position);
			normalize_vec3(&desired);
			scale_vec3(&desired,&desired,MAX_SPEED);
			neg_vec3(&desired,&desired,&currentOb->velocity);


			float steer_scale = map(0,100,dist,0,STEER_MULT);
			//printf("%.4f\n", steer_scale);	
			//scale_vec3(&desired,&desired,steer_scale);
			scale_vec3(&desired,&desired,steer_scale);
			reduce_vec3_inplace(&desired,MAX_FORCE );	
			add_vec3(&currentOb->acceleration,&currentOb->acceleration,&desired);
			//}


			currentOb->velocity.x += currentOb->acceleration.x * dt;
			currentOb->velocity.y += currentOb->acceleration.y * dt;
			currentOb->velocity.z += currentOb->acceleration.z * dt;

			reduce_vec3_inplace(&currentOb->velocity,MAX_SPEED);	


			currentOb->base->position.x += currentOb->velocity.x * dt;
			currentOb->base->position.y += currentOb->velocity.y * dt;
			currentOb->base->position.z += currentOb->velocity.z * dt;


			currentOb->treeIndex = update_object_in_tree(tree,currentOb->treeIndex);

			currentOb->acceleration.x = 0;
			currentOb->acceleration.y = 0;
			currentOb->acceleration.z = 0;
			float addOnX = 0;	
			//printf("%f\n",pi );
			if(currentOb->velocity.x < 0 && currentOb->velocity.z > 0 || currentOb->velocity.x < 0 && currentOb->velocity.z < 0 )
			{
				addOnX = pi;
			}
			else if(currentOb->velocity.x > 0 && currentOb->velocity.z < 0)
			{
				addOnX = 2 * pi;
			}

			float velocityZ = fabs(currentOb->velocity.z);

			//if(currentOb->velocity.z < 0 && currentOb->velocity.y > 0)
			//{
			//		addOnZ = pi / 2.f;	
			//}
			//else if(currentOb->velocity.z < 0 && currentOb->velocity.y < 0)
			//{
			//		addOnZ = -pi / 2.f;
			//}
			//if(currentOb->velocity)
			//
			float xRot = atanf(currentOb->velocity.z / (currentOb->velocity.x != 0 ? currentOb->velocity.x : 0.00001f)) + addOnX - pi;
			//float yRot = atanf(currentOb->velocity.y / (velocityZ != 0 ? velocityZ : 0.00001f ));

			mat4 rotater = {0};
			vec3 axis = {0.f,1.f,0.f};
			create_rotate_mat4(&rotater,axis,xRot);
			vec4 rotatedVel = {currentOb->velocity.x,currentOb->velocity.y,currentOb->velocity.z, 1.f};
			mat4_mult_vec4_inplace(&rotater,&rotatedVel);
			float yRot = rotatedVel.x == 0 ? pi/2.f : 
				atanf(rotatedVel.y / rotatedVel.x);
			yRot *= -1.f;
			//float zRot = atanf(currentOb->velocity.x / currentOb->velocity.y);
			//xRot = xRot == 0 || fabs(xRot) == pi / 2.f ? xRot + 0.01f : xRot;
			//yRot = xRot == 0 || fabs(yRot) == 90.f ? yRot + 0.01f : yRot;
			if(yRot > 1.53f)
				yRot = 1.53f;
			if(yRot < -1.53f)
				yRot = -1.53f;
			float turnRate = 0.01f;
			float lastXRot = currentOb->base->Rotation.y;
			if(fabs(lastXRot - xRot) < turnRate)
			{			
				currentOb->base->Rotation.y = xRot;//temprot;//0.f;//xRot;
			}
			else
			{
				float addX =  lastXRot - xRot < 0 ? turnRate : -turnRate;
				currentOb->base->Rotation.y += addX;
			}
			float lastZrotation = currentOb->base->Rotation.z;
			if(fabs(lastZrotation - (-yRot)) < turnRate)
			{
				currentOb->base->Rotation.z = -yRot;
			}
			else
			{
				float addZ =  lastZrotation - (-yRot) < 0 ? turnRate : -turnRate;
				currentOb->base->Rotation.z += addZ;
			}
			vec3 tempBoxPos = currentOb->velocity;
			normalize_vec3(&tempBoxPos);
			scale_vec3(&tempBoxPos,&tempBoxPos,5.f);
			add_vec3(&tempBoxPos,&tempBoxPos,&currentOb->base->position);
			vec3 boxdim = {0.5f,0.5f,0.5f};
			//draw_box(drend,tempBoxPos,boxdim);
			//currentOb->base->Rotation.z = -yRot;
			//if(!BETWEEN(-90,-yRot,90)) assert(0);


			assert(BETWEEN(-1.57f,-yRot,1.57f));
			//		currentOb->base->Rotation.z = yRot;
			buffer->num = 0;
		}
		else if(currentOb->type == Player)
		{	
			{

				const float ShootRate = 0.2f;
				const float bulletSpeed = 40.f;
				currentOb->lastShot += dt;
				if(is_key_down(KEY_E,inputs))
				{
					if(currentOb->lastShot > ShootRate)
					{
						currentOb->lastShot = 0.f;
						Object* bullet = new_object(all);
						renderData* data = bullet->base;
						data->position = currentOb->base->position;
						data->modelId = PatePallo;
						data->material.diffuse = PalloTexture; 
						bullet->dims = models[Planet1].nativeScale;
						data->scale = 0.5f;
						bullet->velocity = currentOb->velocity;
						normalize_vec3(&bullet->velocity);
						
						bullet->type = Bullet;
						scale_vec3(&bullet->velocity,&bullet->velocity,bulletSpeed);
						PUSH_NEW_OBJ((*objs),bullet);
						PUSH_NEW_OBJ((*rend),data);

						bullet->treeIndex = insert_to_tree(tree,bullet);
						printf("SHOOOT objs num %d \n", objs->num);		
					
					}
				}
			}
			const float minVelocity = 0.5f;
			const float maxVelocity = 4.f;
			const float turnSensitivity = 0.1f;
			const float maxAcceleration = 3.f;
			float verticalAngle = 0.f;
			float horizontalTurn = 0.f;
			float horizontalAngle = 0.f;
			{
				vec3 addVec = currentOb->velocity;
				if(addVec.x == 0 && addVec.y == 0 && addVec.z == 0) 
				{
					addVec.x = cosf(currentOb->base->Rotation.x) * cosf(currentOb->base->Rotation.y);
					addVec.y = sinf(currentOb->base->Rotation.x);
					addVec.z = sinf(currentOb->base->Rotation.y) * cosf(currentOb->base->Rotation.x);
						
				}

				normalize_vec3(&addVec);
				//vec3 vel = {0};
				//scale_vec3(&vel,&addVec,-minVelocity);
				//currentOb->velocity = vel;

				float xOff = inputs->mousePos.x - inputs->lastMousepos.x;
				float yOff = inputs->mousePos.y - inputs->lastMousepos.y;
				xOff *= turnSensitivity;
				yOff *= turnSensitivity;
				//xOff = 1.f;
				//printf("%.3f , %.3f \n", xOff, yOff);
				vec3 horizontalTurnVector = {0.f,0.f,1.f};
				vec3 verticalTurnVector = {0.f,1.f,0.f};
				float movementX = xOff;//currentOb->xOff - xOff;
				float movementY = yOff;//currentOb->yOff - yOff;
				//currentOb->xOff = xOff;
				//currentOb->yOff = yOff;
				//movementX = -1.f * turnSensitivity;
				//movementY = 0.5f;
				horizontalTurnVector.z *= movementX;
				verticalTurnVector.y *= movementY;
				//vec3 tempVel = currentOb->velocity;
				float addOnAngle = addVec.x < 0 ? pi : 0.f;

				horizontalAngle = (addVec.x != 0 ? atanf(addVec.z / addVec.x) : pi / 2.f) + addOnAngle;
				mat4 RotateMat = {0};
				vec3 axis = {0.f,1.f,0.f};
				create_rotate_mat4(&RotateMat,axis,horizontalAngle);

				vec4 tempRotVel = {addVec.x,addVec.y,addVec.z,1.f};
				mat4_mult_vec4_inplace(&RotateMat,&tempRotVel);
				//printf("UnRotated %.4f %.4f %.4f \n", addVec.x,addVec.y,addVec.z);
				//printf("Rotated %.4f %.4f %.4f \n", tempRotVel.x,tempRotVel.y,tempRotVel.z);
				//printf("angle %.4f\n", rad_to_deg(horizontalAngle));
				//vec3 rotatedHorizontal = {tempRotVel.x,tempRotVel.y,tempRotVel.z};//(vec3)tempRotVel;
				//horizontalTurnVector.x = tempRotVel.x;
				//horizontalTurnVector.y = tempRotVel.w;
				//horizontalTurnVector.z = tempRotVel.z;

				verticalAngle = tempRotVel.x == 0.f ?  pi / 2.f : atanf(tempRotVel.y / tempRotVel.x);
				float NineDeg = pi/ 2.f;	
				if(verticalAngle > NineDeg - 0.1f)
					verticalAngle = NineDeg - 0.1f;
				else if(verticalAngle < -NineDeg + 0.1f)
					verticalAngle = -NineDeg + 0.1f;

				vec3 newAcceleration = {tempRotVel.x,tempRotVel.y,tempRotVel.z};
				add_vec3(&newAcceleration,&newAcceleration,&horizontalTurnVector);
				add_vec3(&newAcceleration,&newAcceleration,&verticalTurnVector);
				tempRotVel.x = newAcceleration.x;
				tempRotVel.y = newAcceleration.y;
				tempRotVel.z = newAcceleration.z;
				tempRotVel.w = 0.f;
				horizontalTurn = tempRotVel.z;
				create_rotate_mat4(&RotateMat,axis,-horizontalAngle);
				mat4_mult_vec4_inplace(&RotateMat,&tempRotVel);



				//add_vec3(&rotatedHorizontal,&rotatedHorizontal,&horizontalTurnVector);
				//rotate back
				//



				//create_rotate_mat4(&RotateMat,axis,-horizontalAngle);
				//tempRotVel.x = rotatedHorizontal.x;
				//tempRotVel.y = rotatedHorizontal.y;
				//tempRotVel.z = rotatedHorizontal.z;
				//tempRotVel.w = 1.f;
				//mat4_mult_vec4_inplace(&RotateMat,&tempRotVel);

				//add_vec3(&currentOb->acceleration,&currentOb->acceleration,&horizontalTurnVector);
				currentOb->acceleration.x = tempRotVel.x;
				currentOb->acceleration.y = tempRotVel.y;
				currentOb->acceleration.z = tempRotVel.z;
				reduce_vec3_inplace(&currentOb->acceleration,maxAcceleration);	
				//printf("Acceleration! %.2f %.2f %.2f \n", tempRotVel.x,tempRotVel.y,tempRotVel.z);
				//	printf("%.2f\n",horizontalAngle);
				//	printf("%.2f %.2f %.2f \n", tempVel.x,tempVel.y,tempVel.z);
			}
			scale_vec3(&currentOb->acceleration,&currentOb->acceleration,dt);
			add_vec3(&currentOb->velocity,&currentOb->acceleration,&currentOb->velocity);
			reduce_vec3_inplace(&currentOb->velocity,maxVelocity);
			//scale_vec3(&currentOb->velocity,&currentOb->velocity,dt);
			//add_vec3(&currentOb->base->position,&currentOb->velocity,&currentOb->base->position);
			currentOb->base->position.x += currentOb->velocity.x * dt;
			currentOb->base->position.y += currentOb->velocity.y * dt;
			currentOb->base->position.z += currentOb->velocity.z * dt;

			//printf("Velocity !%.3f %.3f %.3f \n", currentOb->velocity.x,currentOb->velocity.y,currentOb->velocity.z);


			currentOb->acceleration.x = 0;
			currentOb->acceleration.y = 0;
			currentOb->acceleration.z = 0;
			{
				//float addOnAngle = currentOb->velocity.x < 0 ? pi : 0.f;
				//float angle = currentOb->velocity.x == 0 ?
				//		pi / 2.f : atanf(currentOb->velocity.z / currentOb->velocity.x) + addOnAngle;
				currentOb->base->Rotation.y = horizontalAngle - pi;
				currentOb->base->Rotation.z = -verticalAngle;

				const float rollRate = 0.01f;
				const float maxRoll = (pi / 3.f) - 0.01f;
				//float map(float min,float max,float norm,float dmin,float dmax)
				float absTurn = fabs(horizontalTurn);
				if(absTurn > maxAcceleration)
				{
					absTurn = maxAcceleration;
				}
				//absTurn = 0.f;
				float desiredRoll = map(0.f,maxAcceleration,absTurn, 0.f , maxRoll - 0.00001f);
				//printf("Desired roll %.3f \n", rad_to_deg(desiredRoll));
				assert(desiredRoll < maxRoll && desiredRoll >= 0.f);
				desiredRoll *= horizontalTurn < 0 ? 1.f : -1.f;
				//desiredRoll = horizontalTurn < 0 ? -desiredRoll : desiredRoll;

				//printf("Horizontal %.3f \n", horizontalTurn);
				//printf("Desired roll %.3f \n", rad_to_deg(desiredRoll));
				float diff = currentOb->base->Rotation.x - desiredRoll;

				if(fabs(diff) < rollRate)
				{
					currentOb->base->Rotation.x = desiredRoll;
				}
				else
				{
					if(diff > 0)
					{
						currentOb->base->Rotation.x -= rollRate;
					}
					else
					{	
						currentOb->base->Rotation.x += rollRate;
					}
				}
				//printf("Roll %.3f \n", rad_to_deg(currentOb->base->Rotation.x));
				assert(BETWEEN(-maxRoll,currentOb->base->Rotation.x,maxRoll));
				//	currentOb->base->Rotation.x = maxRoll;

				//printf("Objects angle ! %.3f\n",rad_to_deg(	currentOb->base->Rotation.y));
				//printf("%.4f\n",currentOb->base->Rotation.y);
				//printf("velocity %.2f %.2f %.2f \n", currentOb->velocity.x,currentOb->velocity.y,currentOb->velocity.z);
			}
			{
				static float cameraHeight = 5.f;
				const float cameraBack = 10.f;
				const float Infront = -10.f;
				const vec3 worldUp = {0.f,1.f,0.f};
				vec3 addVec = {0};
				addVec.x = cosf(currentOb->base->Rotation.z) * cosf(currentOb->base->Rotation.y);
				addVec.y = sinf(currentOb->base->Rotation.z);
				addVec.z = sinf(currentOb->base->Rotation.y) * cosf(currentOb->base->Rotation.z);
				normalize_vec3(&addVec);
				vec3 upVector = {0};
				vec3 rightVector = {0};
				cross_product(&rightVector,&addVec,&worldUp);
				normalize_vec3(&rightVector);
				cross_product(&upVector,&rightVector,&addVec);
				normalize_vec3(&upVector);

				vec3 cameraPos = {0};
				scale_vec3(&cameraPos,&addVec, cameraBack);
				vec3 camTopVec = upVector;
				scale_vec3(&camTopVec,&camTopVec,cameraHeight);
				add_vec3(&cameraPos,&cameraPos,&camTopVec);
				add_vec3(&cameraPos,&cameraPos,&currentOb->base->position);
				//movementDir.x = 


				//add_vec3(&camToObj,&cameraPos,&camToObj);
				//float CameraSpeed = 1.f;		
				//vec3 camDistVector = {0};
				//neg_vec3(&camDistVector,&cameraPos,&camera->cameraPos);
				//float camDist = 
				vec3 distV = {0};
				neg_vec3(&distV,&camera->cameraPos,&cameraPos);
				float dist = vec3_lenght(&distV);	

				const float minD = 3.f;
				float cameraVelocity = 1.5f * dt;
				if(dist > minD)
				{	
					reduce_vec3_inplace(&distV,minD);
					add_vec3(&camera->cameraPos,&cameraPos,&distV); 
				}
				if(dist < cameraVelocity)
				{
					camera->cameraPos = cameraPos;
				}
				else
				{
					normalize_vec3(&distV);
					scale_vec3(&distV,&distV,-cameraVelocity);
					add_vec3(&camera->cameraPos,&camera->cameraPos,&distV);
				}



				//create view matrix
				vec3 lookToVec = addVec;
				scale_vec3(&lookToVec,&lookToVec,Infront);
				add_vec3(&lookToVec,&currentOb->base->position,&lookToVec);

				vec3 camToObj = {0};
				neg_vec3(&camToObj,&lookToVec,&camera->cameraPos);
				normalize_vec3(&camToObj);
				vec3 cameraUp = {0};
				cross_product(&cameraUp,&camToObj,&worldUp);
				normalize_vec3(&cameraUp);
				cross_product(&cameraUp,&cameraUp,&camToObj);

				vec3 boxdims = {0.5f,0.5f,0.5f};
				//draw_box(drend,lookToVec,boxdims);


				//camera->cameraPos = cameraPos;
				camera->camUp = cameraUp;
				create_lookat_mat4(&camera->view,&camera->cameraPos,&lookToVec,&camera->camUp);

			}

		}
		else if(currentOb->type == Bullet)
		{
			assert(currentOb->base->modelId == PatePallo);
			currentOb->base->position.x += currentOb->velocity.x * dt;
			currentOb->base->position.y += currentOb->velocity.y * dt;
			currentOb->base->position.z += currentOb->velocity.z * dt;
			
			currentOb->treeIndex = update_object_in_tree(tree,currentOb->treeIndex);
			query_area(tree,currentOb->base->position,currentOb->dims.x * 2. * 2.f,buffer);
			for(int i2 = 0; i2 < buffer->num;i2++)
			{
				Object* collider = buffer->buff[i2];
				if(collider->type == Flocker)
				{
				    collider->type = InvalidType;
				    remove_node(tree,currentOb->treeIndex); 
					free_object(all,currentOb);
				    			
					FAST_DELETE_INDEX((*objs),Object*,i);	
					FAST_DELETE_INDEX((*rend),renderData*,i);	
					break;
				}
			}

			buffer->num = 0;
				
			
		}
		else
		{
		     if(currentOb->type == InvalidType)
			 {
				remove_node(tree,currentOb->treeIndex); 
				free_object(all,currentOb);
				    			
				FAST_DELETE_INDEX((*objs),Object*,i);	
				FAST_DELETE_INDEX((*rend),renderData*,i);
			 }
			 else
			 {
				assert(0);	
			 }
		}
	}	
	temprot += 0.05f;
	//printf("NUM UPDATES %d\n",updates);

}
//TODO aseta looppaus index oikein	
//TODO rabalancee alus nopeammin
//TODO parempi hit response	
//TODO boooooost
