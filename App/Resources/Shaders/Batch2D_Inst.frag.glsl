#version 460 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) flat in int v_TexIndex;
layout(location = 3) in float v_Tiling;

layout(location = 0) uniform sampler2D u_Textures[32];

void main()
{
    vec2 uv = v_TexCoord * v_Tiling;
    vec4 tex = texture(u_Textures[v_TexIndex], uv);
    o_Color = tex * v_Color;
}
