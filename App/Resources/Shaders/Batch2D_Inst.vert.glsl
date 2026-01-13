#version 460 core

layout(std140, binding = 0) uniform FrameData
{
    mat4 u_ViewProjection;
};

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

// Instance data (must match your VAO setup)
layout(location = 2) in mat4 i_Transform;   // 2,3,4,5
layout(location = 6) in vec4 i_Color;
layout(location = 7) in vec4 i_UVMinMax;    // (u0,v0,u1,v1)
layout(location = 8) in vec2 i_Tex;         // (texIndex, tiling)

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) flat out int v_TexIndex;
layout(location = 3) out float v_Tiling;

void main()
{
    vec2 uv = mix(i_UVMinMax.xy, i_UVMinMax.zw, a_TexCoord);

    v_Color    = i_Color;
    v_TexCoord = uv;
    v_TexIndex = int(i_Tex.x);
    v_Tiling   = i_Tex.y;

    gl_Position = u_ViewProjection * (i_Transform * vec4(a_Position, 1.0));
}
