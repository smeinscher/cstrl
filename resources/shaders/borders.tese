#version 460 core

layout(quads, fractional_even_spacing) in;

uniform mat4 view;
uniform mat4 projection;

in vec4 color_tese[];
out vec4 color;
out vec2 frag_tex_coord;

float bernstein(int i, float x)
{
    float invx = 1.0 - x;

    if (i == 0)
    {
        return invx * invx * invx;
    }
    if (i == 1)
    {
        return 3 * x * invx * invx;
    }
    if (i == 2)
    {
        return 3 * x * x * invx;
    }
    return x * x * x;
}

vec3 bezier_patch(float u, float v)
{
    int i, j;
    vec3 res = vec3(0.0, 0.0, 0.0);

    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 4; i++)
        {
            float bu = bernstein(i, u);
            float bv = bernstein(j, v);

            res += gl_in[j * 4 + i].gl_Position.xyz * bu * bv;
        }
    }
    return res;
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    vec3 position = normalize(bezier_patch(u, v));
    gl_Position = projection * view * vec4(position, 1.0);
    color = mix(mix(color_tese[0], color_tese[1], u), mix(color_tese[2], color_tese[3], u), v);

    float uc = gl_TessCoord.x * 2.0 - 1.0;
    float vc = gl_TessCoord.y * 2.0 - 1.0;
    float wc = gl_TessCoord.z * 2.0 - 1.0;
    float r = length(vec3(uc, vc, wc));
    vec3 circle_position = normalize(vec3(uc, vc, wc)) * min(1.0, r);

    frag_tex_coord = (circle_position.xy + 1.0) * 0.5;
}
