// Vertex shader to demonstrate normal mapping
// This fragment shader was adapted from the normal mapping
// example in chapter 4 of :
// OpenGL 4.0 Shading language cookbook, second edition 

#version 420

in vec4 fcolour;		// vertex colour
in vec2 ftexcoord;
in vec3 flightdir;		// in tangent space
in vec3 fviewdir;		// in tangent space

layout (binding=0) uniform sampler2D tex1;		// texture
layout (binding=1) uniform sampler2D tex2;		// normal map inside a texture

out vec4 FragColor;		// Output fragment colour

uniform uint colourmode;	// Enables us to cycle through drawing modes to show the textures

// Function to calculate per-vertex lighting
vec3 phongModel( vec3 norm, vec3 diffR ) 
{
	// Calcuate dot product between normal (perturbed by normal map) and light direction in tangent space
    float sDotN = max( dot(flightdir, norm), 0.0 );

	// Calculate per-fragment diffuse reflection in tangent space
    vec3 diffuse = vec3(fcolour) * diffR * sDotN;
	vec3 ambient = diffR * vec3(fcolour) * 0.2 + vec3(fcolour) * 0.1;

	// Calculate per-fragment specular colour 
	// Using a lazy specular hard-coded colour, should really import specular colour and shineness from application!
    vec3 r = reflect( -flightdir, norm );
	    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
	{
        spec = vec3(0.8, 0.5, 0.2) * pow( max( dot(r,fviewdir), 0.0 ), 1.0);
	}

    return ambient + diffuse + spec;
}

void main() 
{
    // Lookup the normal from the normal map 
	vec4 normal_from_map = texture(tex2, ftexcoord);  

	// Convert the extracted normal_from_map to a surface normal
    vec4 normal = 2.0 * normal_from_map - 1.0;
	
	// Extract the texture from the texture map
    vec4 texColor = texture( tex1, ftexcoord );

	// Calculate phong lighting, using the normal from the normal map
	// to perturb the real noraml to simulate bumps and dips in the texture
    vec4 normal_map_lighting = vec4( phongModel(normal.xyz, texColor.rgb), 1.0 );

	// Switch the output colour based on colour mode to cycle through
	// texture, normal map extract and full normal map lighting
	if (colourmode == 0 || colourmode == 1)
		FragColor = normal_map_lighting;
	else if (colourmode == 2)
		FragColor = texColor;
	else if (colourmode == 3)
		FragColor = normal_from_map;
	else if (colourmode == 4)
		FragColor = normal;
}
