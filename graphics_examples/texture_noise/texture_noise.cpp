/*
 Texture_noise
 Basic example to show how to texture a polygon using noise.
 Addapted from OpenGL 4 shading Language Cookbook, chapter 8 
 Iain Martin November 2018
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

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

/* Include the hacked version of SOIL */
#include "SOIL.h"

using namespace std;
using namespace glm;

/* Define buffer object indices */
GLuint quad_vbo, quad_normals, quad_colours, quad_tex_coords;

/* Define textureID*/
GLuint texID;

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

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

/* Perlin noise parameters */
GLuint width, height, num_octaves;
GLfloat frequency, scale_divisor;
bool periodic;
GLubyte* noise; 

void calculateNoise(GLuint width, GLuint height, GLfloat perlin_freq, GLfloat perlin_scale, GLuint perlin_octaves)
{
	// Create the array to store the noise values 
	// The size is the number of vertices * number of octaves 
	noise = new GLubyte[width * height * 4];

	GLfloat xfactor = 1.f / (width - 1);
	GLfloat yfactor = 1.f / (height - 1);

	for (uint row = 0; row < height; row++)
	{
		for (uint col = 0; col < width; col++)
		{
			GLfloat x = xfactor * col;
			GLfloat y = yfactor * row;
			GLfloat sum = 0;
			GLfloat freq = perlin_freq;
			GLfloat scale = perlin_scale;

			// Compute the sum for each octave
			for (uint oct = 0; oct < perlin_octaves; oct++)
			{
				vec2 p(x*freq, y*freq);
				GLfloat val = 0.0f;
				if (periodic)
					val = perlin(p, vec2(freq)) / scale;
				else
					val = perlin(p) / scale;

				sum += val;

				// Move to the next frequency and scale
				freq *= 2.f;
				scale *= perlin_scale;
			}
				
			GLfloat result = (sum + 1.f) / 2.0f;
			GLubyte grey_value = (GLubyte)(result * 255.f);

			// Store the noise value in our noise array
			noise[(row * width + col) * 4] = grey_value;
			noise[(row * width + col) * 4 + 1] = 0;// grey_value;
			noise[(row * width + col) * 4 + 2] = grey_value;
			noise[(row * width + col) * 4 + 3] = 1;
		}
	}
}

/* Create or update the noise texture, recreates the noise array and then updates the texture buffer */
void setNoiseTexture(GLuint width, GLuint height, GLfloat frequency, GLfloat scale_divisor, GLuint num_octaves)
{
	/* Fill the noise array, specifying width, height, frequency, scale (amplitude), number of octaves */
	calculateNoise(width, height, frequency, scale_divisor, num_octaves);

	/* Create and bind the texture */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, noise);
	delete[] noise; /* can delete this array now as data has been copied into texture buffer */
}

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
	angle_y = angle_z = 0;
	angle_x = -20.f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("../../shaders/texture_noise.vert", "../../shaders/texture_noise.frag");
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

	/* Create our quad and texture */
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	// Create dat for our quad with vertices, normals and texture coordinates 
	static const GLfloat quad_data[] =
	{
		// Vertex positions
		0.75f, 0, -0.75f,
		-0.75f, 0, -0.75f,
		-0.75f, 0, 0.75f,
		0.75f, 0, 0.75f,

		// Normals
		0, 1.f, 0,
		0, 1.f, 0,
		0, 1.f, 0,
		0, 1.f, 0,

		// Texture coordinates. Note we only need two per vertex but have a
		// redundant third to fit the texture coords in the same buffer for this simple object
		0.0f, 0.0f, 0,
		2.0f, 0.0f, 0,
		2.0f, 2.0f, 0,
		0.0f, 2.0f, 0,
	};

	// Copy the data into the buffer. See how this example combines the vertices, normals and texture
	// coordinates in the same buffer and uses the last parameter of  glVertexAttribPointer() to
	// specify the byte offset into the buffer for each vertex attribute set.
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	/* Create the noise texture object */
	glGenTextures(1, &texID);

	/* Not actually needed if using one texture at a time */
	glActiveTexture(GL_TEXTURE0);

	/* Define the dimension of the noise texture and create the noise array */
	width = 100;
	height = 100;
	frequency = 4.f;
	scale_divisor = 2.f;
	num_octaves = 2;
	periodic = false;

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

	setNoiseTexture(width, height, frequency, scale_divisor, num_octaves);
	
	/* Standard bit of code to enable a uniform sampler for our texture */
	int loc = glGetUniformLocation(program, "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

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

	// Define the model transformations 
	mat4 model = mat4(1.0f);
	model = translate(model, vec3(x, y, z));
	model = scale(model, vec3(model_scale*5, model_scale*5, model_scale*5));//scale equally in all axis
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

	/* Draw our textured quad*/
	glBindTexture(GL_TEXTURE_2D, texID);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

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
	
	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

	/* Increase or decrease perlin noise number of octaves */
	if ((key == 'O' || key == 'P') && action != GLFW_PRESS)
	{
		if (key == 'O' && num_octaves > 1) num_octaves--;
		if (key == 'P' && num_octaves < 15) num_octaves++;
		printf("\nnum_octaves = %d", num_octaves);
		setNoiseTexture(width, height, frequency, scale_divisor, num_octaves);
	}

	/* Increase or decrease perlin noise frequency */
	if ((key == 'L' || key == ';') && action != GLFW_PRESS)
	{
		if (key == 'L') frequency -= 0.5;
		if (key == ';') frequency += 0.5;
		printf("\nfrequency = %f", frequency);
		setNoiseTexture(width, height, frequency, scale_divisor, num_octaves);
	}

	/* Increase or decrease perlin noise scale divisor  */
	if ((key == ',' || key == '.') && action != GLFW_PRESS)
	{
		if (key == ',') scale_divisor -= 0.5;
		if (key == '.') scale_divisor += 0.5;
		printf("\nscale = %f", scale_divisor);
		setNoiseTexture(width, height, frequency, scale_divisor, num_octaves);
	}

	/* Switch periodic */
	if (key == 'B' && action != GLFW_PRESS)
	{
		periodic = !periodic;
		if (periodic) printf("\nPeriodic noise"); else printf("\nNot periodic noise");
		setNoiseTexture(width, height, frequency, scale_divisor, num_octaves);
	}
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Texture image example");;

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

