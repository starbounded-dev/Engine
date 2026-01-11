#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_UV;

void main()
{
    v_UV = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
