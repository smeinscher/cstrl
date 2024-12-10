#version 460 core

out vec4 frag_color;

in vec2 uv;
in vec4 color;

uniform sampler2D texture0;

const float blur_amount = 1.0;
const float center = 1.1;
const float step_size = 0.004;
const float steps = 3.0;

const float min_offs = (float(steps - 1.0)) / -2.0;
const float max_offs = (float(steps - 1.0)) / 2.0;

void main()
{
    float amount = pow((uv.y * center) * 2.0 - 1.0, 2.0) * blur_amount;
    vec4 blurred = vec4(0.0, 0.0, 0.0, 1.0);

    for (float offs_x = min_offs; offs_x <= max_offs; offs_x++)
    {
        for (float offs_y = min_offs; offs_y <= max_offs; offs_y++)
        {
            vec2 temp_uv = uv;

            temp_uv.x += offs_x * amount * step_size;
            temp_uv.y += offs_y * amount * step_size;

            blurred += texture2D(texture0, uv);
        }
    }

    blurred /= float(steps * steps);

    frag_color = blurred;
}