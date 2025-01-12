#version 450 core
#extension GL_ARB_gpu_shader_int64: enable
#extension GL_ARB_bindless_texture: enable

layout (early_fragment_tests) in;

layout(bindless_sampler, location = 1) uniform sampler2D TEXTURE_HANDLE;
layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};
out vec4 FragColor;

in vec2 uv;
in flat uint ModelIdx;
uniform int fp;

uniform float opacity;

void main()
{
	if (fp == 1)
		FragColor = texture(TEXTURE_HANDLE, vec2(1.0 - uv.x, uv.y));
	else
		FragColor = texture(TEXTURE_HANDLE, uv);

	FragColor.a = FragColor.a * opacity;
}

