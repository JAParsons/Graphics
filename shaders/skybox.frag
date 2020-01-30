//skybox fragment shader
//John Parsons December 2019

#version 400

in vec4 fcolour;
in vec3 fposition, fnormalmat;

uniform uint tex;
uniform vec3 cameraPos;
uniform samplerCube texsampler;

out vec4 outputColor;
void main()
{
	//if applying regular skybox texture
	if(tex == 0){
		//apply texture
		vec4 texcolour = texture(texsampler, fposition);
		outputColor = texcolour;
	}
	else{
		//calculate skybox reflection for the water
		vec3 I = normalize(fposition - vec3(0, 1, 3));
		vec3 R = reflect(I, normalize(fnormalmat));

		outputColor = vec4(texture(texsampler, R).rgb, 1.0)*2;
	}
}