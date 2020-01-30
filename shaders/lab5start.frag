// Minimal fragment shader
// for AC41001 Lab 5
// You should modify this fragment shader to apply texture
// appropriately

#version 400

in vec4 fcolour;
in vec2 ftexcoords;
out vec4 outputColor;
uniform sampler2D tex1;

void main()
{
	vec4 texcolour = texture(tex1, ftexcoords);
	outputColor = texcolour;
}