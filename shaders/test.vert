// Minimal vertex shader
#version 420

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

void main()
{
	vec4 position_h = vec4(position, 1.0);

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
}