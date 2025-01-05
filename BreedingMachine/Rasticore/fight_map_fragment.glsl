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

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec3 frame_color = vec3(0.0);
    vec3 selec_color = vec3(1.0);
    
    //vec2 uv = gl_FragCoord.xy/MapDimensions.xy;
    
    float tx = MapTiles.x;//32.0;
    float ty = MapTiles.y;//32.0;

    vec2 b_px = vec2(1.0);
    vec2 bd = vec2(b_px.x / (MapDimensions.x / tx), b_px.y / (MapDimensions.y / ty));
    
    vec2 t_id = floor(uv * vec2(tx, ty));
    vec2 t_uv = fract(uv * vec2(tx, ty));
    
    vec2 mt_id = floor((MouseCoord.xy/MapDimensions.xy) * vec2(tx, ty));

    vec2 unit_position = mt_id;
    float unit_distance = visn;
    
    float d_up_t = distance(unit_position, t_id);
    float ps = step(unit_distance, d_up_t);
    
    
    float local_uv_border = 1.0 - float(t_uv.x > bd.x && t_uv.x < 1.0 - bd.x)
    * float(t_uv.y > bd.y && t_uv.y < 1.0 - bd.y);
    
    float rs = float(mt_id == t_id);
    
    vec3 col = texture(TEXTURE_HANDLE, uv).xyz * (1.0 - local_uv_border) + frame_color * (local_uv_border);
    col = mix(col, selec_color, rs * 0.5);
    col = mix(col, vec3(0.0), ps * 0.4);
    //col = col + (col + col * pow(distance(uv, (MouseCoord / MapDimensions)), 2.0));
    //col = mix(col, vec3(1.0), pow(distance(uv, (MouseCoord / MapDimensions)), 2.0));
    col = col - (col * min(0.5, -0.2 + (pow(distance(uv, (MouseCoord / MapDimensions)), visn + 0.2))));
    
    fragColor = vec4(col,1.0);
}