/*
 Basic example to demonstrate transformations using a uniform
 Iain Martin September 2018
*/


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* GLM headers */
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation */
#include "wrapper_glfw.h"
#include <iostream>

using namespace std;
using namespace glm;

GLuint positionBufferObject;
GLuint program;
GLuint vao;

/* Uniforms*/
GLuint modelID;

/* Position and view globals */
GLfloat angle_pos, angle_inc, x, model_scale;

/*
This function is called before entering the main rendering loop.
Use it for all you initialisation stuff
*/
void init(GLWrapper *glw)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float vertexPositions[] = {
		0.75f, 0.75f, 0.0f, 1.0f,
		0.75f, -0.75f, 0.0f, 1.0f,
		-0.75f, -0.75f, 0.0f, 1.0f,
	};

	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	try
	{
		program = glw->LoadShader("..\\..\\shaders\\basic_transforms.vert", "..\\..\\shaders\\basic_transforms.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");

	/* Define animation variables */
	x = 0;
	angle_pos = 0;
	angle_inc = 0;
	model_scale = 1.f;
}

//Called to update the display.
//You should call glfwSwapBuffers() after all of your rendering to display what you rendered.
void display()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);

	// Model matrix : an identity matrix (model will be at the origin)
	mat4 model = mat4(1.0f);
	model = rotate(model, -angle_pos, vec3(0, 0, 1));//rotating in clockwise direction around z-axis
	model = scale(model, vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model = translate(model, vec3(x, 0.0f, 0.0f));

	// Send our transformations to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_pos += angle_inc;
}


/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	// Uncomment this line out to disable continuous key presses
//	if (action != GLFW_PRESS) return;

	cout << "KEY: " << (char)key << endl;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc -= 0.002f;
	if (key == 'W') angle_inc += 0.002f;
	if (key == 'Z') model_scale -= 0.01f;
	if (key == 'X') model_scale += 0.01f;
	if (key == 'A') x -= 0.02f;

	if (key == 'S') x += 0.02f;
}

/* An error callback function to output GLFW errors*/
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Hello Graphics World");

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);
	glw->setErrorCallback(error_callback);

	glw->DisplayVersion();
	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}



