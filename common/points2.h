/**  Point class to use in an example particle animation of an upward particle
with an element of random movement and gravity.
Iain Martin
October 2016
*/
#pragma once

#include <glm/glm.hpp>
#include "wrapper_glfw.h"

class points2
{
public:
	points2(GLuint number, GLfloat dist, GLfloat sp);
	~points2();

	void create();
	void draw();
	void animate();
	void updateParams(GLfloat dist, GLfloat sp);
	void initpoint(int i);

	glm::vec3 *vertices;
	glm::vec3 *colours;
	glm::vec3 *velocity;

	GLuint numpoints;		// Number of particles
	GLuint vertex_buffer;
	GLuint colour_buffer;

	// Particle speed
	GLfloat speed;		

	// Particle max distance fomr the origin before we change direction back to the centre
	GLfloat maxdist;	
};

