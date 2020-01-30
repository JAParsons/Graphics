// Minimal fragment shader
// Iain Martin 2018

#version 420 core

in vec4 fcolour;
in vec3 fnormal, flightdir, fposition, fspecular_albedo, fV, fR, femissive;
in vec4 fdiffusecolour, fambientcolour, fdiffuse_albedo;
in flat int fshininess;
out vec4 outputColor;

void main()
{
	vec3 normalisedfnormal = normalize(fnormal);
	vec3 normalisedflightdir = normalize(flightdir);

	// Calculate ambient component
	vec3 ambient = fdiffuse_albedo.xyz * 0.2;

	// Calculate the diffuse component
  	vec3 diffuse = max(dot(normalisedfnormal, normalisedflightdir), 0.0) * fdiffuse_albedo.xyz;

	// Calculate the specular component
	vec3 specular = pow(max(dot(normalize(fR), normalize(fV)), 0.0), fshininess) * fspecular_albedo;

	float distanceToLight = length(flightdir);
	float attenuation_k1 = 0.5;
	float attenuation_k2 = 0.5;
	float attenuation_k3 = 0.5;
	float attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + attenuation_k3 * pow(distanceToLight, 2));

	//outputColor = (fdiffusecolour*diffuse) + fambientcolour + (specular);
	outputColor = vec4(attenuation*(diffuse + ambient + specular) + femissive, 1.0);
	//fcolour = vec4(attenuation*(ambient + diffuse + specular) + emissive + global_ambient, 1.0);
}