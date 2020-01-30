/*
 main.cpp
 Assignment 2 (Honours): Show-off Program with an appropriate background 
 Demonstrates a scene on the planet Tatooine (Star Wars)
 Displays generated terrian, textures, firefly particles, complex drone model, water reflecting the sky
 Includes controls to move the light source and rotate the view
 John Parsons December 2019
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
#include <glm/gtc/noise.hpp>


// Include headers for our objects
#include "cube_tex.h"
#include "sphere_tex.h"
#include "tiny_loader_texture.h"
#include "terrain_object.h"
#include "points2.h"

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program, skyboxProgram, particleProgram;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

//buffer for particle quad
//GLuint quad_vbo;

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
//GLfloat speed;				// movement increment
GLfloat scale_factor;

GLfloat hover, y_rot, x_rot, y_drone_inc, x_drone_inc;

GLfloat light_x, light_y, light_z;

GLfloat drone_x, drone_z, drone_rot;

GLuint shademode;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID, tex_location, tex_location2, shademode_location;
GLuint colourmodeID, emitmodeID, attenuationmodeID;

GLuint modelID_skybox, viewID_skybox, projectionID_skybox, normalmatrixID_skybox;

GLuint modelID_particle, viewID_particle, projectionID_particle, normalmatrixID_particle, point_sizeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

/* Global instances of our objects */
Sphere aSphere;
Cube cube(true);
Cube water(true);

TinyObjLoader drone;

//terrain object + params
terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLfloat sealevel = 0;

GLuint texID, texID2, texID3;

GLuint cubemapTexture;

//perlin noise parameters for procedural terrain texture
GLuint width, height, num_octaves;
GLfloat frequency, scale_divisor;
bool periodic;
GLubyte* noise;

//point sprite object and adjustable parameters
points2* point_anim;
GLfloat speed;
GLfloat maxdist;
GLfloat point_size;		//used to adjust point size in vert

GLfloat part_x, part_y, part_z;

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

//load in faces for skybox cubemap
//method obtained from https://learnopengl.com/Advanced-OpenGL/Cubemaps
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

static void printControls()
{
	cout << "############ Tatooine (Star Wars) - John Parsons 160006092 ############" << endl << endl;
	cout << "Controls:" << endl;
	cout << "1 - move firefly source forward" << endl;
	cout << "2 - move firefly source backward" << endl;
	cout << "3 - move firefly source up" << endl;
	cout << "4 - move firefly source down" << endl;
	cout << "5 - move firefly source right" << endl;
	cout << "6 - move firefly source left" << endl;
	cout << "9 - increase firefly size" << endl;
	cout << "0 - decrease firefly size" << endl;
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

//calculate perlin noise
void calculateNoise(GLuint width, GLuint height, GLfloat perlin_freq, GLfloat perlin_scale, GLuint perlin_octaves)
{
	//array to store the noise values 
	//size is the number of vertices * number of octaves 
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
				vec2 p(x * freq, y * freq);
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

			//store the noise value in the noise array
			noise[(row * width + col) * 4] = grey_value;
			noise[(row * width + col) * 4 + 1] = 0;// grey_value;
			noise[(row * width + col) * 4 + 2] = grey_value;
			noise[(row * width + col) * 4 + 3] = 1;
		}
	}
}

//create or update the noise texture, recreates the noise array and then updates the texture buffer
void setNoiseTexture(GLuint width, GLuint height, GLfloat frequency, GLfloat scale_divisor, GLuint num_octaves)
{
	//fill the noise array, specifying width, height, frequency, scale (amplitude), number of octaves
	calculateNoise(width, height, frequency, scale_divisor, num_octaves);

	/* Create and bind the texture */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, noise);
	delete[] noise; //delete this array as data has been copied into texture buffer
}

//initialisation stuff before entering render loop
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	//speed = 0.05f;
	vx = 10; vy = -100, vz = 0;
	light_x = -1.15f; light_y = -15.16f; light_z = 0.85f;
	drone_x = 5; drone_z = 0.6; drone_rot = 45;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0; emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	tex = 0;
	shademode = 1;

	scale_factor = 0.3f;

	hover = 0;
	y_rot, x_rot = 0;
	y_drone_inc = 0.005f;
	x_drone_inc = 0;

	//define image faces of skybox
	vector<std::string> faces
	{
		"..\\..\\images\\mars\\marslike01ft.tga",
		"..\\..\\images\\mars\\marslike01bk.tga",
		"..\\..\\images\\mars\\marslike01up.tga",
		"..\\..\\images\\mars\\marslike01dn.tga",
		"..\\..\\images\\mars\\marslike01rt.tga",
		"..\\..\\images\\mars\\marslike01lf.tga"
	};

	//set particle params
	speed = 0.00001f;
	maxdist = 3.8f;
	point_anim = new points2(500, maxdist, speed);
	point_anim->create();
	point_size = 5;

	glEnable(GL_PROGRAM_POINT_SIZE);

	/* Define the Blending function */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);
	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/*load and create our monkey object*/
	drone.load_obj("..\\..\\obj\\a.obj");

	/* Load and build the vertex and fragment shaders */
	try
	{
		skyboxProgram = glw->LoadShader("..\\..\\shaders\\skybox.vert", "..\\..\\shaders\\skybox.frag");
		program = glw->LoadShader("..\\..\\shaders\\assignment_2.vert", "..\\..\\shaders\\assignment_2.frag");
		particleProgram = glw->LoadShader("..\\..\\shaders\\point_sprites.vert", "..\\..\\shaders\\point_sprites.frag");
	}
	catch (exception & e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	//define uniforms to send to vertex shader 
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

	//uniforms for skybox shaders
	modelID_skybox = glGetUniformLocation(skyboxProgram, "model");
	viewID_skybox = glGetUniformLocation(skyboxProgram, "view");
	projectionID_skybox = glGetUniformLocation(skyboxProgram, "projection");
	tex_location2 = glGetUniformLocation(skyboxProgram, "tex");


	//uniforms for particle shaders
	modelID_particle = glGetUniformLocation(particleProgram, "model");
	viewID_particle = glGetUniformLocation(particleProgram, "view");
	projectionID_particle = glGetUniformLocation(particleProgram, "projection");
	point_sizeID = glGetUniformLocation(particleProgram, "size");

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	cube.makeCube();
	water.makeCube();

	/* Create the heightfield object */
	octaves = 4;
	perlin_scale = 2.f;
	perlin_frequency = 1.f;
	land_size = 2.f;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(200, 200, land_size, land_size);
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();

	/* Define the dimension of the noise texture and create the noise array */
	width = 100;
	height = 100;
	frequency = 4.f;
	scale_divisor = 2.f;
	num_octaves = 2;
	periodic = false;

	//was trying to work with noise textures here
	//calculateNoise(width, height, frequency, scale_divisor, num_octaves);
	glBindTexture(GL_TEXTURE_2D, texID2);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	setNoiseTexture(width, height, frequency, scale_divisor, num_octaves);

	//load images for skybox
	cubemapTexture = loadCubemap(faces);

	/* load an image file using stb_image */
	const char* filename1 = "..\\..\\images\\ground2.jpg";

	if (!load_texture(filename1, texID, true))
	{
		cout << "Fatal error loading texture: " << filename1 << endl;
		exit(0);
	}

	const char* filename2 = "..\\..\\images\\firefly.png";

	if (!load_texture(filename2, texID3, true))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	// Generate Mip Maps 
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	part_x = 0;
	part_y = 0;
	part_z = 0;

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

	/* Enable Blending for the analytic point sprite */
	glEnable(GL_BLEND);

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
		vec3(0, 1, 3), //camera is at (0,0,3), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  //head is up
	);

	// Apply rotations to the view position. This wil get applied to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(30.16f + light_x, 26.61f + light_y, 8.93f + light_z, 1.0f);
	//vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0f);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID, colourmode);
	glUniform1ui(attenuationmodeID, attenuationmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));

	//tex = 0;
	//shademode = 0;
	//glUniform1ui(tex_location, tex); //send tex var to vert shader
	//glUniform1ui(shademode_location, shademode);

	glBindTexture(GL_TEXTURE_2D, texID);
	glFrontFace(GL_CW);

	glUseProgram(program);
	tex = 1;
	glUniform1ui(tex_location, tex); //send tex var to vert shader

	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

	//draw the sun sphere in the lightsource position to visually represent the light source
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(30.16f + light_x, 26.61f + light_y, 8.93f + light_z));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f)); // make sun sphere
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader	
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		//draw lightposition sphere with emit mode
		emitmode = 1;
		glUniform1ui(emitmodeID, emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);
	}
	model.pop();

	//switch shader program
	glUseProgram(skyboxProgram);
	tex = 0;
	glUniform1ui(tex_location2, tex); //send tex var to vert shader
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	//skybox
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0, 0, 0));
		model.top() = scale(model.top(), vec3(100, 100, 100));
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader	
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		glUniformMatrix4fv(modelID_skybox, 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix4fv(viewID_skybox, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID_skybox, 1, GL_FALSE, &projection[0][0]);

		cube.drawCube(drawmode);
	}
	model.pop();


	tex = 1;
	glUniform1ui(tex_location2, tex); //send tex var to shaders

	//water
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(3, -1.9f, -1.2f));
		model.top() = scale(model.top(), vec3(5.0f, 0.01f, 7.0f));
		model.top() = rotate(model.top(), -radians(50.0f), glm::vec3(0, 0, 1));
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader	
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		glUniformMatrix4fv(modelID_skybox, 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix4fv(viewID_skybox, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID_skybox, 1, GL_FALSE, &projection[0][0]);

		water.drawCube(drawmode);
	}
	model.pop();

	glUseProgram(program);

	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	//render model with vertex colours instead of texture
	tex = 0;
	glUniform1ui(tex_location, tex); //send tex var to vert shader

	//draw terrain
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0, -2, 0));
		model.top() = scale(model.top(), vec3(10, 10, 10));
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		heightfield->drawObject(drawmode);
	}
	model.pop();

	//draw terrain 2nd time
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(11, -2.7, -2));
		model.top() = rotate(model.top(), -radians(180.0f), glm::vec3(0, 1, 0));
		model.top() = rotate(model.top(), radians(8.0f), glm::vec3(1, 0, 0));
		model.top() = rotate(model.top(), -radians(5.0f), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(25, 15, 25));
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		heightfield->drawObject(drawmode);
	}
	model.pop();

	tex = 1;
	glUniform1ui(tex_location, tex); //send tex var to vert shaders
	shademode = 0;
	glUniform1ui(shademode_location, shademode); //send tex var shaders

	//draw drone obj
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(drone_x, -1 + hover, drone_z));
		model.top() = rotate(model.top(), radians(drone_rot), glm::vec3(0, 1, 0));
		model.top() = scale(model.top(), vec3(0.2f, 0.2f, 0.2f));
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		drone.drawObject(drawmode);
	}
	model.pop();

	glBindTexture(GL_TEXTURE_2D, texID3);
	glUseProgram(particleProgram);

	//draw particles
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-1.9f + part_x, 1.1f + part_y, 0.5f + part_z));
		model.top() = scale(model.top(), vec3(1.0f, 1.0f, 1.0f));
		model.top() = rotate(model.top(), -radians(180.0f), glm::vec3(1, 0, 0));

		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		glUseProgram(particleProgram);
		glUniformMatrix4fv(modelID_particle, 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix4fv(viewID_particle, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID_particle, 1, GL_FALSE, &projection[0][0]);
		glUniform1f(point_sizeID, point_size);

		point_anim->draw();
		point_anim->animate();
		glUseProgram(program);
	}
	model.pop();

	//update animation vars
	hover += y_drone_inc;
	if (hover >= 0.2 || hover <= 0) { y_drone_inc *= -1; }

	glDisableVertexAttribArray(0);
	glUseProgram(0);
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

	if (key == 'A') light_x -= 1.0;
	if (key == 'D') light_x += 1.0;
	if (key == 'S') light_y -= 1.0;
	if (key == 'W') light_y += 1.0;
	if (key == 'Q') light_z -= 1.0;
	if (key == 'E') light_z += 1.0;
	if (key == 'T') vx -= 1.f;
	if (key == 'G') vx += 1.f;
	if (key == 'F') vy -= 1.f;
	if (key == 'H') vy += 1.f;
	if (key == 'R') vz -= 1.f;
	if (key == 'Y') vz += 1.f;
	//if (key == GLFW_KEY_LEFT) drone_z -= 0.1;
	//if (key == GLFW_KEY_RIGHT) drone_z += 0.1;
	//if (key == GLFW_KEY_UP) drone_x += 0.1;
	//if (key == GLFW_KEY_DOWN) drone_x -= 0.1;
	//if (key == 'Z') drone_rot -= 3.0;
	//if (key == 'X') drone_rot += 3.0;

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

	if (key == '0') point_size -= 1.f;
	if (key == '9') point_size += 1.f;

	if (key == '1') part_x += 0.1f;
	if (key == '2') part_x -= 0.1f;
	if (key == '3') part_y += 0.1f;
	if (key == '4') part_y -= 0.1f;
	if (key == '5') part_z += 0.1f;
	if (key == '6') part_z -= 0.1f;
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

