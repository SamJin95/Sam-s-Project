#version 430 //GLSL version your computer supports
/************************
SID: 1155046896 & 1155091693
Name :Jin Xiamu & Cheng Brian Wing Hang
**************************/
in vec2 UV;
in vec3 normalsky;
in vec3 VertexLocation;
uniform sampler2D myTextureSampler;
uniform vec4 ambiLight;
uniform vec4 difLight;
uniform vec4 speLight;
uniform vec3 LightLocation;
uniform vec4 LookLocation;
out vec4 thecolor;
void main()
{
	vec3 LightV = normalize(LightLocation - VertexLocation);
	vec4 diffLight = vec4(dot(LightV, normalize(normalsky)), dot(LightV, normalize(normalsky)), dot(LightV, normalize(normalsky)), 1.0) * difLight;// * 1/(dot((LightLocation - VertexLocation),(LightLocation - VertexLocation))) * 2;
	vec3 refLight = reflect(-LightV, normalsky);
	float hp = clamp(dot(refLight, normalize(LookLocation.xyz - VertexLocation)),0,1) ;
	float hp1 = pow(hp, 20);
	vec4 specLight = vec4(hp1,hp1,hp1,1) * speLight;

	thecolor = vec4(texture(myTextureSampler, UV ).rgb, 1.0) * ambiLight +  clamp(vec4(texture(myTextureSampler, UV ).rgb, 1.0)* diffLight,0,1) + vec4(0.6,0.8,0.7, 1.0) * specLight;
}