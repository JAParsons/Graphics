/**
* Cylinder.h
* Example of a cylinder. This was created by David Ogle in 2015 and updated and tidied up
* by Iain Martin in November 2017.
* Provided to the AC41001/AC51008 Graphics class to help debug their own cylinder objects or to
* used in their assignment to provide another object to create models.
*
* The major limitation is that it is hard-coded to use 100 vertices in each rim. I would advise generalising
* this in in functions defineVertices() and makeCylinder() so this will only work if it is defined with a
* definition of 100.
*/

#include "cylinder.h"
const float PI = 3.141592653589f;  /* pi */

#include <iostream>

using namespace glm;
using namespace std;


/**
 * IM: Constrcutor with no parameters which creates a white cylinder
 */
Cylinder::Cylinder() : Cylinder(vec3(1.f, 1.f, 1.f))
{

}


Cylinder::Cylinder(vec3 c) : colour(c)
{
	this->radius = 1.0f;
	this->length = 1.0f;

	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;

	// hard-coded number of vertices around the circle
	// To change this value you will need to generalise the vertex number and offsets in
	// functions defineVertices() and makeCylinder(). It has already been done in drawCylinder().
	this->definition = 100;
	numberOfvertices = definition * 4 + 2; //number of verticies in the cylinder

	enableTexture = false;
}

Cylinder::~Cylinder()
{
}

void Cylinder::makeCylinder()
{
	defineVertices();

	GLuint pindices[406]; //204 //201
	for (int i = 0; i < 101; i++)
	{
		pindices[i] = i;
	}
	pindices[101] = 1;
	for (int i = 102; i < 203; i++)
	{
		pindices[i] = i - 1;
	}
	pindices[203] = 102;

	for (int i = 204; i < 404; i++)
	{
		pindices[i] = i - 2;
	}
	pindices[404] = 202;
	pindices[405] = 203;
	this->isize = (sizeof(pindices) / sizeof(*pindices));
	glGenBuffers(1, &this->cylinderElementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->cylinderElementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize * sizeof(GLuint), pindices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//based on
//https://www.opengl.org/discussion_boards/showthread.php/167115-Creating-cylinder
void Cylinder::defineVertices()
{
	vec3 vertices[402];
	vec3 normals[402];
	vec3 colour[402];

	//number of pVertieces is total points * 3;
	GLfloat halfLength = this->length / 2;

	//define vertex at the center/top of the cylider
	vertices[0] = vec3(0, halfLength, 0);
	normals[0] = vec3(0.0, 1.0, 0.0);
	colour[0] = this->colour;

	//for every point around the circle
	for (int i = 1; i < this->definition + 1; i++)
	{
		GLfloat theta = (2 * PI) / this->definition * i;

		GLfloat x = radius * cos(theta);
		GLfloat y = halfLength;
		GLfloat z = radius * sin(theta);

		vertices[i] = vec3(x, y, z);
		normals[i] = vec3(0.0, 1.0, 0.0);
		colour[i] = this->colour;
	}
	vertices[101] = vec3(0, -halfLength, 0);
	normals[101] = vec3(0.0, -1.0, 0.0);
	colour[101] = this->colour;

	//for every point around the circle
	for (int i = 102; i < (this->definition * 2) + 2; i++)
	{
		GLfloat theta = (2 * PI) / this->definition * (i - 102);

		GLfloat x = radius * cos(theta);
		GLfloat y = -halfLength;
		GLfloat z = radius * sin(theta);

		vertices[i] = vec3(x, y, z);
		normals[i] = vec3(0.0, -1.0, 0.0);
		colour[i] = this->colour;
	}

	//sides				202								402
	int top = 1;
	int bottom = 102;
	for (int i = ((this->definition * 2) + 2); i < numberOfvertices; i += 2)
	{
		vertices[i] = vertices[top];
		normals[i] = vec3(vertices[top].x, 0.0, vertices[top].z);
		colour[i] = this->colour;
		vertices[i + 1] = vertices[bottom];
		normals[i + 1] = vec3(vertices[bottom].x, 0.0, vertices[bottom].z);
		colour[i + 1] = this->colour;
		top++;
		bottom++;
	}

	/* Create the vertex buffer for the cylinder */
	glGenBuffers(1, &this->cylinderBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderBufferObject);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) * numberOfvertices), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->cylinderNormals);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderNormals);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) * numberOfvertices), &normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	glGenBuffers(1, &this->cylinderColours);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderColours);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) * numberOfvertices), &colour[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Cylinder::drawCylinder(int drawmode)
{
	/* Bind the vertes positions */
	glEnableVertexAttribArray(attribute_v_coord);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderBufferObject);
	glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the colours */
	glEnableVertexAttribArray(attribute_v_colours);
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderColours);
	glVertexAttribPointer(attribute_v_colours, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind the normals */
	glEnableVertexAttribArray(attribute_v_normal);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderNormals);
	glVertexAttribPointer(attribute_v_normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, numberOfvertices);
	}
	else
	{
		int numfanvertices = definition + 2;
		int numsidevertices = numfanvertices * 2;
		int side_offset = definition * 2 + 2;
		// Draw the cylinder using filled triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->cylinderElementbuffer);

		// Draw the top lid
		glDrawElements(GL_TRIANGLE_FAN, numfanvertices, GL_UNSIGNED_INT, (GLvoid*)0);

		// Draw the bottom lid
		glDrawElements(GL_TRIANGLE_FAN, numfanvertices, GL_UNSIGNED_INT, (GLvoid*)(numfanvertices * sizeof(GLuint)));

		// Draw the sides
		glDrawElements(GL_TRIANGLE_STRIP, side_offset, GL_UNSIGNED_INT, (GLvoid*)(numsidevertices * sizeof(GLuint)));
	}
}