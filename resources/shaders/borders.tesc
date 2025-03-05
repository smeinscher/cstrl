#version 460 core
layout(vertices = 16) out;

in vec4 color_tesc[];
out vec4 color_tese[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 96; // Subdivision in U
        gl_TessLevelInner[1] = 96; // Subdivision in V
        gl_TessLevelOuter[0] = 96; // Left edge
        gl_TessLevelOuter[1] = 96; // Bottom edge
        gl_TessLevelOuter[2] = 96; // Right edge
        gl_TessLevelOuter[3] = 96; // Top edge
    }
    color_tese[gl_InvocationID] = color_tesc[gl_InvocationID];
}
