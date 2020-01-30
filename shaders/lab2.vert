// Vertex shader with a model transformation uniform

#version 400 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;
out vec4 fcolour;
uniform mat4 model;

void main()
{
	gl_Position = model * position;
	
	fcolour = colour;
	fcolour = position * 2.0 + vec4(0.5, 0.5, 0.5, 1.0);
}