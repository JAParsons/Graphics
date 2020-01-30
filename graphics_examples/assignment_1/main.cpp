/*
 main.cpp
 Assignment 1 (Honours): Lights, camera, and lots of action
 Demonstrates a positional light with attenuation with per-fragment lighting (Cook-torrance & Phong shading) coded in the fragment shader.
 Displays a telescope made of a number of primitive shapes. Light source is represented by a textured sphere
 Includes controls to move the light source and rotate the view
 John Parsons November 2019
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

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h" 

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "cube_tex.h"
#include "sphere_tex.h"
#include "cylinder.h"
#include "triangular_prism.h"

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

GLuint tex;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object
GLfloat speed;				// movement increment
GLfloat scale_factor;

GLfloat collapse, collapse_inc, y_rot, x_rot, y_rot_inc, x_rot_inc;

GLfloat light_x, light_y, light_z;

GLuint shademode;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID, tex_location, shademode_location;
GLuint colourmodeID, emitmodeID, attenuationmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

/* Global instances of our objects */
Sphere aSphere;
Cylinder baseCylinder;
Cylinder poleCylinder;
Cylinder scope1Cylinder;
Cylinder scope2Cylinder;
Cylinder scope3Cylinder;
TriPrism base;
Cube bckgrnd(true);
Cube bckgrnd2(true);
Cube bckgrnd3(true);
Cube bckgrnd4(true);
Cube bckgrnd5(true);
Cube bckgrnd6(true);

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

static void printControls()
{
	cout << "############ Telescope - John Parsons 160006092 ############" << endl << endl;
	cout << "Controls:" << endl;
	cout << "Arrow RIGHT - turn telecope to the right" << endl;
	cout << "Arrow LEFT - turn telecope to the left" << endl;
	cout << "Arrow UP - point telecope down" << endl;
	cout << "Arrow DOWN - point telecope up" << endl;
	cout << "L - collapse telescope" << endl;
	cout << "K - extend telescope" << endl;
	cout << "SPACE - stop transformation" << endl;
	cout << "J - toggle shade mode" << endl;
	cout << ", - toggle draw mode" << endl;
	cout << endl;

	cout << "W - move sun up" << endl;
	cout << "S - move sun down" << endl;
	cout << "A - move sun left" << endl;
	cout << "D - move sun right" << endl;
	cout << "Q - move sun back" << endl;
	cout << "E - move sun forwards" << endl;
	cout << endl;

	cout << "T - rotate camera up" << endl;
	cout << "G - rotate camera down" << endl;
	cout << "F - rotate camera left" << endl;
	cout << "H - rotate camera right" << endl;
	cout << "R - rotate camera back" << endl;
	cout << "Y - rotate camera forwards" << endl;
	cout << endl;
}

/*
This function is called before entering the main rendering loop
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.05f;
	vx = 0; vx = 0, vz = 0;
	light_x = -1.15f; light_y = 0.35f; light_z = 0.85f;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0; emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	tex = 0;
	shademode = 1;

	scale_factor = 0.3f;

	collapse, collapse_inc = 0;
	y_rot, x_rot = 0;
	y_rot_inc, x_rot_inc = 0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("..\\..\\shaders\\assignment_1.vert", "..\\..\\shaders\\assignment_1.frag");
	}
	catch (exception & e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	attenuationmodeID = glGetUniformLocation(program, "attenuationmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");
	tex_location = glGetUniformLocation(program, "tex");
	shademode_location = glGetUniformLocation(program, "shademode");

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	baseCylinder.makeCylinder();
	poleCylinder.makeCylinder();

	scope1Cylinder.makeCylinder();
	scope2Cylinder.makeCylinder();
	scope3Cylinder.makeCylinder();

	base.makeTriPrism();

	bckgrnd.makeCube();
	bckgrnd2.makeCube();
	bckgrnd3.makeCube();
	bckgrnd4.makeCube();
	bckgrnd5.makeCube();
	bckgrnd6.makeCube();

	/* load an image file using stb_image */
	const char* filename1 = "..\\..\\images\\2k_sun.jpg";

	if (!load_texture(filename1, texID, true))
	{
		cout << "Fatal error loading texture: " << filename1 << endl;
		exit(0);
	}

	const char* filename2 = "..\\..\\images\\2k_stars_milky_way.jpg";

	if (!load_texture(filename2, texID2, true))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	// Generate Mip Maps 
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	printControls();
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

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(45.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 3), //camera is at (0,0,3), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  //head is up
	);

	// Apply rotations to the view position. This wil get applied to the whole scene
	view = rotate(view, -radians(vx-15), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy-25), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0f);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID, colourmode);
	glUniform1ui(attenuationmodeID, attenuationmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));

	tex = 0;
	glUniform1ui(tex_location, tex); //send tex var to vert shader
	glUniform1ui(shademode_location, shademode);

	glBindTexture(GL_TEXTURE_2D, texID);
	glFrontFace(GL_CW);

	/* Draw the sun sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f)); // make sun sphere
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID, emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);
	}
	model.pop();

	//glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texID2);

	//draw textured background panel
	model.push(model.top());
	{
		// Define the model transformations for the panel
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, -20.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(100.0f, 100.0f, 0.1f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bckgrnd.drawCube(drawmode);
	}
	model.pop();

	//draw textured background panel
	model.push(model.top());
	{
		// Define the model transformations for the panel
		model.top() = rotate(model.top(), -radians(180.0f), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, -20.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(-100.0f, -100.0f, 0.1f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bckgrnd2.drawCube(drawmode);
	}
	model.pop();

	//draw textured background panel
	model.push(model.top());
	{
		// Define the model transformations for the panel
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, -20.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(-100.0f, -100.0f, 0.1f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bckgrnd3.drawCube(drawmode);
	}
	model.pop();

	//draw textured background panel
	model.push(model.top());
	{
		// Define the model transformations for the panel
		model.top() = rotate(model.top(), -radians(270.0f), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, -20.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(-100.0f, -100.0f, 0.1f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bckgrnd4.drawCube(drawmode);
	}
	model.pop();

	//draw textured background panel
	model.push(model.top());
	{
		// Define the model transformations for the panel
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(1, 0, 0)); //rotating in clockwise direction around y-axis
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, -20.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(-100.0f, -100.0f, 0.1f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bckgrnd5.drawCube(drawmode);
	}
	model.pop();

	//draw textured background panel
	model.push(model.top());
	{
		// Define the model transformations for the panel
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(1, 0, 0)); //rotating in clockwise direction around y-axis
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, 20.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(-100.0f, -100.0f, 0.1f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bckgrnd6.drawCube(drawmode);
	}
	model.pop();

	//render model with vertex colours instead of texture
	tex = 1;
	glUniform1ui(tex_location, tex); //send tex var to vert shader

	//draw the pole cylinder
	model.push(model.top());
	{
		// Define the model transformations for the pole
		model.top() = translate(model.top(), vec3(0.0f, -1.3f, 0.0f) * scale_factor);
		model.top() = scale(model.top(), vec3(0.2f, 2.0f, 0.2f) * scale_factor);

		// Send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw pole cylinder*/
		poleCylinder.drawCylinder(drawmode);
	}
	model.pop();

	//draw the base prism
	model.push(model.top());
	{
		//define the model transformations for the prism
		model.top() = translate(model.top(), vec3(0.35f, -2.28f, 0.0f) * scale_factor);
		model.top() = rotate(model.top(), -radians(-45.0f), glm::vec3(0, 1, 0)); //rotate in anti-clockwise direction around y-axis
		model.top() = scale(model.top(), vec3(3.0f, 0.3f, 3.0f) * scale_factor);

		//send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		//recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw prism*/
		base.drawTriPrism(drawmode);
	}
	model.pop();

	//define the global model transformations. Note, we're not modifying the light source position
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale)); //scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x + angle_inc_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y + x_rot), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z + y_rot), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	//draw the 1st scope cylinder
	model.push(model.top());
	{
		//define the model transformations for the 1st scope cylinder
		model.top() = translate(model.top(), vec3(collapse - 2.0f, 0.0f, 0.0f) * scale_factor);
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 0, 1)); //rotate in clockwise direction around x-axis
		model.top() = scale(model.top(), vec3(0.5f, 2.0f, 0.5f) * scale_factor); // scale cylinder

		//send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		//recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cylinder*/
		scope1Cylinder.drawCylinder(drawmode);
	}
	model.pop();

	//draw the 2nd scope cylinder
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.0f, 0.0f, 0.0f) * scale_factor);
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 0, 1)); //rotate in clockwise direction around x-axis
		model.top() = scale(model.top(), vec3(0.45f, 2.0f, 0.45f) * scale_factor);
		
		//send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		//recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cylinder*/
		scope2Cylinder.drawCylinder(drawmode);
	}
	model.pop();

	//draw the 3rd scope cylinder
	model.push(model.top());
	{
		//define the model transformations for the 3rd scope cylinder
		model.top() = translate(model.top(), vec3(2.0f - collapse, y, z) * scale_factor);
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 0, 1)); //rotate in clockwise direction around z-axis
		model.top() = scale(model.top(), vec3(0.4f, 2.0f, 0.4f) * scale_factor);

		//send the model uniform and normal matrix to the currently bound shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		//recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw cylinder*/
		scope3Cylinder.drawCylinder(drawmode);
	}
	model.pop();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;

	x_rot += x_rot_inc;
	y_rot += y_rot_inc;

	//limit telescope tilt range
	if (y_rot < -20.0f) { y_rot = -20.0f; y_rot_inc = 0; }
	if (y_rot > 20.0f) { y_rot = 20.0f; y_rot_inc = 0; }

	//update and limit telescope collapse range
	collapse += collapse_inc*0.1;
	if (collapse < 0.0f) { collapse = 0.0f; collapse_inc = 0; }
	if (collapse > 1.0f) { collapse = 1.0f; collapse_inc = 0; }
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'A') light_x -= speed;
	if (key == 'D') light_x += speed;
	if (key == 'S') light_y -= speed;
	if (key == 'W') light_y += speed;
	if (key == 'Q') light_z -= speed;
	if (key == 'E') light_z += speed;
	if (key == 'T') vx -= 1.f;
	if (key == 'G') vx += 1.f;
	if (key == 'F') vy -= 1.f;
	if (key == 'H') vy += 1.f;
	if (key == 'R') vz -= 1.f;
	if (key == 'Y') vz += 1.f;
	if (key == 'K') collapse_inc -= speed/3;
	if (key == 'L') collapse_inc += speed/3;
	if (key == GLFW_KEY_LEFT) x_rot_inc += speed;
	if (key == GLFW_KEY_RIGHT) x_rot_inc -= speed;
	if (key == GLFW_KEY_UP) y_rot_inc -= speed;
	if (key == GLFW_KEY_DOWN) y_rot_inc += speed;
	if (key == GLFW_KEY_SPACE) { y_rot_inc = 0; x_rot_inc = 0; collapse_inc = 0; }

	if (key == 'J' && action != GLFW_PRESS)
	{
		shademode = !shademode;
		cout << "shademode = " << shademode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024*2, 768*2, "Telescope - John Parsons");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

