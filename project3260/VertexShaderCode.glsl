#version 430//GLSL version your computer supports
/************************
SID: 1155046896 & 1155091693
Name :Jin Xiamu & Cheng Brian Wing Hang
**************************/

in layout(location=0) vec3 pos;
in layout(location=1) vec2 vUV;
in layout(location=2) vec3 vnormal;

uniform mat4 modelTransformMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;
out vec3 normalsky;
out vec3 VertexLocation;

void main()
{	
	gl_Position = projectionMatrix * viewMatrix  * modelTransformMatrix * vec4(pos, 1.0);	
	normalsky = (modelTransformMatrix * vec4 (vnormal, 0)).xyz;
	VertexLocation = (modelTransformMatrix * vec4(pos, 1.0)).xyz;
	UV = vUV;
}

