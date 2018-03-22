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
typedef struct
{
	int a;
} myree;
#include "source\camera.c"
#define SCREENWIDHT 800
#define SCREENHEIGHT 600

#define FATALERROR assert(0);
#define FATALERRORMESSAGE(STRING) printf(STRING); assert(0);
typedef struct 
{
	ubyte*	keys;
	ubyte*	lastkeys;
	vec2	mousepos;
	vec2	lastMousepos;

} InputManager;



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

static InputManager in;
void set_key(int key,ubyte state)
{
	in.keys[key] = state;
}
void set_mouse( float x, float y)
{
	in.mousepos.x = x;
	in.mousepos.y = y;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_RELEASE)
	set_key( key, action == GLFW_PRESS ? 1 : 0);
}
static ubyte mouse_init = 0;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	set_mouse((float)xpos, (float)ypos);
	if(!mouse_init)
	{
		mouse_init = 1;
		in.lastMousepos = in.mousepos;
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
int key_pressed(int key)
{

	return in.keys[key] == 1 && in.lastkeys[key] == 0;
}
int key_down(int key)
{
	return in.keys[key] == 1;
}
int key_released(int key)
{
	return in.keys[key] == 0 && in.lastkeys[key] == 1;
}




void init_keys()
{
	int i = GLFW_KEY_LAST;
	in.keys = calloc(GLFW_KEY_LAST,sizeof(ubyte));
	in.lastkeys = calloc(GLFW_KEY_LAST, sizeof(ubyte));
	in.mousepos.x = (float)SCREENWIDHT / 2.f;
	in.mousepos.y = (float)SCREENHEIGHT / 2.f;
	in.lastMousepos.x = in.mousepos.x;
	in.lastMousepos.y = in.mousepos.y;
}
void dipose_inputs()
{
	free(in.keys);
	free(in.lastkeys);

}
void update_keys()
{
	memcpy(in.lastkeys, in.keys, sizeof(ubyte)*GLFW_KEY_LAST);
	if (!mouse_init) return;
	in.lastMousepos = in.mousepos;
}


#define SHADER_FILES(FILE)\
		FILE(vert_sha) \
		FILE(frag_sha) \
		FILE(light_vert)\
		FILE(light_frag)\
		FILE(model_frag)\
		FILE(model_vert)\


#define TXT_FILES(FILE) \
		FILE(nonefile)  \
		

#define PNG_FILES(FILE) \
		FILE(linux_pingu)\


#define JPG_FILES(FILE) \
		FILE(laatikko)	\

#define MODEL_FILES(FILE) \
		FILE(none)		\
		FILE(teapot)		\
		FILE(teapotnormal)		\

#define GENERATE_ENUM(ENUM) ENUM,

#define GENERATE_STRING(STRING) #STRING".txt",

#define GENERATE_STRINGPNG(STRING) #STRING".png",

#define GENERATE_STRINGJPG(STRING) #STRING".jpg",

#define GENERATE_SHADER_STRING(STRING) "shaders/"#STRING".txt",

#define GENERATE_MODEL_STRING(STRING) "models/"#STRING".obj",


static const char* txt_file_names[] = {
	TXT_FILES(GENERATE_STRING)
	SHADER_FILES(GENERATE_SHADER_STRING)
};

static const char* pic_file_names[] = {
	PNG_FILES(GENERATE_STRINGPNG)
	JPG_FILES(GENERATE_STRINGJPG)
};
static const char* model_file_names[] = {
	GENERATE_MODEL_STRING(GENERATE_STRINGMODEL)
};

enum model_files
{
	MODEL_FILES(GENERATE_ENUM)
	maxmodelfiles
};

enum txt_files
{
	TXT_FILES(GENERATE_ENUM)
	SHADER_FILES(GENERATE_ENUM)
	maxtxtfiles
};


enum picture_files {
	PNG_FILES(GENERATE_ENUM)
	JPG_FILES(GENERATE_ENUM)
	maxpicfiles
};



typedef struct
{
	uint	ID;
	int		widht;
	int		height;
	int		channels;
} Texture;
Texture textureCache[maxpicfiles] = {0};
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
#include "source\objload.c"


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

typedef struct
{
	uint			vao;
	uint			vbo;
	ShaderHandle	shader;
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

	char* vert_s = load_file(light_vert,NULL);
	uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
	free(vert_s);

	char* frag_s = load_file(light_frag,NULL);
	uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
	free(frag_s);
	l->shader.progId = glCreateProgram();
	glAttachShader(l->shader.progId, vertID);
	glAttachShader(l->shader.progId, fragID);

	add_attribute(&l->shader, "vertexPosition");

	link_shader(&l->shader, vertID, fragID);

	use_shader(&l->shader);
	unuse_shader(&l->shader);


	TeaPot = load_model(teapot);

	glBindVertexArray(0);
	glGenVertexArrays(1, &l->vao);
	glBindVertexArray(l->vao);


	glGenBuffers(1, &l->vbo);
	//glGenBuffers(1, &EBO);


	glBindBuffer(GL_ARRAY_BUFFER, l->vbo);
	glBufferData(GL_ARRAY_BUFFER, /*sizeof(verticesBOX)*/sizeof(vec3)*TeaPot.vertexsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, /*sizeof(verticesBOX)*/sizeof(vec3)*TeaPot.vertexsize, /*verticesBOX*/TeaPot.vertexbuffer);


	// we only need to bind to the VBO, the container's VBO's data already contains the correct data.
	// set the vertex attributes (only position data for our lamp)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

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
int main()
{

	mat4 root = { 0 };
	vec3 aaxis = { 0,1,0 };
	vec4 pooint = { 0,1,0,1};

	create_rotate_mat4(&root, aaxis, deg_to_rad(90.f));
	vec4 res = { 0 };
	mat4_mult_vec4(&res, &root, &pooint);
	
	identity(&root);
	rotate_mat4_X(&root, deg_to_rad(90.f));
	vec4 pooint1 = { 0,1,0,1 };
	vec4 res2 = { 0 };
	mat4_mult_vec4(&res2, &root,&pooint1);
	int a = 0;
//	rotate_mat4_X()

	/*unsigned char* kkkkkkk = NULL;
	kkkkkkk = "hei";
	printf("%s", kkkkkkk);*/


	//mat4 ort = { 0 };
	//identity(&ort);
	//vec3 rotateAxis = { 1.0f, 0.0f, 0.0 };
	//rotate_mat4(&ort, &ort, rotateAxis, deg_to_rad(-55.f));

	////rotate_mat4(&ortRotarer, rotateAxis, degrees_to_radians(-55.0f));

	////identify(&ortRotarer);
	//mat4 per = { 0 };
	//perspective(&per, degrees_to_radians(45.0f), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT, SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
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
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	init_keys();

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

	Light light = { 0 };
	init_light(&light);

	glCheckError();

	float camSpeed = 0.05f;
	//vec3 camDir = { 0.f , 0.f , -1.f };
	Camera camera = { 0 };
	init_camera(&camera);


	mat4 lampRotater = { 0 };
	vec3 lampAxis = { 1.f, 1.f, 0.f};
	create_rotate_mat4(&lampRotater, lampAxis, deg_to_rad(0.01f));
	vec3 oldLightPos = { 10.f , 0.f , -10.f };

	
	/*ModelHandle TeaPotNormal = load_model(teapotnormal);
	{
	
		ShaderHandle modelnormalshader = { 0 };
		char* vert_sModel = load_file(model_vert, NULL);
		uint vertIDModel = compile_shader(GL_VERTEX_SHADER, vert_sModel);
		free(vert_sModel);

		char* frag_sModel = load_file(model_frag, NULL);
		uint fragIDModel = compile_shader(GL_FRAGMENT_SHADER, frag_sModel);
		free(frag_sModel);
		modelnormalshader.progId = glCreateProgram();
		glAttachShader(modelnormalshader.progId, vertIDModel);
		glAttachShader(modelnormalshader.progId, fragIDModel);

		add_attribute(&modelnormalshader, "vertexPosition");
		add_attribute(&modelnormalshader, "normal");

		link_shader(&modelnormalshader, vertID, fragID);

		use_shader(&modelnormalshader);
		unuse_shader(&modelnormalshader);
	
	}*/



	Renderer rend = { 0 };
	init_renderer(&rend);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if (key_pressed(GLFW_KEY_ESCAPE))
		{
			break;
		}
		if (key_down(GLFW_KEY_W))
		{
			vec3 addvec;
			scale_vec3(&addvec, &camera.cameraDir, camSpeed);
			add_vec3(&camera.cameraPos, &camera.cameraPos, &addvec);
		}
		if (key_down(GLFW_KEY_A))
		{
			vec3 addvec;
			cross_product(&addvec, &camera.cameraDir, &camera.camUp);
			normalize_vec3(&addvec);
			scale_vec3(&addvec, &addvec, camSpeed);
			neg_vec3(&camera.cameraPos, &camera.cameraPos, &addvec);
		}
		if (key_down(GLFW_KEY_D))
		{
			vec3 addvec;
			cross_product(&addvec, &camera.cameraDir, &camera.camUp);
			normalize_vec3(&addvec);
			scale_vec3(&addvec, &addvec, camSpeed);
			add_vec3(&camera.cameraPos, &camera.cameraPos, &addvec);
		}
		if (key_down(GLFW_KEY_S))
		{
			vec3 addvec;
			scale_vec3(&addvec, &camera.cameraDir, -camSpeed);
			add_vec3(&camera.cameraPos, &camera.cameraPos, &addvec);;
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		
#ifdef reee
		vec4 tempL = { oldLightPos.x,oldLightPos.y,oldLightPos.z,1.f };
		vec4 resL = { 0 };
		mat4_mult_vec4(&resL, &lampRotater, &tempL);
		vec3 newlightPos = { resL.x,resL.y,resL.z };
		
		oldLightPos = newlightPos;

		//render_boxes(&shader, VBO,VAO,projectionLOC,modelLOC,viewLOC,oldLightPos,&camera,&projection);
#endif
		perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
		render_light(light, &camera, &projection, newlightPos);


		printf("%.2f %.2f %.2f \n", newlightPos.x, newlightPos.y, newlightPos.z);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glCheckError();
		vec3 pos = { 0 };

		Material cube = { 0 };
		vec3 diff = { 0.9f, 0.9f, 0.9f };
		vec3 spec = { 0.9f, 0.9f, 0.9f };
		float shine = 12.0f;

		cube.diffuse = diff;
		cube.specular = spec;
		cube.shininess = shine;
		


		LightValues pro = { 0 };
		vec3 ambL = { 0.7f, 0.7f, 0.7f };
		vec3 diffL = { 0.5f, 0.5f, 0.5f };
		vec3 specL = { 1.0f, 1.0f, 1.0f };
		pro.position = newlightPos;
		pro.ambient = ambL;
		pro.diffuse = diffL;
		pro.specular = specL;

		pro.constant = 1.f;
		pro.linear = 0.1f;
		pro.quadratic = 0.032f;

		render(&rend, teapot, pos, pos, 0.5f, cube, pro, &camera, 0);

		if (!mouse_init)
		{
			update_camera(&camera, in.mousepos, in.mousepos);
			
		}
		else
		{
			update_camera(&camera, in.mousepos, in.lastMousepos);
		}
		update_keys();
		glfwSwapBuffers(window);
	}
	dispose_model_memory();
	dipose_inputs();
	glfwTerminate();
	return 1;
}
