/*
 Terrain example
 Creates a class to define a terrain with an array of vertices and calculates vertex normals
 Example for Lab6
 Iain Martin November 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
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

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "terrain_object.h"

using namespace std;
using namespace glm;

/* Define buffer object indices */
GLuint positionBufferObject, colourObject, normalsBufferObject;
GLuint sphereBufferObject, sphereNormals, sphereColours, sphereTexCoords;
GLuint elementbuffer;

GLuint program, program2;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */


/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;


terrain_object *heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLuint land_resolution;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = -0.55f;
	y = 0;
	z = 0;
	angle_x = 295.f;
	angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 0.33f;
	aspect_ratio = 1.3333f;
	colourmode = 0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Create the heightfield object */
	octaves = 4;
	perlin_scale = 2.f;
	perlin_frequency = 1.f;
	land_size = 20.f;
	land_resolution = 100;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size);
	heightfield->createObject();
	
	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("..\\..\\shaders\\terrain.vert", "..\\..\\shaders\\terrain.frag");
		program2 = glw->LoadShader("..\\..\\shaders\\test.vert", "..\\..\\shaders\\test.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");


}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Define the model transformations matrix
	mat4 model = mat4(1.0f);
	
	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

	/* Define the model transformations for our terrain */
	model = mat4(1.0f);
	model = translate(model, vec3(-x-0.5, 0, 0));
	model = scale(model, vec3(model_scale/3.f, model_scale/3.f, model_scale/3.f));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our heightfield */
	heightfield->drawObject(drawmode);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	bool recreate_terrain = false;		// Set to true if we want to recreate the terrain
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_x -= 0.05f;
	if (key == 'W') angle_inc_x += 0.05f;
	if (key == 'E') angle_inc_y -= 0.05f;
	if (key == 'R') angle_inc_y += 0.05f;
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == 'A') model_scale -= 0.02f;
	if (key == 'S') model_scale += 0.02f;
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}


	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Terrain Basic");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}


