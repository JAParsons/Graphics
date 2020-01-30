#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class TriPrism
{
public:
	TriPrism();
	~TriPrism();

	void makeTriPrism();
	void drawTriPrism(int drawmode);

	// Define vertex buffer object names (e.g as globals)
	GLuint positionBufferObject;
	GLuint colourObject;
	GLuint normalsBufferObject;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numvertices;
	int drawmode;
};
