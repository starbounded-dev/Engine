#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform FrameData
{
    mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform ObjectData
{
    mat4 u_Model;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_UV;

void main()
{
    vec4 world = u_Model * vec4(a_Position, 1.0);
    v_Normal = mat3(u_Model) * a_Normal;
    v_UV = a_TexCoord;
    gl_Position = u_ViewProjection * world;
}
