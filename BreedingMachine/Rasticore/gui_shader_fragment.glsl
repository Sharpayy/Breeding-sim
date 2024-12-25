#version 450 core

#extension GL_ARB_gpu_shader_int64: enable
#extension GL_ARB_bindless_texture: enable

#define GUI_ELEMENT_SLIDER 1
#define GUI_ELEMENT_BUTTON 2
#define GUI_ELEMENT_WINDOW 3
#define GUI_ELEMENT_IMAGE  4

struct _sampler2D
{
	sampler2D sampler;
	vec2 Reserved;
};

#ifdef ENABLE_GROOMING
layout(bindless_sampler) uniform sampler2D tex_0;
layout(bindless_sampler) uniform sampler2D tex_1;
#endif

layout (std140, binding = 12) uniform GUI_ELEMENT
{
	vec2 scale_;
	vec2 pos_;
	int gui_element;

	float val;

	float z;
	float Reserved1;
	
	_sampler2D store_index[4];
};

uniform mat4 projection_matrix;

in vec2 ouv;
in vec3 npos;
out vec4 OutColor;
in float fco;

void main()
{
#ifndef ENABLE_GROOMING
	sampler2D texture_0 = store_index[0].sampler;
	sampler2D texture_1 = store_index[1].sampler;
#else
	sampler2D texture_0 = tex_0;
	sampler2D texture_1 = tex_1;
#endif

	if (gui_element == GUI_ELEMENT_SLIDER)
	{
		if (fco > val)
		{
			OutColor = texture(texture_0, ouv);
		}
		else
		{
			OutColor = texture(texture_1, ouv);
		}
	}
	else if (gui_element == GUI_ELEMENT_BUTTON)
	{
		vec4 c = texture(texture_0, ouv);
		OutColor = vec4(c.rgb * (1.0 - 0.5 * float(1.0 == step(0.5, val))), c.a);
	}
	else if (gui_element == GUI_ELEMENT_WINDOW)
	{
		OutColor = texture(texture_0, ouv);
	}
	else if (gui_element == GUI_ELEMENT_IMAGE)
	{
		OutColor = texture(texture_0, ouv);
	}
	else
	{
		OutColor = vec4(1.0, 0.66, 0.8, 1.0);
	}
}

