#version 460 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_UV;

layout(std140, binding = 2) uniform MaterialData
{
    vec4  u_Tint;
    float u_Ambient;
    float _pad0, _pad1, _pad2; // std140 alignment padding
};

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(vec3(0.6, 1.0, 0.3));
    float ndl = max(dot(N, L), 0.0);

    vec3 lit = u_Tint.rgb * (u_Ambient + ndl * (1.0 - u_Ambient));
    o_Color = vec4(lit, u_Tint.a);
}