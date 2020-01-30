/*
 Terrain and simple object loader example
 This example also shows how to render a height map and place an object on the terrain.
 The terrain is not transformed to simplify the grid height calculations.
 Use keys z, x, b, n to move the Monkey around and see how it sits on the terrain
 Iain Martin October 2018
*/


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "tiny_loader.h"
#include "terrain_object.h"

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

TinyObjLoader monkey;

terrain_object *heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLfloat sealevel = 0;

using namespace glm;
using namespace std;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0;
	y = 0;
	z = 0;
	angle_x = 295.f;
	angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scaler = 0.33f;
	aspect_ratio = 1.3333f;
	colourmode = 1;

	// Set the view angle to view the terrain
	vx = 330.f; vy = 0, vz = 0.f;
	
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and create our monkey object*/
	monkey.load_obj("..\\..\\obj\\monkey_normals.obj");
	// Set all the colour vertices to blue
	// Note that this is a quick hack, it would be better to load and process the mtl file
	monkey.overrideColour(vec4(0.5f, 0, 1.0, 1.f));

	/* Create the heightfield object */
	octaves = 4;
	perlin_scale = 2.f;
	perlin_frequency = 1.f;
	land_size = 2.f;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(200, 200, land_size, land_size);
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();
	
	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("..\\..\\shaders\\terrain.vert", "..\\..\\shaders\\terrain.frag");
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

	// Place the Mokey object on the terrain at its current start position
	vec2 pos = heightfield->getGridPos(x, z);
	y = heightfield->heightAtPosition(x, z);
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

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Define the model transformations for the Monkey object
	mat4 model = mat4(1.0f);
	model = translate(model, vec3(x, y+0.1f, z));	// +0.1 t0 place on the terrain
	model = scale(model, vec3(0.1f, 0.1f, 0.1f));//scale equally in all axis
	model = rotate(model, -radians(angle_x-50.f), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

	/* Draw our Blender Monkey object */
	monkey.drawObject(drawmode);

	/* Define the model transformations for our Terrain object - we don't transform it to
	   make it easier to place objects on the terrain*/
	model = mat4(1.0f);
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our terrain */

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
	bool placeObject = false;			// Set to true to set our object on the terrain
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
	if (key == 'A') scaler -= 0.02f;
	if (key == 'S') scaler += 0.02f;
	if (key == 'Z') {x -= 0.05f; placeObject = true; }
	if (key == 'X') {x += 0.05f; placeObject = true; }
	if (key == 'B') {z -= 0.05f; placeObject = true; }
	if (key == 'N') {z += 0.05f; placeObject = true; }

	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	if (key == 'O') vz -= 1.f;
	if (key == 'P') vz += 1.f;

	if (key == '1') { sealevel -= 0.5f; recreate_terrain = true; }
	if (key == '2') { sealevel += 0.5f; recreate_terrain = true; }

	if (key == ',' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == '.' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

	if (key == 'P')
	{
		printf("\nScale = %f", scaler);
		printf("\nangle_x=%f", angle_x);
		printf("\nx=%f", x);
	}

	if (key == '[' && action != GLFW_PRESS)
	{
		if (octaves > 1) octaves--;
		recreate_terrain = true;
		printf("\nOctave = %d", octaves);
	}
	if (key == ']' && action != GLFW_PRESS)
	{
		if (octaves < 12) octaves++;
		recreate_terrain = true;
		printf("\nOctave = %d", octaves);
	}

	if (key == 'L' && action != GLFW_PRESS)
	{
		perlin_frequency -= 0.1f;
		recreate_terrain = true;
		printf("\nperlin_frequency = %f", perlin_frequency);
	}

	if (key == ';' && action != GLFW_PRESS)
	{
		perlin_frequency += 0.1f;
		recreate_terrain = true;
		printf("\nperlin_frequency = %f", perlin_frequency);
	}

	if (key == '#' && action != GLFW_PRESS)
	{
		perlin_scale -= 0.1f;
		recreate_terrain = true;
		printf("\nperlin_scale = %f", perlin_scale);
	}

	if (key == '/' && action != GLFW_PRESS)
	{
		perlin_frequency += 0.1f;
		recreate_terrain = true;
		printf("\nperlin_scale = %f", perlin_scale);
	}

	// Keep object on the terrain
	if (placeObject)
	{
		// Debug to output grid position of object
		vec2 pos = heightfield->getGridPos(x, z);
		
		// Set the object height form the grid position
		y = heightfield->heightAtPosition(x, z);
	}

	if (recreate_terrain)
	{
		delete heightfield;
		heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
		heightfield->createTerrain(200, 200, land_size, land_size, sealevel);
		heightfield->setColourBasedOnHeight();
		heightfield->createObject();
	}

	
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Colourful Noise and Object");;

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

