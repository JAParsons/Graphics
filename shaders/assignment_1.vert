//Specify minimum OpenGL version
#version 420 core

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoords;

// This is the output vertex colour sent to the rasterizer
out vec4 fcolour, fCookColour;
out vec3 fnormal, flightdir, fposition, fspecular_albedo, fR, fV, femissive;
out vec4 fdiffusecolour, fambientcolour , fdiffuse_albedo;
out flat int fshininess;
out flat int ftex;
out vec2 ftexcoords;

// These are the uniforms that are defined in the application
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, emitmode;
uniform vec4 lightpos;
uniform uint tex;

uniform uint attenuationmode;

// Global constants (for this vertex shader)
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int  shininess = 8;

void main()
{
	ftexcoords = texcoords;

	fspecular_albedo = specular_albedo;
	fshininess = shininess;

	vec3 emissive = vec3(0);				// Create a vec3(0, 0, 0) for our emmissive light
	vec4 position_h = vec4(position, 1.0);	// Convert the (x,y,z) position to homogeneous coords (x,y,z,w)
	vec4 diffuse_albedo;					// This is the vertex colour, used to handle the colourmode change
	vec3 light_pos3 = lightpos.xyz;			

	// Switch the vertex colour based on the colourmode
	if (colourmode == 1)
		diffuse_albedo = vec4(1.0, 1.0, 1.0, 1.0);
	else
		diffuse_albedo = vec4(1.0, 1.0, 1.0, 1.0);

	vec3 ambient = diffuse_albedo.xyz * 0.2;
	fambientcolour = vec4(ambient, 1.0); //ambient light to pass to frag

	fdiffuse_albedo = diffuse_albedo;

	// Define our vectors to calculate diffuse and specular lighting
	mat4 mv_matrix = view * model;		// Calculate the model-view transformation
	vec4 P = mv_matrix * position_h;	// Modify the vertex position (x, y, z, w) by the model-view transformation
	//vec4 
	vec3 N = normalize(normalmatrix * normal);		// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
	fnormal = N; //normal for frag
	vec3 L = light_pos3 - P.xyz;		// Calculate the vector from the light position to the vertex in eye space
	flightdir = L; //light dir for frag
	float distanceToLight = length(L);	// For attenuation
	L = normalize(L);					// Normalise our light vector

	fdiffusecolour = colour;

	// Calculate the specular component using Phong specular reflection
	vec3 V = normalize(-P.xyz);	
	vec3 R = reflect(-L, N);

	fV = V;
	fR = R;

	// If emitmode is 1 then we enable emmissive lighting
	if (emitmode == 1) emissive = vec3(1.0, 1.0, 0.8); 
	femissive = emissive;

	gl_Position = (projection * view * model) * position_h;
}


