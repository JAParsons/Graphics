/**  Point class to use in an example particle animation of an upward particle
with an element of random movement and gravity.
Iain Martin
November 2018
*/
#include "points2.h"
#include "glm/gtc/random.hpp"

using namespace glm;

/* Constructor, set initial parameters*/
points2::points2(GLuint number, GLfloat dist, GLfloat sp)
{
	numpoints = number;
	maxdist = dist;
	speed = sp;
}


points2::~points2()
{
	delete [] colours;
	delete[] vertices;
}

void points2::updateParams(GLfloat dist, GLfloat sp)
{
	maxdist = dist;
	speed = sp;
}


void  points2::create()
{
	vertices = new vec3[numpoints];
	colours = new vec3[numpoints];
	velocity = new vec3[numpoints];

	/* Define random colour and vertical velocity + small random variation */
	for (int i = 0; i < numpoints; i++)
	{
		// create the particle at the initial position
		initpoint(i);
	}

	/* Create the vertex buffer object */
	/* and the vertex buffer positions */
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(vec3), vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &colour_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(vec3), colours, GL_STATIC_DRAW);
}


void points2::draw()
{
	/* Bind  vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Draw our points*/
	glDrawArrays(GL_POINTS, 0, numpoints);
}


void points2::animate()
{
	for (int i = 0; i < numpoints; i++)
	{
		// Shift vertex position by velocity vector
		vertices[i] += velocity[i];

		// Add a small random value to the velocity
		velocity[i] += vec3(ballRand(linearRand(0.f, speed/40.f)));

		// Calculate distance to the origin
		GLfloat dist = length(vertices[i]);

		// If we are too far away then kill the particle by starting at the origin again
		if (dist > (maxdist - linearRand(0.f, 0.5f)))
		{
			// restart thee particle at the initial position
			initpoint(i);
		}
		else
		{
			//add random direction for a firefly effect
			velocity[i].y += linearRand(-0.001f, 0.001f);
			velocity[i].x += linearRand(-0.001f, 0.001f);
			velocity[i].z += linearRand(-0.001f, 0.001f);
		}
	}

	// Update the vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(vec3), vertices, GL_DYNAMIC_DRAW);
}


// Set the initial particle conditions
void points2::initpoint(int i)
{
	vertices[i] = vec3(0);// vec3((linearRand(0.f, speed*4.f), linearRand(0.f, speed * 2), linearRand(0.f, speed*4.f)));
	colours[i] = vec3(linearRand(0.2f, 0.3f), linearRand(0.4f, 0.5f), linearRand(0.8f, 1.0f));
	//velocity[i] = vec3(0, 0.05, 0) + vec3(ballRand(linearRand(0.f, speed)));
	velocity[i] = vec3(0, 0.0005, 0) + vec3(ballRand(linearRand(0.f, speed)));
	velocity[i] = normalize(velocity[i]) / 500.f;
}