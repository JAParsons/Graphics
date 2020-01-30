// Vertex shader with vertex colour attribute

#version 420
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;
out vec4 fcolour;

void main()
{
	gl_Position = position;
	fcolour = colour;
}