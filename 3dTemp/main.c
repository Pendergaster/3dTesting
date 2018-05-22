#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <glad/glad.h>
#include <glfw3.h>
#include <assert.h>
#define MATH_IMPLEMENTATION
#include<mathutil.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdarg.h>

#include <nuklear.h>
#include <nuklear_glfw_gl3.h>
#include <Windows.h>
#include <smallDLLloader.h>
#ifdef MULTITHREAD
#define THREAD_IMPLEMENTATION
#endif
#include <kthp.h>

FILETIME Win32GetLastWriteTime(const char* path)
{
	FILETIME time = { 0 };
	WIN32_FILE_ATTRIBUTE_DATA data;

	if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
		time = data.ftLastWriteTime;

	return time;
}
int MEMTRACK = 0;

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

typedef struct
{
	int a;
} myree;
#include "source\camera.c"


#define SHADER_FILES(FILE)\
		FILE(vert_sha) \
		FILE(frag_sha) \
		FILE(light_vert)\
		FILE(light_frag)\
		FILE(model_frag)\
		FILE(model_vert)\
		FILE(debug_vert)\
		FILE(debug_frag)\
		FILE(skybox_vert)\
		FILE(skybox_frag)\
		FILE(frame_vert)\
		FILE(frame_frag)\
		FILE(blur_vert)\
		FILE(blur_frag)\
		FILE(pokemon_frag)\
		FILE(particle_frag)\
		FILE(particle_vert)\


#define TXT_FILES(FILE) \
		FILE(nonefile)  \


#define ENGINE_SIDE
#include <CommonEngine.h>

#define GENERATE_STRING(STRING) #STRING".txt",

#define GENERATE_SHADER_STRING(STRING) "shaders/"#STRING".txt",

const char* txt_file_names[] = {
	TXT_FILES(GENERATE_STRING)
	SHADER_FILES(GENERATE_SHADER_STRING)
};
static const char* pic_file_names[] = {
	PNG_FILES(GENERATE_STRINGPNG)
	JPG_FILES(GENERATE_STRINGJPG)
};
static const char* skybox_names[] = 
{
	TGA_FILES(GENERATE_STRINGTGA)
};


static const char* model_file_names[] = {
	MODEL_FILES(GENERATE_MODEL_STRING)
};

enum txt_files
{
	TXT_FILES(GENERATE_ENUM)
	SHADER_FILES(GENERATE_ENUM)
	maxtxtfiles
};

FILETIME LASTWRITES[maxtxtfiles];

#define FATALERROR assert(0);
#define FATALERRORMESSAGE(STRING) printf(STRING); assert(0);


//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	glViewport(0, 0, width, height);
//}

float fov = 45.f;
float fovSens = 1.5f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

	if (fov >= 1.0f && fov <= 45.0f)
		fov -= (float)yoffset * fovSens;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
static Engine* engptr;
////static InputManager in;
//void set_key(int key,ubyte state)
//{
//	in.keys[key] = state;
//}
//void set_mouse( float x, float y)
//{
//	in.mousepos.x = x;
//	in.mousepos.y = y;
//}
//
//



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		set_engine_key(&engptr->inputs, key);
	}
	else if (action == GLFW_RELEASE)
	{
		release_engine_key(&engptr->inputs, key);
	}
}
static ubyte mouse_init = 0;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	set_engine_mouse(&engptr->inputs,xpos,ypos);
	if(!mouse_init)
	{
		mouse_init = 1;
		engptr->inputs.lastMousepos = engptr->inputs.mousePos;
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
		}
		else if (action == GLFW_RELEASE)
		{
		}
	}
}


Texture* textureCache = NULL;

void loadTexture(const int file)
{
	if(textureCache[file].ID != 0)
	{
		return;
	}
	Texture* tex = &textureCache[file];
	//int* k = malloc(1000);
	//k[3] = 0;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex->ID);
	glBindTexture(GL_TEXTURE_2D, tex->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data = stbi_load(pic_file_names[file], &tex->widht, &tex->height, &tex->channels,0);
	if(!data)
	{
		printf("FAILED TO LOAD PICTURE %s\n", pic_file_names[file]);
		FATALERROR;
	}
	if (tex->channels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, tex->widht, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (tex->channels == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, tex->widht, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		FATALERROR;
	}
	stbi_image_free(data);
}

#include "source/shaderutils.c"
char* load_file_from_source(const char* file, int* size);
char* load_file(int file,int* size)
{
	char *source = NULL;
	source = load_file_from_source(txt_file_names[file], size);

	return source;
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		char* error = NULL;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			//case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			//case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		//FATALERRORMESSAGE("GL ERROR %s \n", error);	
		printf("GL ERROR %s, LINE %d , FILE %s \n", error, line, file);
 		FATALERROR;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


uint load_skybox(uint* skyvao,uint* skyvbo)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Texture tex ={0};
    glGenTextures(1, &tex.ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex.ID);
	//stbi_set_flip_vertically_on_load(1);
    glCheckError();
    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(skybox_names[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            stbi_image_free(data);
			FATALERROR;
        }
    }

    glCheckError();
	//stbi_set_flip_vertically_on_load(0);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glCheckError();
	float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};


	ShaderHandle* sha = get_shader(skyboxShader);

	LASTWRITES[skybox_frag] = Win32GetLastWriteTime(txt_file_names[skybox_frag]);
	LASTWRITES[skybox_vert] = Win32GetLastWriteTime(txt_file_names[skybox_vert]);


	char* vert_s = load_file(skybox_vert, NULL);
	uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
	free(vert_s);

	char* frag_s = load_file(skybox_frag, NULL);
	uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
	free(frag_s);

	sha->progId = glCreateProgram();
	glAttachShader(sha->progId, vertID);
	glAttachShader(sha->progId, fragID);

	add_attribute(sha, "aPos");

	link_shader(sha, vertID, fragID);
	
    	glCheckError();
	use_shader(sha);
	unuse_shader(sha);
	glCheckError();
	
	uint vao = 0;
	uint vbo = 0;
	glGenVertexArrays(1,&vao);
	glGenBuffers(1,&vbo);
	
	
	assert(vao != 0 && vbo != 0);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);


	//glBindBuffer(GL_ARRAY_BUFFER,vbo);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36, NULL, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 36, skyboxVertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	*skyvao = vao;
	*skyvbo = vbo;
    return tex.ID;
}

char* load_file_from_source(const char* file, int* size)
{
	char *source = NULL;
	FILE *fp = fopen(file, "r");
	if (fp != NULL) {
		/* Go to the end of the file. */
		if (fseek(fp, 0L, SEEK_END) == 0) {
			/* Get the size of the file. */
			long bufsize = ftell(fp);
			if (bufsize == -1) { /* Error */ }

			/* Allocate our buffer to that size. */
			source = malloc(sizeof(char) * (bufsize + 1));

			/* Go back to the start of the file. */
			if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

			/* Read the entire file into memory. */
			size_t newLen = fread(source, sizeof(char), bufsize, fp);
			if (ferror(fp) != 0) {
				fputs("Error reading file", stderr);
				FATALERROR;
			}
			else {
				source[newLen++] = '\0'; /* Just to be safe. */
				if(size)
				{
					*size = (int)newLen;
				}
			}
		}
		fclose(fp);
	}
	else
	{
		printf("FILE NOT FOUND");
		FATALERROR;
	}
	return source;
}


#include "source\objload.c"
typedef struct
{
	/*uint			vao;
	uint			vbo;*/
	ModelHandle		model;
	uint			shader;
} Light;

	
ModelHandle TeaPot;
void init_light(Light* l)
{
	LASTWRITES[light_frag] = Win32GetLastWriteTime(txt_file_names[light_frag]);
	LASTWRITES[light_vert] = Win32GetLastWriteTime(txt_file_names[light_vert]);

	ShaderHandle* s = get_shader(LIGHT);
	char* vert_s = load_file(light_vert,NULL);
	uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
	free(vert_s);

	char* frag_s = load_file(light_frag,NULL);
	uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
	free(frag_s);
	s->progId = glCreateProgram();
	glAttachShader(s->progId, vertID);
	glAttachShader(s->progId, fragID);

	add_attribute(s, "vertexPosition");

	link_shader(s, vertID, fragID);

	use_shader(s);
	unuse_shader(s);
	l->shader = LIGHT;
#if 0
	TeaPot = load_model(Planet1);
	l->model = TeaPot;
#endif


}


#include "source/particle_system.c"
#include "source/modelrendering.c"
void hotload_shaders(double dt);
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
static void error_callback(int e, const char *d)
{
	printf("Error %d: %s\n", e, d);
}


#include "source/nuklear_util.c"
#include "source/debugrendering.c"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
//CREATEDYNAMICARRAY(int, foo)
//CREATEDYNAMICARRAY(vec2, vert_bufferj
//CREATEDYNAMICARRAY(int, index_buffer)
float quadVertices[] = {  
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};	
int main()
{
	//assert(0);	
	int a = 2 + 2;
	Engine engine = {0};
	engptr = &engine;
	textureCache = engine.textureCache;
	model_cache = engine.model_cache;
	memset(engine.textureCache, 0, sizeof(Texture)*maxpicfiles);
	memset(engine.model_cache, 0, sizeof(ModelHandle)*maxmodelfiles);
	
	//memset(engine.skyBoxCache, 0, sizeof(ModelHandle)*maxmodelfiles);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3),
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", monitor, NULL);
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	SCREENWIDHT = mode->width;
	SCREENHEIGHT = mode->height;
	if (window == NULL)
	{
		printf("Failed to create window\n");
		_getch();
		return 0;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		_getch();
		return 0;
	}

	glViewport(0, 0, SCREENWIDHT, SCREENHEIGHT);
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//init_engine(&engine);
	glCheckError();
	struct nk_context *ctx;
	struct nk_colorf bg;


	ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
	{struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&atlas);
	nk_glfw3_font_stash_end(); }
	bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

#define reee


	mat4 view = { 0 };
	identity(&view);
	vec3 traVec = { 0.f , 0.f , -3.f };
	translate_mat4(&view, &view, traVec);

	mat4 projection = { 0 };
	perspective(&projection, deg_to_rad(45.f), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 10000.f);

	glCheckError();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);  
	glEnable(GL_CULL_FACE);  
//glCullFace(GL_FRONT); 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	Renderer rend = { 0 };
	glCheckError(); 
	init_renderer(&rend);
	glCheckError();
	const double dt = 1.0 / 60.0;
	engine.DT = dt;
	double currentTime = glfwGetTime();
	double accumulator = 0.0;

	float time[200] = { 0 };
	const float fpsUpdateRate = 1.0f;
	float lastTime = 0.f;
	float currentFps = 0.f;
	int index = 0;
	float currentFrameTime = 0.f;
	const float frametimeUpTime = 0.5f;
	float frameTimer = 0.f;
	float updating = 0.f;
	//load_model(Planet1);

	glCheckError();

	//ParticleSystem PS = {0};

    init_particle_system(&engine.PS,glfwGetTime());
	///eTexture particleTexture = jkjkjkjiPatePallo);


	//DebugRend debugRned = { 0 };
	init_debugrend(&engine.drend);
	glCheckError();
	mat4 lampRotater = { 0 };
	vec3 lampAxis = { 1.f, 0.f, 0.f};
	create_rotate_mat4(&lampRotater, lampAxis, deg_to_rad(2.f));

	LightValues pro = { 0 };
	//vec3 lightcolor = { sinf(0.2f* glfwGetTime()),sinf( 0.7f* glfwGetTime()), sinf(1.3f * glfwGetTime()) };
	vec3 diffL = { 0.8f, 0.8f, 0.8f };
	//scale_vec3(&diffL, &diffL, 0.5f);
	vec3 ambL = { 0.5f, 0.5f, 0.5f };
	//scale_vec3(&ambL, &ambL, 0.2f);
	vec3 specL = { 1.0f, 1.0f, 1.0f };
	vec3 oldLightPos = {2.f , 2.f , 2.f};
	pro.position = oldLightPos;
	pro.ambient = ambL;
	pro.diffuse = diffL;
	pro.specular = specL;

	pro.constant = 1.f;
	pro.linear = 0.001f;
	pro.quadratic = 0.002f;

	printf("LOADING GAME");
	func_ptr init_game = NULL;
	func_ptr update_game = NULL;
	func_ptr dispose_game = NULL;
	DLLHandle game_dll = {0};
	load_DLL(&game_dll, "DebugBin/game.dll");
	init_game = load_DLL_function(game_dll, "init_game");
	update_game = load_DLL_function(game_dll, "update_game");
	dispose_game = load_DLL_function(game_dll, "dispose_game");
	for(int i = 0; i < maxpicfiles;i++)
	{
		loadTexture(i);
	}
	glCheckError();

	engine.skyBoxID = load_skybox(&engine.skyBoxvao,&engine.skyBoxvbo);
	glCheckError();
	for (int i = 0; i < maxmodelfiles; i++)
	{
		load_model(i);
	}
	init_game(&engine);

	uint FrameBuffer = 0;
	glGenFramebuffers(1,&FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER,FrameBuffer);

    uint FrameTextures[2] = {0u,0u}; // first is scene and second is bright channel
    glGenTextures(2, FrameTextures);
			glCheckError();
	for(int i = 0; i < 2; i++)
	{
			glBindTexture(GL_TEXTURE_2D, FrameTextures[i]);
		  
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREENWIDHT, SCREENHEIGHT, 0, GL_RGBA, GL_FLOAT, NULL); 
			// gl_float for HDR, gl unsigned bute with out

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, FrameTextures[i], 0);  // attach it to framebuffer

	}
	
    

	unsigned int RenderBuffer;
    glGenRenderbuffers(1, &RenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, RenderBuffer);  // bind buffer
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREENWIDHT, SCREENHEIGHT); // create depth and stencil buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBuffer);// attach it!  
    glCheckError();
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Frame buffers arent baked! \n");
		assert(0);
	}
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);  

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // bind default back

			glCheckError();
    {
			ShaderHandle* framSha = get_shader(frameShader);
			LASTWRITES[frame_frag] = Win32GetLastWriteTime(txt_file_names[frame_frag]);
			LASTWRITES[frame_vert] = Win32GetLastWriteTime(txt_file_names[frame_vert]);

			char* fvert = load_file(frame_vert,NULL);
			uint fvid = compile_shader(GL_VERTEX_SHADER,fvert);
			free(fvert);
			fvert = NULL;

		    char* ffrag = load_file(frame_frag,NULL);
			uint ffid = compile_shader(GL_FRAGMENT_SHADER,ffrag);
			free(ffrag);
			ffrag = NULL; 

			framSha->progId = glCreateProgram();
			glAttachShader(framSha->progId,fvid);
			glAttachShader(framSha->progId,ffid);

		    add_attribute(framSha,"aPos");
		    add_attribute(framSha,"aTexCoords");
			link_shader(framSha,fvid,ffid);
			use_shader(framSha);
		    unuse_shader(framSha);
			glCheckError();
	}
	uint pokemonTex = 0;
	{
			ShaderHandle* framSha = get_shader(PokemonProg);
			LASTWRITES[pokemon_frag] = Win32GetLastWriteTime(txt_file_names[pokemon_frag]);
			//LASTWRITES[frame_vert] = Win32GetLastWriteTime(txt_file_names[frame_vert]);

			char* fvert = load_file(frame_vert,NULL);
			uint fvid = compile_shader(GL_VERTEX_SHADER,fvert);
			free(fvert);
			fvert = NULL;

		    char* ffrag = load_file(pokemon_frag,NULL);
			uint ffid = compile_shader(GL_FRAGMENT_SHADER,ffrag);
			free(ffrag);
			ffrag = NULL; 

			framSha->progId = glCreateProgram();
			glAttachShader(framSha->progId,fvid);
			glAttachShader(framSha->progId,ffid);

		    add_attribute(framSha,"aPos");
		    add_attribute(framSha,"aTexCoords");
			link_shader(framSha,fvid,ffid);
			use_shader(framSha);
		    unuse_shader(framSha);
			glCheckError();




			Texture tex = {0};//&textureCache[file];
			//int* k = malloc(1000);
			//k[3] = 0;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glGenTextures(1, &tex.ID);
			glBindTexture(GL_TEXTURE_2D, tex.ID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_REPEAT);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			unsigned char* data = stbi_load("criss_cross_pattern.png", &tex.widht, &tex.height, &tex.channels,0);
			if(!data)
			{
				FATALERROR;
				//FATALERRORMESSAGE("FAILED TO LOAD PICTURE %s\n", pic_file_names[file]);
			}
			if (tex.channels == 3)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.widht, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else if (tex.channels == 4)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.widht, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				FATALERROR;
			}
			stbi_image_free(data);
			pokemonTex = tex.ID;
	}
	uint frameVao = 0;
	{
			glGenVertexArrays(1,&frameVao);
			uint frameVertBuff = 0;
			glGenBuffers(1,&frameVertBuff);
			glBindVertexArray(frameVao);
			glBindBuffer(GL_ARRAY_BUFFER,frameVertBuff);
			glCheckError();
			glEnableVertexAttribArray(0);	
			glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)0);
			glEnableVertexAttribArray(1);	
			glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)(2 * sizeof(float)));
			glCheckError();
			glBufferData(GL_ARRAY_BUFFER,sizeof(float) * 36,NULL,GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(float)* 36, quadVertices);
			glCheckError();
			glBindBuffer(GL_ARRAY_BUFFER,0);
			glCheckError();

			glBindVertexArray(0);
	}
   

// frame buffers for blurring
	unsigned int pingpongFBO[2];
    unsigned int pingpongColorBuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorBuffers);
			glCheckError();
    for (unsigned int i = 0; i < 2; i++)
   {
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGB16F, SCREENWIDHT, SCREENHEIGHT, 0, GL_RGB, GL_FLOAT, NULL
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorBuffers[i], 0
			);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		    {
				printf("Frame buffers arent baked! \n");
				assert(0);
		    }
			glCheckError();

    }
	//blurring program
    {
		ShaderHandle* sha = get_shader(BlurShader);
		//ShaderHandle* framSha = get_shader(frameShader);
		LASTWRITES[blur_frag] = Win32GetLastWriteTime(txt_file_names[blur_frag]);
		LASTWRITES[blur_vert] = Win32GetLastWriteTime(txt_file_names[blur_vert]);

		char* fvert = load_file(blur_vert,NULL);
    	uint fvid = compile_shader(GL_VERTEX_SHADER,fvert);
		free(fvert);
		fvert = NULL;

		char* ffrag = load_file(blur_frag,NULL);
		uint ffid = compile_shader(GL_FRAGMENT_SHADER,ffrag);
		free(ffrag);
		ffrag = NULL;

		sha->progId = glCreateProgram();
		glAttachShader(sha->progId,fvid);
		glAttachShader(sha->progId,ffid);
			glCheckError();

		add_attribute(sha,"aPos");
		add_attribute(sha,"aTexCoords");
		link_shader(sha,fvid,ffid);
		use_shader(sha);
		unuse_shader(sha);
		glCheckError();
	
	}
    uint BlurVao = 0;
	{
			glGenVertexArrays(1,&BlurVao);
			uint frameVertBuff = 0;
			glGenBuffers(1,&frameVertBuff);
			glBindVertexArray(BlurVao);
			glBindBuffer(GL_ARRAY_BUFFER,frameVertBuff);
			glCheckError();
			glEnableVertexAttribArray(0);	
			glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)0);
			glEnableVertexAttribArray(1);	
			glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)(2 * sizeof(float)));
			glCheckError();
			glBufferData(GL_ARRAY_BUFFER,sizeof(float) * 36,NULL,GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(float)* 36, quadVertices);
			glCheckError();
			glBindBuffer(GL_ARRAY_BUFFER,0);
			glCheckError();

			glBindVertexArray(0);
	}

	/*
	*LASTWRITES[model_frag] = Win32GetLastWriteTime(txt_file_names[model_frag]);
		LASTWRITES[model_vert] = Win32GetLastWriteTime(txt_file_names[model_vert]);
		ShaderHandle* shader = get_shader(SHA_PROG_NO_UV);// &rend->withTex;
		rend->noTex = SHA_PROG_NO_UV;
		char* vert_s = load_file(model_vert, NULL);
		uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
		free(vert_s);

		char* frag_s = load_file(model_frag, NULL);
		uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
		free(frag_s);
		shader->progId = glCreateProgram();
		glAttachShader(shader->progId, vertID);
		glAttachShader(shader->progId, fragID);

		add_attribute(shader, "vertexPosition");
		add_attribute(shader, "normal");

		link_shader(shader, vertID, fragID);

		use_shader(shader);
		/*vec3 te = { 0 };
		set_vec3(shader, "material.diffuse", &te);
		unuse_shader(shader);
		glCheckError();
	* */ 
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#ifdef MULTITHREAD
    kth_pool_t 	tp;
	Game       	thread_Game;
	Engine      thread_engine;
#define RENDERDATA_SIZE 100000
    renderData	render_data = malloc(sizeof(renderData) * RENDERDATA_SIZE);		
#define NUM_THREADS 2
    if (kth_pool_init(&tp, NUM_THREADS, 64))
        return 1;

    if (kth_pool_run(&tp))
        return 2;
#endif
	
	//ubyte PostProcess[2];
	//memset(PostProcess, 0 ,sizeof(PostProcess));
    
    //float kernels[2][9] = 
	//{
	//		{
		//1.f,1.f,1.f,1.f,−8.f,1.f,1.f,1.f,1.f
	//	1.0f / 16.f, 2.0f / 16.f, 1.0f / 16.f,
   // 2.0f / 16.f, 4.0f / 16.f, 2.0f / 16.f,
    //1.0f / 16.f, 2.0f / 16.f, 1.0f / 16.f 
	//		},
	//		{
	//	1.f,1.f,1.f,1.f,−8.f,1.f,1.f,1.f,1.f
	//		}
	//};
		ubyte running = 1.f;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		nk_glfw3_new_frame();
		double newTime = glfwGetTime();
		engine.currentTime = (float)newTime;
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;

		time[index++] = (float)newTime - lastTime;
		if (index >= 200)index = 0;
		frameTimer += (float)newTime - lastTime;
		if(frameTimer > frametimeUpTime)
		{
			frameTimer = 0;
			currentFrameTime = (float)newTime - lastTime;
		}
		updating += newTime - lastTime;
		lastTime = (float)newTime;
		if (updating > fpsUpdateRate)
		{
			float add = 0;
			for (int i = 0; i < 200; i++)
			{
				add += time[i];
			}
			currentFps = 1.f / (add / 200.f);
			updating = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		{
			break;
		}
	//	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	//	{
	//		if (!engine.inputs.inputsDisabled)
	//		{
	//			engine.inputs.inputsDisabled = 1;
	//			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//		}
	//		else
	//		{
	//			engine.inputs.inputsDisabled = 0;
	//			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//		}
		//}

		if( show_engine_stats(ctx,currentFps,currentFrameTime,engine.camera.cameraDir,engine.camera.yaw,engine.camera.pitch
								,engine.camera.cameraPos,&engine.distScale))
		{
			dispose_game(&engine);
			load_DLL(&game_dll, "DebugBin/game.dll");
			init_game = load_DLL_function(game_dll, "init_game");
			update_game = load_DLL_function(game_dll, "update_game");
			dispose_game = load_DLL_function(game_dll, "dispose_game");
			init_game(&engine);
		}
		//overview(ctx);
		//calculator(ctx);
		//
		vec3 parPos = {-10,0,0};
		//ParticleSpawner TEST_SPAWNER = DEFAULT_PARTICLESPAWNER;
		//spawn_particle(&PS,parPos,0.5f,5.f, glfwGetTime());
		 glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


#ifdef MULTITHREAD
		thread_engine = engine; 

#endif
		if(is_key_activated(&engine.inputs,KEY_M))
		{
				running = 0;	
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if(is_key_activated(&engine.inputs,KEY_N))
		{
				running = 1;	
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		while(accumulator >= dt)
		{
#ifdef MULTITHREAD
		    	
#endif
		   if(running)
		   {
					update_game(&engine);
		   } 
			accumulator -= dt;

			hotload_shaders(dt);
		
			update_engine_keys(&engine.inputs);

			vec4 tempL = { oldLightPos.x,oldLightPos.y,oldLightPos.z,1.f };
			vec4 resL = { 0 };
			mat4_mult_vec4(&resL, &lampRotater, &tempL);
			vec3 newlightPos = { resL.x,resL.y,resL.z };

			oldLightPos = newlightPos;
			pro.position = oldLightPos;
			static float temp = 0;
			temp += 0.01;
			vec3 pos1 = { 0,0,0 };
			vec3 pos2 = { 20,0,20 };
			vec3 dims = { 1,1,1 };
			//draw_box(&debugRned, pos1, model_cache[Planet1].nativeScale);

			//update_spawners(&PS,&TEST_SPAWNER,1,(float)dt,glfwGetTime());
			populate_debugRend_buffers(&engine.drend);
			glCheckError();

			glCheckError();

			create_buffers_particle(&engine.PS);
			//break;
		}
		//FIRST PASS!
		glBindFramebuffer(GL_FRAMEBUFFER,FrameBuffer);
		
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);

		glEnable(GL_CULL_FACE);  

		perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 10000.f);

		glCheckError();
		vec3 pos = { 0 };

	
		//glEnable(GL_FRAMEBUFFER_SRGB);
		render_models(&rend, engine.renderArray, engine.sizeOfRenderArray, &engine.camera, pro);
		render_debug_lines(&engine.drend,&engine.camera.view);
		glCheckError();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		render_particles(&engine.PS,&engine.camera.view,&projection,glfwGetTime(),engine.textureCache[flare].ID);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCheckError();

#if 1

		glActiveTexture(GL_TEXTURE0);
		ShaderHandle* skysha = get_shader(skyboxShader);
		glDepthFunc(GL_LEQUAL);
		//skyboxShader.use();
		glBindVertexArray(engine.skyBoxvao);
		use_shader(skysha);
		// ... set view and projection matrix
		//mat4 projection = { 0 };
		//perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
		set_mat4(skysha,"projection",projection.mat);
		mat4 skyView = engine.camera.view;
		skyView.mat[3][0] = 0;
		skyView.mat[3][1] = 0;
		skyView.mat[3][2] = 0;
		skyView.mat[3][3] = 1;
		set_mat4(skysha,"view",skyView.mat);
		glBindVertexArray(engine.skyBoxvao);
		glBindTexture(GL_TEXTURE_CUBE_MAP, engine.skyBoxID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);
		unuse_shader(skysha);
		glBindVertexArray(0);

#endif

		//glDisable(GL_FRAMEBUFFER_SRGB);
		

		//SECOND PASS!
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		//Blurr calculations!
		
		ubyte horizontal = 1, first_iteration = 1;
        uint amount = 5;
        //shaderBlur.use();
		glBindVertexArray(BlurVao);
		ShaderHandle* blursha = get_shader(BlurShader);
		use_shader(blursha);
		glActiveTexture(GL_TEXTURE0);
		//TODO tama!!!!
		glDisable(GL_DEPTH_TEST);
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            //shaderBlur.setInt("horizontal", horizontal);
			set_uniform_int(blursha,"horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? FrameTextures[1] : pingpongColorBuffers[!horizontal]);  
			// bind texture of other framebuffer (or scene if first iteration)
		    
		    glDrawArrays(GL_TRIANGLES, 0, 6);  

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = 0;
        }
		unuse_shader(blursha);
		glBindVertexArray(0);
	
   //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		
		//post processing stuff!
		glEnable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindVertexArray(frameVao);
		if(!Pokemon)
		{
				
				ShaderHandle* framSha = get_shader(frameShader);
				use_shader(framSha);

				set_uniform_int(framSha,"screenTexture",0);
				set_uniform_int(framSha,"BlurTexture",1);

				glUniform1fv(glGetUniformLocation(framSha->progId, "kernel"), 9, currentKernel);
				glDisable(GL_DEPTH_TEST);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,FrameTextures[0]); // 0 is normal scene
				glActiveTexture(GL_TEXTURE1);
						
				glBindTexture(GL_TEXTURE_2D,pingpongColorBuffers[!horizontal]);  

		}
		else
		{
		
				ShaderHandle* framSha = get_shader(PokemonProg);
				use_shader(framSha);

				set_uniform_int(framSha,"screenTexture",0);
				set_uniform_int(framSha,"BlurTexture",1);
				set_uniform_int(framSha,"DistTex",2);
				set_uniform_float(framSha,"cutOff",engine.distScale);

				glDisable(GL_DEPTH_TEST);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,FrameTextures[0]); // 0 is normal scene
				glActiveTexture(GL_TEXTURE1);
						
				glBindTexture(GL_TEXTURE_2D,pingpongColorBuffers[!horizontal]);
				glActiveTexture(GL_TEXTURE2);


				glBindTexture(GL_TEXTURE_2D, pokemonTex); // 0 is normal scene


		}
		glDrawArrays(GL_TRIANGLES, 0, 6);  


		glBindVertexArray(0);

		render_nuklear();

		glfwSwapBuffers(window);
	}
	dispose_particles(&engine.PS);
	dispose_game(&engine);
	dispose_model_memory();
	dispose_debug_renderer(&engine.drend);
	glfwTerminate();
	printf("MEMTRACK = %d", MEMTRACK);
	assert(MEMTRACK == 0);
	return 1;
}

void hotload_shaders(double dt)
{
	static double time = 0;
	time += dt;
	if (time > 0.5)
	{
		time = 0;
		if (shader_cache[SHA_PROG_NO_UV].numAttribs != 0 && shader_cache[SHA_PROG_NO_UV].progId != 0)
		{
			FILETIME newTimeFrag = Win32GetLastWriteTime(txt_file_names[model_frag]);
			FILETIME newTimeVert = Win32GetLastWriteTime(txt_file_names[model_vert]);
			if (CompareFileTime(&LASTWRITES[model_frag], &newTimeFrag) || CompareFileTime(&LASTWRITES[model_vert], &newTimeVert))
			{
				uint success = 0;
				printf("LOADING SHADERS \n %s %s \n", txt_file_names[model_frag], txt_file_names[model_vert]);
				do
				{
					LASTWRITES[model_frag] = newTimeFrag;
					LASTWRITES[model_vert] = newTimeVert;

					ShaderHandle tempsha = { 0 };

					char* vert_s = load_file(model_vert, NULL);
					uint vertID = soft_compile_shader(GL_VERTEX_SHADER, vert_s);
					free(vert_s);
					if (vertID == INVALIDSHADER)
					{
						break;
					}

					char* frag_s = load_file(model_frag, NULL);
					uint fragID = soft_compile_shader(GL_FRAGMENT_SHADER, frag_s);
					free(frag_s);
					if (fragID == INVALIDSHADER)
					{
						break;
					}
					tempsha.progId = glCreateProgram();
					glAttachShader(tempsha.progId, vertID);
					glAttachShader(tempsha.progId, fragID);

					add_attribute(&tempsha, "vertexPosition");
					add_attribute(&tempsha, "normal");


					uint suc = soft_link_shader(&tempsha, vertID, fragID);
					if (!suc) break;
					use_shader(&tempsha);
					unuse_shader(&tempsha);
					success = 1;
					dispose_shader(&shader_cache[SHA_PROG_NO_UV]);
					shader_cache[SHA_PROG_NO_UV] = tempsha;
				} while (0);
				if (!success)
				{
					printf("FAILED TO COMPILE SHADERS");
				}
			}
		}
		if (shader_cache[SHA_PROG_UV].numAttribs != 0 && shader_cache[SHA_PROG_UV].progId != 0)
		{
			FILETIME newTimeFrag = Win32GetLastWriteTime(txt_file_names[frag_sha]);
			FILETIME newTimeVert = Win32GetLastWriteTime(txt_file_names[vert_sha]);

			if (CompareFileTime(&LASTWRITES[frag_sha], &newTimeFrag) || CompareFileTime(&LASTWRITES[vert_sha], &newTimeVert))
			{
				LASTWRITES[frag_sha] = newTimeFrag;
				LASTWRITES[vert_sha] = newTimeVert;

				uint success = 0;
				do
				{
					printf("LOADING SHADERS \n %s %s \n", txt_file_names[model_frag], txt_file_names[model_vert]);

					LASTWRITES[frag_sha] = newTimeFrag;
					LASTWRITES[vert_sha] = newTimeVert;

					ShaderHandle tempsha = { 0 };

					char* vert_s = load_file(vert_sha, NULL);
					uint vertID = soft_compile_shader(GL_VERTEX_SHADER, vert_s);
					free(vert_s);
					if (vertID == INVALIDSHADER)
					{
						break;
					}

					char* frag_s = load_file(frag_sha, NULL);
					uint fragID = soft_compile_shader(GL_FRAGMENT_SHADER, frag_s);
					free(frag_s);
					if (fragID == INVALIDSHADER)
					{
						break;
					}
					tempsha.progId = glCreateProgram();
					glAttachShader(tempsha.progId, vertID);
					glAttachShader(tempsha.progId, fragID);

					add_attribute(&tempsha, "vertexPosition");
					add_attribute(&tempsha, "uv");
					add_attribute(&tempsha, "normal");


					uint suc = soft_link_shader(&tempsha, vertID, fragID);
					if (!suc) break;
					use_shader(&tempsha);
					unuse_shader(&tempsha);
					success = 1;
					dispose_shader(&shader_cache[SHA_PROG_UV]);
					shader_cache[SHA_PROG_UV] = tempsha;

				} while (0);
				if (!success)
				{
					printf("FAILED TO COMPILE SHADERS");
				}
			}
		}
		if (shader_cache[LIGHT].numAttribs != 0 && shader_cache[LIGHT].progId != 0)
		{
			FILETIME newTimeFrag = Win32GetLastWriteTime(txt_file_names[light_frag]);
			FILETIME newTimeVert = Win32GetLastWriteTime(txt_file_names[light_vert]);

			if (CompareFileTime(&LASTWRITES[light_frag], &newTimeFrag) || CompareFileTime(&LASTWRITES[light_vert], &newTimeVert))
			{
				LASTWRITES[light_frag] = newTimeFrag;
				LASTWRITES[light_vert] = newTimeVert;

				uint success = 0;
				do
				{
					printf("LOADING SHADERS \n %s %s \n", txt_file_names[light_frag], txt_file_names[light_vert]);

					LASTWRITES[light_frag] = newTimeFrag;
					LASTWRITES[light_vert] = newTimeVert;

					ShaderHandle tempsha = { 0 };

					char* vert_s = load_file(light_vert, NULL);
					uint vertID = soft_compile_shader(GL_VERTEX_SHADER, vert_s);
					free(vert_s);
					if (vertID == INVALIDSHADER)
					{
						break;
					}

					char* frag_s = load_file(light_frag, NULL);
					uint fragID = soft_compile_shader(GL_FRAGMENT_SHADER, frag_s);
					free(frag_s);
					if (fragID == INVALIDSHADER)
					{
						break;
					}
					tempsha.progId = glCreateProgram();
					glAttachShader(tempsha.progId, vertID);
					glAttachShader(tempsha.progId, fragID);

					add_attribute(&tempsha, "vertexPosition");



					uint suc = soft_link_shader(&tempsha, vertID, fragID);
					if (!suc) break;
					use_shader(&tempsha);
					unuse_shader(&tempsha);
					success = 1;
					dispose_shader(&shader_cache[LIGHT]);
					shader_cache[LIGHT] = tempsha;
					

				} while (0);
				if (!success)
				{
					printf("FAILED TO COMPILE SHADERS");
				}
			}

		}

		if (shader_cache[DEBUG_PROG].numAttribs != 0 && shader_cache[DEBUG_PROG].progId != 0)
		{
			FILETIME newTimeFrag = Win32GetLastWriteTime(txt_file_names[debug_frag]);
			FILETIME newTimeVert = Win32GetLastWriteTime(txt_file_names[debug_vert]);

			if (CompareFileTime(&LASTWRITES[debug_frag], &newTimeFrag) || CompareFileTime(&LASTWRITES[debug_vert], &newTimeVert))
			{
				LASTWRITES[debug_frag] = newTimeFrag;
				LASTWRITES[debug_vert] = newTimeVert;

				uint success = 0;
				do
				{
					printf("LOADING SHADERS \n %s %s \n", txt_file_names[debug_frag], txt_file_names[debug_vert]);

					LASTWRITES[debug_frag] = newTimeFrag;
					LASTWRITES[debug_vert] = newTimeVert;

					ShaderHandle tempsha = { 0 };

					char* vert_s = load_file(debug_vert, NULL);
					uint vertID = soft_compile_shader(GL_VERTEX_SHADER, vert_s);
					free(vert_s);
					if (vertID == INVALIDSHADER)
					{
						break;
					}

					char* frag_s = load_file(debug_frag, NULL);
					uint fragID = soft_compile_shader(GL_FRAGMENT_SHADER, frag_s);
					free(frag_s);
					if (fragID == INVALIDSHADER)
					{
						break;
					}
					tempsha.progId = glCreateProgram();
					glAttachShader(tempsha.progId, vertID);
					glAttachShader(tempsha.progId, fragID);
					
					add_attribute(&tempsha, "vertexPosition");

					uint suc = soft_link_shader(&tempsha, vertID, fragID);
					if (!suc) break;
					use_shader(&tempsha);
					unuse_shader(&tempsha);
					success = 1;
					dispose_shader(&shader_cache[DEBUG_PROG]);
					shader_cache[DEBUG_PROG] = tempsha;


				} while (0);
				if (!success)
				{
					printf("FAILED TO COMPILE SHADERS");
				}
			}
		}


	}
}
