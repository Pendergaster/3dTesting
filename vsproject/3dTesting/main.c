#include <stdio.h>
#include <conio.h>
#include <glad\glad.h>
#include <glfw3.h>
#define MATH_IMPLEMENTATION
#include<mathutil.h>

#define SCREENWIDHT 800
#define SCREENHEIGHT 600

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


static InputManager* in;
void set_key(int key,ubyte state)
{
	in->keys[key] = state;
}
void set_mouse( float x, float y)
{
	in->mouseX = x;
	in->mouseY = y;
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

	return in->keys[key] == 1 && in->lastkeys[key] == 0;
}
int key_down(int key)
{
	return in->keys[key] == 1;
}
int key_released(int key)
{
	return in->keys[key] == 0 && in->lastkeys[key] == 1;
}




void init_keys()
{
	in = calloc(1, sizeof(InputManager));
	in->keys = calloc(GLFW_KEY_LAST,sizeof(ubyte));
	in->lastkeys = calloc(GLFW_KEY_LAST, sizeof(ubyte));
	in->mouseX = 0;
	in->mouseY = 0;
}
void dipose_inputs()
{
	free(in->keys);
	free(in->lastkeys);
	free(in);
}
void update_keys()
{
	memcpy(in->lastkeys, in->keys, sizeof(ubyte)*GLFW_KEY_LAST);
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