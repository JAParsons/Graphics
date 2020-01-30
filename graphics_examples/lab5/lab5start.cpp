/*
 Lab5start
 This is an starting project for lab5. The goal for you is to apply texture
 to both the cube and the sphere
 Iain Martin October 2018
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
#pragma comment(lib, "soil.lib")

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h" 

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include "cube_tex.h"
#include "sphere_tex.h"
#include <iostream>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

/* Define buffer object indices */
GLuint positionBufferObject, colourObject, normalsBufferObject;
GLuint sphereBufferObject, sphereNormals, sphereColours, sphereTexCoords;
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader prgoram */
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
Cube aCube(true);
Sphere aSphere;

GLuint texID, texID2;

using namespace std;
using namespace glm;


bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps)
{
	glGenTextures(1, &texID);

	// local image parameters
	int width, height, nrChannels;

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;

		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			   // texID[i] will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}

	stbi_image_free(data);
	return true;
}



/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0;
	numlats = 60;		// Number of latitudes in our sphere
	numlongs = 60;		// Number of longitudes in our sphere

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* create the sphere and cube objects */
	aCube.makeCube();
	aSphere.makeSphere(numlats, numlongs);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("..\\..\\shaders\\lab5start.vert", "..\\..\\shaders\\lab5start.frag");
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

	// Enable face culling. This will cull the back faces of all
	// triangles. Be careful to ensure that triangles are drawn
	// with correct winding.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);



	///* provides unused texture identifier names */
	//glGenTextures(1, &texID);

	///* Binds " texID " as a 2D texture and activates it */
	//glBindTexture(GL_TEXTURE_2D, texID);

	//// Define the type of data stored in the texture object
	//glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, 8, 8);



	/* load an image file using stb_image */
	const char* filename1 = "..\\..\\images\\grass.jpg";

	if (!load_texture(filename1, texID, true))
	{
		cout << "Fatal error loading texture: " << filename1 << endl;
		exit(0);
	}

	const char* filename2 = "..\\..\\images\\earth_no_clouds_8k.jpg";

	if (!load_texture(filename2, texID2, true))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	// Generate Mip Maps 
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	//// Image parameters  
	//int width, height, nrChannels;

	//unsigned char* data = stbi_load("..//..//images//grass.jpg", &width, &height, &nrChannels, 0);

	//// check for an error during the load process  
	//if (data) {

	//	// Note: this is not a full check of all pixel format types, just the most common two!   
	//	int pixel_format = 0;
	//	if (nrChannels == 3)
	//		pixel_format = GL_RGB;
	//	else
	//		pixel_format = GL_RGBA;

	//	// Create the texture, passing in the pointer to the loaded image pixel data   
	//	glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

	//	stbi_image_free(data);
	//}
	//else
	//{
	//	printf("stb_image  loading error:");
	//	exit(0);
	//}

	//int loc = glGetUniformLocation(program, "tex1");
	//if (loc >= 0) glUniform1i(loc, 0);
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

	// Define the model transformations for the cube
	mat4 model = mat4(1.0f);
	model = translate(model, vec3(x+0.5, y, z));
	model = scale(model, vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
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

	glBindTexture(GL_TEXTURE_2D, texID);

	/* Draw our cube*/
	glFrontFace(GL_CW);
	aCube.drawCube(drawmode);

	/* Define the model transformations for our sphere */
	model = mat4(1.0f);
	model = translate(model, vec3(-x-0.5, 0, 0));
	model = scale(model, vec3(model_scale/3.f, model_scale/3.f, model_scale/3.f));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	glBindTexture(GL_TEXTURE_2D, texID2);

	/* Draw our sphere */
	aSphere.drawSphere(drawmode);

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
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab5: Fun with texture");;

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
