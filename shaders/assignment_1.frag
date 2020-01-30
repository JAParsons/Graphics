//Specify minimum OpenGL version
#version 420 core

in vec4 fcolour;
in vec3 fnormal, flightdir, fposition, fspecular_albedo, fV, fR, femissive;
in vec4 fdiffusecolour, fambientcolour, fdiffuse_albedo;
in flat int fshininess;
in vec2 ftexcoords;

uniform sampler2D tex1;
uniform uint tex;
uniform uint shademode;

out vec4 outputColor;

//functions for calculating Cook-Torrance shading
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

const float PI = 3.14159265359;

// material parameters
vec3  albedo;
float metallic;
float roughness;
float ao;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

void main()
{
	albedo = vec3(0.0f);
	metallic = 0.9f;
	roughness = 0.1f;

	//###################   PHONG SHADING   ########################
	vec3 N = normalize(fnormal);
	vec3 L = normalize(flightdir);

	// Calculate ambient component
	vec3 ambient = fdiffuse_albedo.xyz * 0.2;

	// Calculate the diffuse component
  	vec3 diffuse = max(dot(N, L), 0.0) * fdiffuse_albedo.xyz;

	// Calculate the specular component
	vec3 specular = pow(max(dot(normalize(fR), normalize(fV)), 0.0), fshininess) * fspecular_albedo;

	// Calculate attenuation
	float distanceToLight = length(flightdir);
	float attenuation_k1 = 0.5;
	float attenuation_k2 = 0.5;
	float attenuation_k3 = 0.5;
	float attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + attenuation_k3 * pow(distanceToLight, 2));
	//#############################################################

	//check if the texture flag is set
	if(tex == 0)
	{
		//apply texture
		vec4 texcolour = texture(tex1, ftexcoords);
		outputColor = fcolour + texcolour*2;
		outputColor = texcolour*2 + vec4(femissive*0.1f, 1.0);
	}
	else if(shademode == 1) //cook-torrance shading
	{
		//###############   COOK-TORRANCE SHADING   ###################
		//a modified cook-torrance implementation based on this code: https://learnopengl.com/PBR/Lighting
		vec3 F0 = vec3(0.04); 
		F0 = mix(F0, fdiffuse_albedo.xyz, metallic);

		// reflectance equation
		vec3 Lo = vec3(0.0);

		// calculate light radiance
		vec3 H = normalize(fV + L);
		float cookAttenuation = 1.0 / (distanceToLight * distanceToLight);
		vec3 radiance = fdiffusecolour.xyz * cookAttenuation;
    
		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);        
		float G   = GeometrySmith(N, fV, L, roughness);      
		vec3 F    = fresnelSchlick(max(dot(H, fV), 0.0), F0);       
        
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;	  
        
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, fV), 0.0) * max(dot(N, L), 0.0);
		vec3 cookSpecular = numerator / max(denominator, 0.001);  
            
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);                
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;

		vec3 cookAmbient = vec3(0.03) * albedo * ao;
		vec3 colour = cookAmbient + Lo;
	
		colour = colour / (colour + vec3(1.0));
		colour = pow(colour, vec3(1.0/2.2));
   
		vec4 fragColour = vec4(colour, 1.0);
		//#############################################################

		outputColor = vec4(attenuation*(fragColour.xyz + ambient + cookSpecular) + femissive*0.2f, 1.0); //altered default cook-torrance with some of my phong components
	}
	else if(shademode == 0) //phong shading
	{
		outputColor = vec4(attenuation*(diffuse + ambient + specular) + femissive*0.2f, 1.0);
	}
}