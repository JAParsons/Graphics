// Minimal fragment shader

#version 400 core

in vec4 fcolour;
out vec4 outputColor;
void main()
{
	outputColor = fcolour;
}