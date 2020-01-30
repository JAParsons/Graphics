// Basic fragment shader to demosntrate combining texturing with 
// vertex lighting
// Iain Martin 2018

#version 400

in vec4 fcolour;
in vec2 ftexcoord;
out vec4 outputColor;

uniform sampler2D tex1;

void main()
{
	vec4 texcolour = texture(tex1, ftexcoord);
	outputColor = fcolour * texcolour;

	outputColor = texcolour;
}