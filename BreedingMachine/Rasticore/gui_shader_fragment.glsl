#version 450 core

#extension GL_ARB_gpu_shader_int64: enable
#extension GL_ARB_bindless_texture: enable

#define GUI_ELEMENT_SLIDER 1

layout (std430, binding = 11) buffer TS
{
	sampler2D textures[];
};

struct _sampler2D
{
	sampler2D sampler;
	uint64_t Reserved;
};

layout (std140, binding = 12) uniform GUI_ELEMENT
{
	vec2 scale_;
	vec2 pos_;
	int gui_element;

	float val;

	float Reserved0;
	float Reserved1;
	
	_sampler2D store_index[8];
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
		vec4 ColorFill = vec4(0.0, 0.5, 0.9, 1.0);
		vec4 ColorFull = vec4(0.2, 0.5, 0.1, 1.0);


		if (fco > val)
		{
			//OutColor = ColorFill;
			OutColor = texture(store_index[0].sampler, ouv);
		}
		else
		{
			//OutColor = ColorFull;
			OutColor = texture(store_index[1].sampler, ouv);
		}

	}
	else
	{
		OutColor = vec4(1.0, 0.66, 0.8, 1.0);

	}
}

