/*
 normalmap
 Basic example to show normal mapping
 The fragment shader was adapted from the normal map example in chapter 4 of the OpenGL Shading Language cookbook
 The texture and bump map are from: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
 Uses texture and normal map and stb_image.h to load the texture and normal map
 Iain Martin November 2018
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

/* Include the stb image loader */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;
using namespace std;

/* Define buffer object indices */
GLuint quad_vbo, quad_normals, quad_colours, quad_tex_coords;

/* Define textureID*/
GLuint texID1, texID2;

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					   I've included this to show you how to pass in an unsigned integer into
					   your vertex shader. */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

// Mipmap mode variable
GLuint mipmap_min_mode = 0;
string colourmode_strings[] = { "normal_map_lighting", "normal_map_lighting_with_colour", "texture", "normal_from_map", "normal" };


/**
 * Uses stb_image.h to load and image and create a texture from it
 */
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
	x = 0;
	y = 0;
	z = 0;
	angle_y = angle_z = 0;
	angle_x = -20.f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 2;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("../../shaders/normalmap.vert", "../../shaders/normalmap.frag");
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

	// Create data for our quad with vertices, normals and texturee coordinates 
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
		1.0f, 0.0f, 0,
		1.0f, 1.0f, 0,
		0.0f, 1.0f, 0,

		// Tangent vectors
		1.f, 0, 0,
		1.f, 0, 0,
		1.f, 0, 0,
		1.f, 0, 0
	};

	// Copy the data into the buffer. See how this example combines the vertices, normals and texture
	// coordinates in the same buffer and uses the last parameter of  glVertexAttribPointer() to
	// specify the byte offset into the buffer for each vertex attribute set.
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)(36 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	/* Activate GL_TEXTURE0 (not really necessary as this is the default) */
	glActiveTexture(GL_TEXTURE0);

	// Load the texture object, obtaining a texture ID for the 2D texture object
//	if (!load_texture("..\\..\\images\\grass01.png", texID1, true))
	if (!load_texture("..\\..\\images\\diffuse.png", texID1, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	/* Standard bit of code to enable a uniform sampler for our texture */
	int loc1 = glGetUniformLocation(program, "tex1");
	if (loc1 >= 0) glUniform1i(loc1, 1);
	
	/* Define the texture behaviour parameters using Mipmaps */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	/* Create the second texture object, the normal map */

	/* Activate GL_TEXTURE1 (the second texture object) */
	glActiveTexture(GL_TEXTURE1);

	// Load the texture object, obtaining a texture ID for the 2D texture object
//	if (!load_texture("..\\..\\images\\grass01_n.png", texID1, true))
	if (!load_texture("..\\..\\images\\normal.png", texID1, true))
	{
		cout << "Fatal error loading normal map texture: " << endl;
		exit(0);
	}

	/* Standard bit of code to enable a uniform sampler for our texture */
	int loc2 = glGetUniformLocation(program, "tex2");
	if (loc2 >= 0) glUniform1i(loc2, 1);
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
	model = scale(model, vec3(scaler*5, scaler*5, scaler*5));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 Projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 View = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &Projection[0][0]);

	/* Draw our textured quad*/
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
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;

	/* Use this colour mode variable in the fragment shader to cycle through 
	   showing the texture, normal map and the normal mapped lighting */
	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode ++;
		if (colourmode > 4) colourmode = 0;
		cout << "colourmode = " << colourmode_strings[colourmode] << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

	/* Cycle different texture filtering minimise modes */
	if (key == 'F' && action != GLFW_PRESS)
	{
		mipmap_min_mode++;
		if (mipmap_min_mode > 5) mipmap_min_mode = 0;
		if (mipmap_min_mode == 0)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			std::cout << "Mipmaps MIN_FILTER GL_LINEAR" << std::endl;
		}
		else if (mipmap_min_mode == 1)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			std::cout << "Mipmaps MIN_FILTER GL_NEAREST" << std::endl;
		}
		else if (mipmap_min_mode == 2)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			std::cout << "Mipmaps MIN_FILTER GL_NEAREST_MIPMAP_NEAREST" << std::endl;
		}
		else if (mipmap_min_mode == 3)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			std::cout << "Mipmaps MIN_FILTER GL_LINEAR_MIPMAP_NEAREST" << std::endl;
		}
		else if (mipmap_min_mode == 4)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			std::cout << "Mipmaps MIN_FILTER GL_NEAREST_MIPMAP_LINEAR" << std::endl;
		}
		else if (mipmap_min_mode == 5)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			std::cout << "Mipmaps MIN_FILTER GL_LINEAR_MIPMAP_LINEAR" << std::endl;
		}
	}
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Multi-Texture example");;

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


