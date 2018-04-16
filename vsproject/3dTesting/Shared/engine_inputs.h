



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
	KEY_RAND = 1 << 32,
	//max_keys
	/*KEY_Z,
	KEY_B,
	KEY_B,
	KEY_B,
	KEY_B,*/
};

typedef struct
{
	uint32_t	keys;
	uint32_t	lastKeys;
	vec2		mousePos;
	vec2		lastMousepos;
} EngineInputs;

#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
uint is_key_down(uint key,EngineInputs* in)
{
	return BIT_CHECK(in->keys, key);
}