//basic vertex shader for skybox
//John Parsons December 2019

#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;

//uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

//output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
out vec3 fposition, fnormalmat;

void main()
{
	//calculate out variables
	fposition = vec3(model * vec4(position, 1.0));
	fnormalmat = mat3(transpose(inverse(model))) * normal;

	vec4 position_h = vec4(position, 1.0);

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
}

