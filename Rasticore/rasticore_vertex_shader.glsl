#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv_c;

out vec2 uv;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

layout(std430, binding = 5) buffer OPV
{
	mat4 matModel[];
};

layout(std430, binding = 6) buffer IDX
{
	uint modelIdx[];
};

uniform float gScalex;
uniform float gScaley;

uniform float gMovex;
uniform float gMovey;

void main()
{
	// * matModel[modelIdx[gl_InstanceID]]
	gl_Position = matProjCamera  * vec4(vec3(pos.x * gScalex + gMovex, pos.y * gScaley + gMovey, pos.z), 1.0);
	uv = uv_c;
}