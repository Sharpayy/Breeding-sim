#version 450 core

#extension GL_ARB_gpu_shader_int64: enable
#extension GL_ARB_bindless_texture: enable

#define GUI_ELEMENT_SLIDER 1
#define GUI_ELEMENT_BUTTON 2
#define GUI_ELEMENT_WINDOW 3

layout (std430, binding = 11) buffer TS
{
	sampler2D textures[];
};

struct _sampler2D
{
	sampler2D sampler;
	vec2 Reserved;
};

layout (std140, binding = 12) uniform GUI_ELEMENT
{
	vec2 scale_;
	vec2 pos_;
	int gui_element;

	float val;

	float Reserved0;
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
	if (gui_element == GUI_ELEMENT_SLIDER)
	{
		if (fco > val)
		{
			OutColor = texture(store_index[0].sampler, ouv);
		}
		else
		{
			OutColor = texture(store_index[1].sampler, ouv);
		}

	}
	else if (gui_element == GUI_ELEMENT_BUTTON)
	{
		vec4 c = texture(store_index[0].sampler, ouv);
		OutColor = vec4(c.rgb * (1.0 - 0.5 * float(1.0 == step(0.5, val))), c.a);
	}
	else if (gui_element == GUI_ELEMENT_WINDOW)
	{
		OutColor = texture(store_index[0].sampler, ouv);
	}
	else
	{
		OutColor = vec4(1.0, 0.66, 0.8, 1.0);

	}
}

