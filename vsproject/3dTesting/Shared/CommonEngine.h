
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
#define BIT_UNSET(a,b) (a &= b)
#define BETWEEN(a,x,b) (a < x && b > x)

#ifdef ENGINE_SIDE
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

static inline void set_engine_key(EngineInputs* in,uint key)
{
    int realKey = key - GLFW_KEY_A + 1;
    if(BETWEEN(0,realKey,30))
    {
        BIT_SET(in->keys, (1 << (realKey - 1)));
    }
}
static inline release_engine_key(EngineInputs* in,uint key)
{
	BIT_UNSET(in->keys, key);
}
static inline void update_engine_keys(EngineInputs* in)
{
	in->lastKeys = in->keys;
	in->lastMousepos = in->mousePos;
}
static inline set_engine_mouse(EngineInputs* in,double x,double y)
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
	// fron pitäs olla 0 0 2
	create_lookat_mat4(&c->view, &c->cameraPos, &front, &c->camUp);
}

typedef struct
{
	EngineInputs	inputs;
	EngineCamera	camera;
} Engine;

static inline void init_engine(Engine* en)
{
	init_engine_inputs(&en->inputs);
	init_engine_camera(&en->camera);
}
//static inline void rotate_engine_camera(EngineCamera* cam, float yaw, float roll)
//{
//
//}
