/**
 * Cylinder.h
 * Example of a cylinder. This was created by David Ogle in 2015 and updated and tidied up
 * by Iain Martin in November 2017.
 * Provided to the AC41001/AC51008 Graphics class to help debug their own cylinder objects or to
 * used in their assignment to provide another flexible
 */

#ifndef CYLINDER_H
#define CYLINDER_H

#include "wrapper_glfw.h"
#include <glm/glm.hpp>

class Cylinder
{
private:
	glm::vec3 colour;
	GLfloat radius, length;
	GLuint definition;
	GLuint cylinderBufferObject, cylinderNormals, cylinderColours, cylinderElementbuffer;
	GLuint num_pvertices;
	GLuint isize;
	GLuint numberOfvertices;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	bool enableTexture;

	void defineVertices();

public:
	Cylinder();
	Cylinder(glm::vec3 c);
	~Cylinder();
	void makeCylinder();
	void drawCylinder(int drawmode);
};

#endif