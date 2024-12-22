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

void main()
{
    vec2 iResolution = vec2(1024.0);
    vec2 iMouse = vec2(2.0);
	vec3 frame_color = vec3(0.0);
    vec3 selec_color = vec3(1.0);
    
    float tx = 5.0;
    float ty = 5.0;

    vec2 b_px = vec2(3.1);
    vec2 bd = vec2(b_px.x / (iResolution.x / tx), b_px.y / (iResolution.y / ty));
    
    vec2 t_id = floor(uv * vec2(tx, ty));
    vec2 t_uv = fract(uv * vec2(tx, ty));
    
    vec2 mt_id = floor((iMouse.xy/iResolution.xy) * vec2(tx, ty));
    
    vec2 unit_position = mt_id;
    float unit_distance = 2.1;
    
    float d_up_t = distance(unit_position, t_id);
    float ps = step(unit_distance, d_up_t);
    
    
    float local_uv_border = 1.0 - float(t_uv.x > bd.x && t_uv.x < 1.0 - bd.x)
    * float(t_uv.y > bd.y && t_uv.y < 1.0 - bd.y);
    
    float rs = float(mt_id == t_id);
    
    vec3 col = texture(TEXTURE_HANDLE, uv).xyz * (1.0 - local_uv_border) + frame_color * (local_uv_border);
    col = mix(col, selec_color, rs * 0.5);
    col = mix(col, vec3(0.0), ps * 0.4);

    FragColor = vec4(col, 1.0);
}

