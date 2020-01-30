/* tetrahedron.h
 Example class to create a tetrahedron object
 Iain Martin November 2018
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Tetrahedron
{
public:
	Tetrahedron();
	~Tetrahedron();

	/* function prototypes */
	void defineTetrahedron();
	void drawTetrahedron(int drawmode);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<GLushort> elements;

	// Define vertex buffer object names (e.g as globals)
	GLuint tetra_buffer_normals;
	GLuint tetra_buffer_vertices;
	GLuint tetra_buffer_colours;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numvertices;
	int drawmode;
};
