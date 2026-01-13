#version 460 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_UV;

float checker(vec2 uv)
{
    vec2 c = floor(uv * 10.0);
    return mod(c.x + c.y, 2.0);
}

void main()
{
    vec3 n = normalize(v_Normal);
    vec3 normalColor = abs(n);

    float c = checker(v_UV);
    vec3 uvTint = mix(vec3(0.15), vec3(1.0), c);

    o_Color = vec4(normalColor * uvTint, 1.0);
}
