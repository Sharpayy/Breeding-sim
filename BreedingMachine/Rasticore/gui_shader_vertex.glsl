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

out vec2 ouv;
out vec3 npos;
out float fco;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in float fc;

void main()
{
	
	float z_comp = 5.0 - 2.0 * float(gui_element == GUI_ELEMENT_WINDOW);

	vec3 p = vec3(pos.x * scale_.x + pos_.x, pos.y * scale_.y + pos_.y, pos.z + z);
	gl_Position = projection_matrix * vec4(p, 1.0);
	ouv = uv;
	npos = p;
	fco = fc;
}
