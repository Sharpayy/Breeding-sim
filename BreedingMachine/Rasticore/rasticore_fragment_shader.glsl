#version 450 core
#extension GL_ARB_gpu_shader_int64: enable
#extension GL_ARB_bindless_texture: enable

layout (early_fragment_tests) in;

layout(bindless_sampler, location = 1) uniform sampler2D TEXTURE_HANDLE;
layout(bindless_sampler, location = 2) uniform sampler2D TEXTURE_HANDLE_ATK;
layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};
out vec4 FragColor;

struct EntityApperance
{
	float attackTextureOpacity;
	float blockadeOpacity;
	
	float Reserved[2];
};

layout (std140, binding = 8) uniform uboEntityApperance
{
	EntityApperance[] entityApperance;
};

in vec2 uv;
in flat uint ModelIdx;
uniform int fp;

void main()
{
	if (fp == 1)
		FragColor = texture(TEXTURE_HANDLE, vec2(1.0 - uv.x, uv.y));
	else
		FragColor = texture(TEXTURE_HANDLE, uv);
}

