#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <glad\glad.h>
#include <glfw3.h>
#include <assert.h>
#define MATH_IMPLEMENTATION
#include<mathutil.h>

#define SCREENWIDHT 800
#define SCREENHEIGHT 600

#define FATALERROR assert(0);

typedef struct 
{
	ubyte*	keys;
	ubyte*	lastkeys;
	float	mouseX;
	float	mouseY;
} InputManager;



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


static InputManager in;
void set_key(int key,ubyte state)
{
	in.keys[key] = state;
}
void set_mouse( float x, float y)
{
	in.mouseX = x;
	in.mouseY = y;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	set_key( key, action == GLFW_PRESS ? 1 : 0);
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	set_mouse((float)xpos, (float)ypos);
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
	in.mouseX = 0;
	in.mouseY = 0;
}
void dipose_inputs()
{
	free(in.keys);
	free(in.lastkeys);

}
void update_keys()
{
	memcpy(in.lastkeys, in.keys, sizeof(ubyte)*GLFW_KEY_LAST);
}



typedef struct
{
	
	uint	progId;
	int		numAttribs;
} ShaderHandle;

uint compile_shader(uint glenum,const char* source)
{
	uint compilecheck = 0;
	uint shader = glCreateShader(glenum);
	if(shader == 0) FATALERROR;

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compilecheck);

	if(!compilecheck)
	{
		uint infolen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
		if(infolen > 1)
		{
			char* infoLog = malloc(sizeof(char) * infolen);
			glGetShaderInfoLog(shader, infolen, NULL, infoLog);
			printf("Error compiling shader :\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		FATALERROR;
	}
	return shader;
}
void dispose_shader(ShaderHandle* sha)
{
	if (sha->progId)glDeleteProgram(sha->progId);
}
void unuse_shader(ShaderHandle* sha)
{
	glUseProgram(0);
	for (int i = 0; i < sha->numAttribs; i++) {
		glDisableVertexAttribArray(i);
	}
}
void add_attribute(ShaderHandle* shader,const char* name)
{
	printf("Adding attribute %s to program %d", name, shader->progId);
	glBindAttribLocation(shader->progId, shader->numAttribs++, name);
}
uint get_uniform_location(ShaderHandle* shader,const char* name)
{
	GLint location = glGetUniformLocation(shader->progId, name);
	if (location == GL_INVALID_INDEX)
	{
		printf("failed to get uniform");
		FATALERROR;
	}
	return location;
}



void use_shader(ShaderHandle* shader)
{
	glUseProgram(shader->progId);
	for (int i = 0; i < shader->numAttribs; i++) {
		glEnableVertexAttribArray(i);
	}
}

void link_shader(ShaderHandle* shader,uint vert,uint frag)
{
	printf("Linking program %d\n", shader->progId);
	glLinkProgram(shader->progId);
	uint linked = 0;
	glGetProgramiv(shader->progId, GL_LINK_STATUS, &linked);

	if(!linked)
	{
		uint infolen = 0;
		glGetProgramiv(shader->progId, GL_INFO_LOG_LENGTH, &infolen);
		if(infolen > 1)
		{
			char* infolog = malloc(sizeof(char)*infolen);
			glGetProgramInfoLog(shader->progId, infolen, NULL, infolog);
			printf("Error linking program \n %s", infolog);
			free(infolog);
		}
		glDeleteProgram(shader->progId);
		FATALERROR;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT, SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
	if(window == NULL)
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
	init_keys();

	mat4 t = identity();
	mat4 b = identity();
	for(int jj = 0; jj < 4;jj++)
	{
		b.mat[0][jj] = 1;
		b.mat[1][jj] = 2;
		b.mat[2][jj] = 3;
		b.mat[3][jj] = 4;
	}
	mat4 mul = mult_mat4(&b, &b);


	mat4 ortho = orthomat(0.f,800.f,0.f,600.f,0.1f,100.f);
	mat4 proj = perspective(deg_to_rad(45.f), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if(key_pressed(GLFW_KEY_ESCAPE))
		{
			break;
		}
		if(key_pressed(GLFW_KEY_W))
		{
			printf("w pressed");
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update_keys();
		glfwSwapBuffers(window);
	}

	dipose_inputs();
	glfwTerminate();
	return 1;
}