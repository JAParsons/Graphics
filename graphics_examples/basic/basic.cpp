/* Basic GLFW3 Example using an OpenGL context.
   This is the most basic OpenGL program I can write. It is all in one source code file.


   To build this source file in Visual Studio
	- Create lib and include directories
	- Copy the GLFW and GLLoad libs (and/or GLEW libs) into the lib directory
	- Copy the GLLoad and GLFW headers (and/or GLEW headers) into the include directory
	- Add the lib path to the VC++ lib directory (right click on project, then properties)
	- Add the include path to the VC++ include directory (right click on project, then properties)
	- Note that you may have to rebuild the libs from source if they don't work on
	  your system
   */

// Test to see if I can get GLEW to work with GLFW 3.2.1
#define USE_GLLOAD

// When testing with GLEW, I'm using the staticaly built library sp need to define this
//#define GLEW_STATIC

/* Link to libraries, could define these as linker inputs in the project settings instead if you prefer */
// Link to GLFW (window system)
#if defined(_DEBUG)
	#pragma comment(lib, "glfw3D.lib")
#else
	#pragma comment(lib, "glfw3.lib")
#endif

// Link to GLEW OR GLLOAD (OpenGL Function extensions)
#if defined(USE_GLLOAD) && defined (_DEBUG)
	#pragma comment(lib, "glloadD.lib")
#elif defined(USE_GLLOAD)
	#pragma comment(lib, "glload.lib")
#elif defined (_DEBUG)
	#pragma comment(lib, "glew32sd.lib")
#else
	#pragma comment(lib, "glew32s.lib")
#endif

// Link to basic OpenGL functionality (not enough on its own)
#pragma comment(lib, "opengl32.lib")

/* Include standard libraries and the GLFW3 header */
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

/* Include GL_Load OR GLEW  to give access to OpenGL functions*/
#ifdef USE_GLLOAD
	#include <glload/gl_4_0.h>
	#include <glload/gl_load.h>
#else
	#include <GL/glew.h>
#endif

/* Include GLFW which is our simple Window system */
#include <GLFW/glfw3.h>

/* Define some global objects that we'll use to render */
GLuint positionBufferObject;
GLuint program;
GLuint vao;
GLfloat x;
GLfloat y;
GLfloat inc;

/* Array of vertex positions */
GLfloat vertexPositions[] = {
	0.75f, 0.75f, 0.0f, 1.0f,
	0.75f, -0.75f, 0.0f, 1.0f,
	-0.75f, -0.75f, 0.0f, 1.0f,
};

GLfloat vertexPositions2[] = {
	0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.0f, 1.0f,
};

/* Build shaders from strings containing shader source code */
GLuint BuildShader(GLenum eShaderType, const std::string &shaderText)
{
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = shaderText.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		// Output the compile errors
		
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch (eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		std::cerr << "Compile error in " << strShaderType << "\n\t" << strInfoLog << std::endl;
		delete[] strInfoLog;

		throw std::exception("Shader compile exception");
	}

	return shader;
}

/* Error callback, outputs error to stl error stream */
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

/* Key response callback */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		x += 0.02;

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		x -= 0.02;
}

/* Window reshape callback
   Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}





/* Our own initialisation function */
void init()
{
	/* Define animation variables */
	x = 0;
	y = 0;
	inc = 0.0003f;

	glEnable(GL_PROGRAM_POINT_SIZE);

	//set draw mode to wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	/* Create a vertex buffer object to store our array of vertices */
	/* A vertext buffer is a memory object that is created and owned by 
	   the OpenGL context */

	/* Generate buffer names (unique index identifiers) */
	glGenBuffers(1, &positionBufferObject);

	/* Specify the current active buffer object by identifer */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);

	/* Allocates OpenGL memory for storing data or indices, any data 
	   previously defined is deleted*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_DYNAMIC_DRAW);

	/* Stop using buffer object for target (GL_ARRAY_BUFFER) because buffer name = 0*/
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Define the vertex shader code as a string */
	const std::string vertexShader(
		"#version 330\n"
		"layout(location = 0) in vec4 position;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = position;\n"
		"}\n"
		);

	/* Define the fragment shader as a string */
	const std::string fragmentShader(
		"#version 330\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
		"}\n"
		);

	/* Build both shaders */
	GLuint vertShader = BuildShader(GL_VERTEX_SHADER, vertexShader);
	GLuint fragShader = BuildShader(GL_FRAGMENT_SHADER, fragmentShader);

	/* Create a shader program object and link the vertex and fragment shaders
	into a single shader program */
	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	/* Output and shader compilation errors */
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
		throw std::runtime_error("Shader could not be linked.");
	}
}


/* Rendering function */
void display()
{
	vertexPositions[0] = x;
	vertexPositions[9] = x;

	/* Update the vertext buffer object with the modified array of vertices */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_DYNAMIC_DRAW);

	/* Define the background colour*/
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set the current shader program to be used */
	glUseProgram(program);

	/* Set the current active buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);

	/* Specifies where the dat values accociated with index can accessed in the vertex shader */
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable  the vertex array associated with the index*/
	glEnableVertexAttribArray(0);

	/* Constructs a sequence of geometric primitives using the elements from the currently
	   bound matrix */
	glDrawArrays(GL_TRIANGLES, 0, 3);

	/* Disable vertex array and shader program */
	glDisableVertexAttribArray(0);
	glUseProgram(0);
}

/* Standard main progrm */
int main(void)
{
    GLFWwindow* window;

	/* Register the error callback first to enable any GLFW errors to be processed*/
    glfwSetErrorCallback(error_callback);

	/* Init GLFW */
    if (!glfwInit())
        exit(EXIT_FAILURE);

	/* Create a GLFW window, bail out if it doesn't work */
    window = glfwCreateWindow(1920, 1080, "I changed the name", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

	/* Associate an OpenGL context with the recently created GLFW window */
    glfwMakeContextCurrent(window);

#ifdef USE_GLLOAD
	/* Initialise GLLoad library. You must have obtained a current OpenGL context or this will fail */
	/* We need this to link to modern OpenGL function*/
	if (!ogl_LoadFunctions())
	{
		fputs("oglLoadFunctions() failed. Exiting", stderr);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
#else
	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

#endif

	/* Register callbacks for keyboard and window resize */
    glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, reshape);

	/* Call our own function to perform any setup work*/
	init();

	/* The event loop */
    while (!glfwWindowShouldClose(window))
    {
		/* Call our own drawing function */
		display();

		/* Swap buffers: GLFW is double buffered as standard */
        glfwSwapBuffers(window);

		/* Processes registered events, causes the callbacks to be called.*/
        glfwPollEvents();

		/* Modify our animation variables */
		//x += inc;
		//if (x >= 2 || x <= 0) inc = -inc;
    }

	/* Clean up */
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
