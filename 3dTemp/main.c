#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <glad\glad.h>
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
FILETIME Win32GetLastWriteTime(const char* path)
{
	FILETIME time = { 0 };
	WIN32_FILE_ATTRIBUTE_DATA data;

	if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
		time = data.ftLastWriteTime;

	return time;
}
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


#define TXT_FILES(FILE) \
		FILE(nonefile)  \


#define ENGINE_SIDE
#include <CommonEngine.h>

#define GENERATE_STRING(STRING) #STRING".txt",

#define GENERATE_SHADER_STRING(STRING) "shaders/"#STRING".txt",

static const char* txt_file_names[] = {
	TXT_FILES(GENERATE_STRING)
	SHADER_FILES(GENERATE_SHADER_STRING)
};
static const char* pic_file_names[] = {
	PNG_FILES(GENERATE_STRINGPNG)
	JPG_FILES(GENERATE_STRINGJPG)
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


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

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

Texture loadTexture(const int file)
{
	if(textureCache[file].ID != 0)
	{
		return textureCache[file];
	}
	Texture* tex = &textureCache[file];
	//int* k = malloc(1000);
	//k[3] = 0;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex->ID);
	glBindTexture(GL_TEXTURE_2D, tex->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data = stbi_load(pic_file_names[file], &tex->widht, &tex->height, &tex->channels,0);
	if(!data)
	{
		FATALERROR;
		//FATALERRORMESSAGE("FAILED TO LOAD PICTURE %s\n", pic_file_names[file]);
	}
	if (tex->channels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->widht, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (tex->channels == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->widht, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		FATALERROR;
	}
	stbi_image_free(data);

	return *tex;
}
char* load_file_from_source(const char* file, int* size);
char* load_file(int file,int* size)
{
	char *source = NULL;
	source = load_file_from_source(txt_file_names[file], size);

	return source;
}
#include "source/shaderutils.c"
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


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		unsigned char* error = NULL;
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
		printf("GL ERROR %s \n", error);
		FATALERROR;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

#include "source\objload.c"
typedef struct
{
	/*uint			vao;
	uint			vbo;*/
	ModelHandle		model;
	uint			shader;
} Light;

	float verticesBOX[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f,  0.5f, 
		0.5f, -0.5f,  0.5f, 
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};
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
float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,	    0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,	    0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	    0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,	    0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,	    0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,	    0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,		0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,		0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,		0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		  -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		  -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		  -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,		  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  1.0f,  0.0f,
};



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

//CREATEDYNAMICARRAY(int, foo)
//CREATEDYNAMICARRAY(vec2, vert_buffer)
//CREATEDYNAMICARRAY(int, index_buffer)
int main()
{
	//assert(0);	
	Engine engine = {0};
	engptr = &engine;
	textureCache = engine.textureCache;
	model_cache = engine.model_cache;
	memset(engine.textureCache, 0, sizeof(Texture)*maxpicfiles);
	memset(engine.model_cache, 0, sizeof(ModelHandle)*maxmodelfiles);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3),
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
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
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//init_engine(&engine);
	struct nk_context *ctx;
	struct nk_colorf bg;


	ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
	{struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&atlas);
	nk_glfw3_font_stash_end(); }
	bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

#define reee
#ifdef vanha

	//char* you = "vertexPosition"; // , "uv", "normal");
	//ShaderHandle shader = create_shader_stof(3, vert_sha, frag_sha, you);
	ShaderHandle shader = { 0 };
	char* vert_s = load_file(vert_sha,NULL);
	uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
	free(vert_s);

	char* frag_s = load_file(frag_sha,NULL);
	uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
	free(frag_s);
	shader.progId = glCreateProgram();
	glAttachShader(shader.progId, vertID);
	glAttachShader(shader.progId, fragID);

	add_attribute(&shader, "vertexPosition");
	add_attribute(&shader, "uv");
	add_attribute(&shader, "normal");


	link_shader(&shader, vertID, fragID);

	use_shader(&shader);
	unuse_shader(&shader);


	uint VBO, VAO/*, EBO;*/;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glCheckError();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glCheckError();

	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	glCheckError();


	Texture box = loadTexture(laatikko);


	glCheckError();

	use_shader(&shader);
	set_uniform_int(&shader, "tex", 0);

	glCheckError();

#endif // reee

	


	//mat4 model = { 0 };
	//identity(&model);
	//vec3 axis = { 1.f,0.f,0.f };
	//vec3 cubePos = { 3.f, 2.f , 0.f };
	//translate_mat4(&model, &model, cubePos);
	//rotate_mat4(&model, &model, axis, deg_to_rad(-55.f));

	mat4 view = { 0 };
	identity(&view);
	vec3 traVec = { 0.f , 0.f , -3.f };
	translate_mat4(&view, &view, traVec);

	mat4 projection = { 0 };
	perspective(&projection, deg_to_rad(45.f), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);

	glCheckError();

#ifdef vanha

	uint modelLOC = glGetUniformLocation(shader.progId, "model");
	uint viewLOC = glGetUniformLocation(shader.progId, "view");
	uint projectionLOC = glGetUniformLocation(shader.progId, "projection");

	glCheckError();

/*
	glUniformMatrix4fv(modelLOC, 1, GL_FALSE, (GLfloat*)model.mat);
	glUniformMatrix4fv(viewLOC, 1, GL_FALSE, (GLfloat*)view.mat);
	glUniformMatrix4fv(projectionLOC, 1, GL_FALSE, (GLfloat*)projection.mat)*/;

	
	glCheckError();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, box.ID);
	glCheckError();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	unuse_shader(&shader);
#endif
	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	Light light = { 0 };
	init_light(&light);

	glCheckError();

	//vec3 camDir = { 0.f , 0.f , -1.f };
	/*EngineCamera camera = { 0 };
	init_engine_camera(&camera);*/


	mat4 lampRotater = { 0 };
	vec3 lampAxis = { 1.f, 0.f, 0.f};
	create_rotate_mat4(&lampRotater, lampAxis, deg_to_rad(2.f));
	vec3 oldLightPos = { 0.f , 5.f , 0.f };
	



	Renderer rend = { 0 };
	init_renderer(&rend);
	const double dt = 1.0 / 60.0;

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

	//DebugRend debugRned = { 0 };
	init_debugrend(&engine.drend);

	

	LightValues pro = { 0 };
	//vec3 lightcolor = { sinf(0.2f* glfwGetTime()),sinf( 0.7f* glfwGetTime()), sinf(1.3f * glfwGetTime()) };
	vec3 diffL = { 0.8f, 0.8f, 0.8f };
	//scale_vec3(&diffL, &diffL, 0.5f);
	vec3 ambL = { 0.05f, 0.05f, 0.05f };
	//scale_vec3(&ambL, &ambL, 0.2f);
	vec3 specL = { 1.0f, 1.0f, 1.0f };

	pro.position = oldLightPos;
	pro.ambient = ambL;
	pro.diffuse = diffL;
	pro.specular = specL;

	pro.constant = 1.f;
	pro.linear = 0.001f;
	pro.quadratic = 0.002f;
	
	//printf("kokok %f", planet->material.specular.x);
	func_ptr init_game = NULL;
	func_ptr update_game = NULL;
	func_ptr dispose_game = NULL;
	DLLHandle game_dll = {0};
//C:\Users\Pate\Documents\3dTesting\3dTesting\vsproject\3dTesting\game\Project1\x64\Debug
	load_DLL(&game_dll, "DebugBin/game.dll");
	init_game = load_DLL_function(game_dll, "init_game");
	update_game = load_DLL_function(game_dll, "update_game");
	dispose_game = load_DLL_function(game_dll, "dispose_game");
	for(int i = 0; i < maxpicfiles;i++)
	{
		loadTexture(i);
	}
	for (int i = 0; i < maxmodelfiles; i++)
	{
		load_model(i);
	}
	init_game(&engine);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		nk_glfw3_new_frame();
		double newTime = glfwGetTime();

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
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			break;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			if (!engine.inputs.inputsDisabled)
			{
				engine.inputs.inputsDisabled = 1;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else
			{
				engine.inputs.inputsDisabled = 0;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}

		show_engine_stats(ctx,currentFps,currentFrameTime);
		//overview(ctx);
		//calculator(ctx);
		while (accumulator >= dt)
		{
			update_game(&engine);
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

			populate_debugRend_buffers(&engine.drend);

			glCheckError();
		}


		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);

		glCheckError();
		vec3 pos = { 0 };

	
		render_models(&rend, engine.renderArray, engine.sizeOfRenderArray, &engine.camera, pro);
		render_debug_lines(&engine.drend,&engine.camera.view);
		glCheckError();

		render_nuklear();

		glfwSwapBuffers(window);
	}
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
