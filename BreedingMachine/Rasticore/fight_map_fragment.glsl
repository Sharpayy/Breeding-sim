#version 450 core
#extension GL_ARB_gpu_shader_int64: enable
#extension GL_ARB_bindless_texture: enable

layout (early_fragment_tests) in;

in vec2 uv;

layout(bindless_sampler, location = 1) uniform sampler2D TEXTURE_HANDLE;
out vec4 fragColor;

uniform vec2 MouseCoord;
uniform vec2 MapDimensions;
uniform vec2 MapTiles;

void main()
{
    vec3 frame_color = vec3(0.0);
    vec3 selec_color = vec3(1.0);
    
    //vec2 uv = gl_FragCoord.xy/MapDimensions.xy;
    
    float tx = MapTiles.x;//32.0;
    float ty = MapTiles.y;//32.0;

    vec2 b_px = vec2(0.0);
    vec2 bd = vec2(b_px.x / (MapDimensions.x / tx), b_px.y / (MapDimensions.y / ty));
    
    vec2 t_id = floor(uv * vec2(tx, ty));
    vec2 t_uv = fract(uv * vec2(tx, ty));
    
    vec2 mt_id = floor((MouseCoord.xy/MapDimensions.xy) * vec2(tx, ty));

    vec2 unit_position = mt_id;
    float unit_distance = 5.1;
    
    float d_up_t = distance(unit_position, t_id);
    float ps = step(unit_distance, d_up_t);
    
    
    float local_uv_border = 1.0 - float(t_uv.x > bd.x && t_uv.x < 1.0 - bd.x)
    * float(t_uv.y > bd.y && t_uv.y < 1.0 - bd.y);
    
    float rs = float(mt_id == t_id);
    
    vec3 col = texture(TEXTURE_HANDLE, uv).xyz * (1.0 - local_uv_border) + frame_color * (local_uv_border);
    col = mix(col, selec_color, rs * 0.5);
    col = mix(col, vec3(0.0), ps * 0.4);
    

    fragColor = vec4(col,1.0);
}