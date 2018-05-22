
#define PNG_FILES(FILE) \
		FILE(MoonTexture)\
		FILE(SpaceshipTexture)\
		FILE(flare)\
		//FILE(criss_cross_pattern)\
		//FILE(linux_pingu)\


#define JPG_FILES(FILE) \
		FILE(PalloTexture)	\
		
#define MODEL_FILES(FILE) \
		FILE(Planet1)		\
		FILE(SpaceShip)		\
		FILE(PatePallo)		\

#define TGA_FILES(FILE) \
		FILE(starfield_lf)\
		FILE(starfield_rt)\
		FILE(starfield_up)\
		FILE(starfield_dn)\
		FILE(starfield_ft)\
		FILE(starfield_bk)\
	
#define GENERATE_ENUM(ENUM) ENUM,


#define GENERATE_STRINGPNG(STRING) #STRING".png",

#define GENERATE_STRINGJPG(STRING) #STRING".jpg",

#define GENERATE_STRINGTGA(STRING) #STRING".tga",

#define GENERATE_MODEL_STRING(STRING) "models/"#STRING".obj",


enum model_files
{
	MODEL_FILES(GENERATE_ENUM)
	maxmodelfiles
};

enum picture_files {
	PNG_FILES(GENERATE_ENUM)
	JPG_FILES(GENERATE_ENUM)
	maxpicfiles
};

enum skybox_files
{
	TGA_FILES(GENERATE_ENUM)
	maxskyboxfiles
};
enum EngineKeys
{
	KEY_A = 1 << 0/*= GLFW_KEY_A*/,
	KEY_B = 1 << 1,
	KEY_C = 1 << 2,
	KEY_D = 1 << 3,
	KEY_E = 1 << 4,
	KEY_F = 1 << 5,
	KEY_G = 1 << 6,
	KEY_H = 1 << 7,
	KEY_I = 1 << 8,
	KEY_J = 1 << 9,
	KEY_K = 1 << 10,
	KEY_L = 1 << 11,
	KEY_M = 1 << 12,
	KEY_N = 1 << 13,
	KEY_O = 1 << 14,
	KEY_P = 1 << 15,
	KEY_Q = 1 << 16,
	KEY_R = 1 << 17,
	KEY_S = 1 << 18,
	KEY_T = 1 << 19,
	KEY_U = 1 << 20,
	KEY_V = 1 << 21,
	KEY_W = 1 << 22,
	KEY_X = 1 << 23,
	KEY_Y = 1 << 24,
	KEY_MAX = 1 << 30,
	//max_keys
};

int SCREENWIDHT = 1400;
int SCREENHEIGHT = 900;

typedef struct
{
	uint32_t	keys;
	uint32_t	lastKeys;
	vec2		mousePos;
	vec2		lastMousepos;
	ubyte		inputsDisabled;
} EngineInputs;


// a state b key
#define BIT_CHECK(a,b) ((a & b) > 0)
#define BIT_SET(a,b) ( a |= b)
#define BIT_UNSET(a,b) (a &= ~b)
#define BETWEEN(a,x,b) (a < x && b > x)

static inline void init_engine_inputs(EngineInputs* in)
{
	in->keys = 0;
	in->lastKeys = 0;
	in->mousePos.x = (float)SCREENWIDHT / 2.f;
	in->mousePos.y = (float)SCREENHEIGHT / 2.f;
	in->lastMousepos.x = in->mousePos.x;
	in->lastMousepos.y = in->mousePos.y;
	in->inputsDisabled = 0;
}

#ifdef ENGINE_SIDE
static inline void set_engine_key(EngineInputs* in,uint key)
{
    int realKey = key - GLFW_KEY_A + 1;
    if(BETWEEN(0,realKey,30))
    {
        BIT_SET(in->keys, (1 << (realKey - 1)));
    }
}
static inline void release_engine_key(EngineInputs* in,uint key)
{
	int realKey = key - GLFW_KEY_A + 1;
	if (BETWEEN(0, realKey, 30))
	{
		BIT_UNSET(in->keys, (1 << (realKey - 1)));
	}
}
static inline void update_engine_keys(EngineInputs* in)
{
	in->lastKeys = in->keys;
	in->lastMousepos = in->mousePos;
}
static inline void set_engine_mouse(EngineInputs* in,double x,double y)
{
	in->mousePos.x = (float)x;
	in->mousePos.y = (float)y;
}
#endif


static inline uint is_key_activated(EngineInputs* in,uint key)
{
	return	BIT_CHECK(in->keys, key) && !(BIT_CHECK(in->lastKeys, key));
}

uint is_key_down(uint key,EngineInputs* in)
{
    return BIT_CHECK(in->keys,key);
}


typedef struct
{
	mat4	view;
	vec3	cameraPos;// = { 0.f,0.f,3.f };
	vec3	cameraDir;
	vec3	camUp;
	float	yaw;
	float	pitch;
	//float	fov;
} EngineCamera;
static const vec3 camDir = { 0.f , 0.f , -1.f };
static const vec3 worldUP = { 0.f,1.f,0.f };

static inline void init_engine_camera(EngineCamera* c)
{
	identity(&c->view);
	vec3 temp = { 0 };
	vec3 cameraPos = { 0.f,0.f,3.f };
	vec3 cameraTarget = { 1.f,0.f,0.f };

	c->cameraPos = cameraPos;
	c->cameraDir = camDir;

	create_lookat_mat4(&c->view, &c->cameraPos, &c->cameraDir, &worldUP);

	c->camUp.x = 0.f;
	c->camUp.y = 1.f;
	c->camUp.z = 0.f;
	c->yaw = -90.0f;
	c->pitch = 0;
}

static inline void update_engine_camera(EngineCamera* c, vec2 newMousePos, vec2 lastMousePos)
{
	float xOff = newMousePos.x - lastMousePos.x;
	float yOff = lastMousePos.y - newMousePos.y;

	const float sensitivity = 0.05f;
	xOff *= sensitivity;
	yOff *= sensitivity;


	c->yaw += xOff;
	c->pitch += yOff;

	if (c->pitch > 85.0f)
		c->pitch = 85.0f;
	if (c->pitch < -85.0f)
		c->pitch = -85.0f;

	c->cameraDir.x = cosf(deg_to_rad(c->pitch))*cosf(deg_to_rad(c->yaw));
	c->cameraDir.y = sinf(deg_to_rad(c->pitch));
	c->cameraDir.z = sinf(deg_to_rad(c->yaw))*cosf(deg_to_rad(c->pitch));

	normalize_vec3(&c->cameraDir);

	vec3 front = { 0 };
	add_vec3(&front, &c->cameraPos, &c->cameraDir);


	cross_product(&c->camUp, &c->cameraDir, &worldUP);
	normalize_vec3(&c->camUp);
	cross_product(&c->camUp, &c->camUp, &c->cameraDir);
	normalize_vec3(&c->camUp);
	//normalize_vec3(&front);
	// fron pit�s olla 0 0 2
	create_lookat_mat4(&c->view, &c->cameraPos, &front, &c->camUp);
}



typedef struct
{
	uint diffuse;
	vec3 specular;
	float shininess;
} Material;
static const Material DEFAULT_MATERIAL = { .diffuse = 0,.specular = { .x = 0.7f ,.y = 0.7f,.z = 0.7f },.shininess = 32.f };

typedef struct
{
	vec3	position;
	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
	float	constant;
	float	linear;
	float	quadratic;
} LightValues;

typedef struct
{
	int			modelId;
	vec3		Rotation;
	vec3		position;
	Material	material;
	float		scale;
	vec3        lightScale;
} renderData;

static const renderData DEFAULT_RENDERDATA = { .modelId = Planet1 ,.Rotation = { 0 },.position = { 0 },.material = { .diffuse = 0,.specular = { .x = 0.7f ,.y = 0.7f,.z = 0.7f },.shininess = 32.f },.scale = 1, .lightScale = {.x =  1.0f,.y = 1.0f,.z = 1.0f} };


typedef struct
{
	uint	vertexsize;
	uint	vao;
	uint	vbo;
	uint	nbo;
	uint	uvbo;
	vec3	nativeScale;
	//vec3 	colorScale; 
} ModelHandle;

typedef struct
{
	uint	ID;
	int		widht;
	int		height;
	int		channels;
} Texture;


//static vert_buffer verts;

CREATEDYNAMICARRAY(vec3, vert_buffer)
CREATEDYNAMICARRAY(int, index_buffer)

typedef struct
{
	uint				vao;
	uint				vbo;
	uint				ibo;
	vert_buffer			verts;
	index_buffer		indexes;
	//vec3*				verst;
	//uint				numVerts;
	//int*				indexes;
	int					numIndicies;

} DebugRend;

#ifdef  GAME_SIDE


static inline void draw_line(DebugRend* rend, const vec3 pos1, const vec3 pos2)
{
	vec3* vertArray = NULL;
	int ind = rend->verts.num;
	GET_NEW_BLOCK(rend->verts, vertArray, 2);
	vertArray[0] = pos1;
	vertArray[1] = pos2;

	PUSH_NEW_OBJ(rend->indexes, ind);
	PUSH_NEW_OBJ(rend->indexes, ind + 1);
}

void draw_box(DebugRend* rend, const vec3 pos, const vec3 dim)
{
	int ind = rend->verts.num;
	vec3* vertArray = NULL;
	GET_NEW_BLOCK(rend->verts, vertArray, 8);
	vec3 p1 = { .x = pos.x - dim.x ,.y = pos.y - dim.y,.z = pos.z - dim.z };
	vec3 p2 = { .x = pos.x - dim.x ,.y = pos.y + dim.y,.z = pos.z - dim.z };
	vec3 p3 = { .x = pos.x + dim.x ,.y = pos.y + dim.y,.z = pos.z - dim.z };
	vec3 p4 = { .x = pos.x + dim.x ,.y = pos.y - dim.y,.z = pos.z - dim.z };

	vec3 p5 = { .x = pos.x - dim.x ,.y = pos.y - dim.y,.z = pos.z + dim.z };
	vec3 p6 = { .x = pos.x - dim.x ,.y = pos.y + dim.y,.z = pos.z + dim.z };
	vec3 p7 = { .x = pos.x + dim.x ,.y = pos.y + dim.y,.z = pos.z + dim.z };
	vec3 p8 = { .x = pos.x + dim.x ,.y = pos.y - dim.y,.z = pos.z + dim.z };

	vertArray[0] = p1;
	vertArray[1] = p2;
	vertArray[2] = p3;
	vertArray[3] = p4;

	vertArray[4] = p5;
	vertArray[5] = p6;
	vertArray[6] = p7;
	vertArray[7] = p8;

	int *indbuff = NULL;
	GET_NEW_BLOCK(rend->indexes, indbuff,24);


	indbuff[0] = ind++;
	indbuff[1]  = ind;
	indbuff[2]  = ind++;
	indbuff[3]  = ind;
	indbuff[4]  = ind++;
	indbuff[5]  = ind;
	indbuff[6]  = ind;
	indbuff[7]  = (ind++) -3;
		
	indbuff[8]  = ind++;
	indbuff[9]  = ind;
	indbuff[10]  = ind++;
	indbuff[11]  = ind;
	indbuff[12]  = ind++;
	indbuff[13]  = ind;
	indbuff[14]  = ind;
	indbuff[15]  = (ind++ )- 3;
	

	indbuff[16] = indbuff[0];
	indbuff[17] = indbuff[8];

	indbuff[18] = indbuff[2];
	indbuff[19] = indbuff[10];

	indbuff[20] = indbuff[4];
	indbuff[21] = indbuff[12];

	indbuff[22] = indbuff[6];
	indbuff[23] = indbuff[14];
	
}

#endif //  GAME_SIDE

typedef struct
{
		vec2	times; //x starttime, y lifetime 
		vec3  	startPosition;
		vec3 	velocity;
		float 	scale;
} Particle;
typedef struct
{
		Particle* 	particles;
		uint		vao;
		uint 		buffer;
		int 		currentParticleIndex;
		float       startTime;
} ParticleSystem;

#define MAX_PARTICLES 10000
#ifdef GAME_SIDE



enum SpawerType
{
		Cone,
};

typedef struct 
{
	uint type;
	union
	{
		struct //cone
		{
				float spawnRate;
				float spawnArea;
				float xangle;
				float yangle;
				float timer;
				vec3  position;
				float velocity;
				float scale;
				float lifeTime;
				float pitch;
				float yaw;
				vec3  upVec;
		};
	};
} ParticleSpawner;

static const ParticleSpawner DEFAULT_PARTICLESPAWNER = {.type = Cone,.spawnRate = 0.2f,.spawnArea = 1,
		.xangle = 0.2f,.yangle = 0.2f, .timer = 0.f,
		.position = {0},.velocity = 20.f,.scale = 0.5f,.lifeTime = 10.f, .pitch = 0.f, .yaw = 0.f ,.upVec = {0.f, 1.0f, 0.f}};

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


static void spawn_particle(ParticleSystem* ps,vec3 pos,vec3 velocity,float scale,float lifeTime,float spawnTime);

static void update_spawners(ParticleSystem* ps,ParticleSpawner* spawners, uint amount,float dt,float currentTime)
{
		for(int i = 0; i < amount; i++)
		{
				ParticleSpawner* current = &spawners[i];
				if(current->type == Cone)
				{
					current->timer += dt;
					if(current->timer >= dt)
					{
						float pitch = current->pitch;
						float yaw = current->yaw;
						
						pitch += rand_interval(0,(int)(current->xangle * 100 * 2)) * 0.01 - current->xangle;
						yaw += rand_interval(0,(int)(current->yangle * 100 * 2)) * 0.01 - current->yangle;



						current->timer = 0;
						vec3 dir = {0};
						dir.x = cosf(pitch) * cosf(yaw);		
						dir.y = sinf(pitch);		
						dir.z = sinf(yaw) * cosf(pitch);		

						normalize_vec3(&dir);
						scale_vec3(&dir,&dir,current->velocity);

						
						spawn_particle(ps,current->position,dir,current->scale,current->lifeTime,currentTime);	
						


					}
				}
		}
}

static inline void update_spawner(ParticleSystem* ps,ParticleSpawner* current,float dt,float currentTime )
{
				if(current->type == Cone)
				{
					current->timer += dt;
					if(current->timer >= current->spawnRate)
					{
						float time =  current->timer - current->spawnRate ;
						do
						{
								time -= current->spawnRate;
						float pitch = current->pitch;
						float yaw = current->yaw;
						
						pitch += rand_interval(0,(int)(current->xangle * 100 * 2)) * 0.01 - current->xangle;
						yaw += rand_interval(0,(int)(current->yangle * 100 * 2)) * 0.01 - current->yangle;



						current->timer = 0;
						vec3 dir = {0};
						dir.x = cosf(pitch) * cosf(yaw);		
						dir.y = sinf(pitch);		
						dir.z = sinf(yaw) * cosf(pitch);		

						normalize_vec3(&dir);
						scale_vec3(&dir,&dir,current->velocity);
						
						vec3 rightVec = {0};
						cross_product(&rightVec, &current->upVec, &dir);
						normalize_vec3(&rightVec);
						float randX = rand_interval(0,(int)(current->spawnArea * 100 * 2)) * 0.01 - current->spawnArea;
						float randY = rand_interval(0,(int)(current->spawnArea * 100 * 2)) * 0.01 - current->spawnArea;

						//printf("randX %f \n",randX);
						//printf("randY %f \n",randY);
						vec3 tempVec = current->upVec;	
						scale_vec3(&tempVec,&tempVec,randX);
						scale_vec3(&rightVec,&rightVec,randY);
						add_vec3(&tempVec,&tempVec,&rightVec);
						reduce_vec3_inplace(&tempVec,current->spawnArea);

						vec3 endPos = {0};
						add_vec3(&endPos,&current->position,&tempVec);
							

						spawn_particle(ps,endPos,dir,current->scale,current->lifeTime,currentTime);	
						}	
						while(time > current->spawnRate);


					}
				}
}

static void spawn_particle(ParticleSystem* ps,vec3 pos,vec3 velocity,float scale,float lifeTime,float spawnTime)
{
	ps->particles[ps->currentParticleIndex].startPosition = pos;	
	ps->particles[ps->currentParticleIndex].times.x = spawnTime;	
	ps->particles[ps->currentParticleIndex].times.y = lifeTime; 	
    ps->particles[ps->currentParticleIndex].velocity = velocity; 	
	ps->particles[ps->currentParticleIndex++].scale = scale;
	if(ps->currentParticleIndex >= MAX_PARTICLES) ps->currentParticleIndex = 0;
}

#endif

typedef struct
{
	EngineInputs	inputs;
	EngineCamera	camera;
	ModelHandle		model_cache[maxmodelfiles];
	Texture			textureCache[maxpicfiles];
	uint			skyBoxID;
	uint			skyBoxvbo;
	uint			skyBoxvao;
	renderData**		renderArray;
	uint			sizeOfRenderArray;
	void*			userdata;
	DebugRend		drend;
	float			DT;
	float    		currentTime;
	ParticleSystem  PS;
	float 			distScale;
} Engine;

#ifdef GAME_SIDE


static inline void init_engine(Engine* en)
{
	init_engine_inputs(&en->inputs);
	init_engine_camera(&en->camera);
	en->renderArray = NULL;
	en->sizeOfRenderArray = 0;
	en->userdata = NULL;
	INITARRAY(en->drend.verts);
	INITARRAY(en->drend.indexes);
}

static inline void dispose_engine(Engine* en)
{
	DISPOSE_ARRAY(en->drend.verts);
	DISPOSE_ARRAY(en->drend.indexes);
}
#endif // GAME_SIDE
//static inline void rotate_engine_camera(EngineCamera* cam, float yaw, float roll)
//{
//
//}
