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

uniform float visn;

#define MAX_UNITS 16
uniform vec4 units[MAX_UNITS];

void main()
{
    vec3 frame_color = vec3(0.0);
    vec3 selec_color = vec3(1.0);
    
    float tx = MapTiles.x;
    float ty = MapTiles.y;
    vec2 t_id = floor(uv * vec2(tx, ty));
    vec2 t_uv = fract(uv * vec2(tx, ty));
    
    vec2 b_px = vec2(1.0);
    vec2 bd = vec2(b_px.x / (MapDimensions.x / tx), b_px.y / (MapDimensions.y / ty));
        
    vec2 mt_id = floor((MouseCoord.xy/MapDimensions.xy) * vec2(tx, ty));
    float ps = 0.0;
    float fog = 0.0;

    vec4 uc = vec4(0.0);
    float duc = 1.0;
    
    for (int i = 0; i < MAX_UNITS; i++)
    {
        if (units[i].z != 0.0)
        {
            vec2 unit_position = floor(((units[i].xy+vec2(512.0))/MapDimensions.xy) * vec2(tx, ty));
            float unit_distance = units[i].z;
        
            float d_up_t = distance(unit_position, t_id);
            ps = ps + 1.0 - step(unit_distance, d_up_t);
            float norm_dst = distance((units[i].xy+vec2(512.0))/MapDimensions.xy, uv);
            if (duc > norm_dst)
            {
                uc = units[i];
                duc = norm_dst;
            }
        }
    }
    fog = -0.4 + pow(distance((uc.xy+vec2(512.0))/MapDimensions.xy, uv) * 1.5, sqrt(uc.z));
    
    float local_uv_border = 1.0 - float(t_uv.x > bd.x && t_uv.x < 1.0 - bd.x)
    * float(t_uv.y > bd.y && t_uv.y < 1.0 - bd.y);
    
    float rs = float(mt_id == t_id);
    
    vec3 col = texture(TEXTURE_HANDLE, uv).xyz * (1.0 - local_uv_border) + frame_color * (local_uv_border);
    col = mix(col, selec_color, rs * 0.5);
    col = mix(col, vec3(0.0), (1.0 - min(1.0, ps)) * 0.4);
    col = mix(col, vec3(1.0), fog);
    col = mix(col, vec3(0.0, 0.6, 0.2), (1.0 - step(visn, distance(mt_id, t_id))) * 0.15);
    
    fragColor = vec4(col,1.0);
}