#version 420 core
in vec4 vPosition;	// Input from VBO
out vec4 colour;		// Output from vertex shader
uniform mat4 model;		// Model transformation matrix

void main()
{
   gl_Position = model * vPosition;
}