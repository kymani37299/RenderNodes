#version 430

#ifdef VERTEX

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_UV;
layout (location = 2) in vec3 in_Normal;

layout(location = 0) out vec2 out_UV;
layout(location = 1) out vec3 out_Normal;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = vec4(in_Pos * 0.1, 1.0f) * Model * View * Projection;
    out_UV = in_UV;
    out_Normal = in_Normal;
}

#endif // VERTEX

#ifdef FRAGMENT

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 out_UV;
layout(location = 1) in vec3 out_Normal;

layout(binding=0) uniform sampler2D Albedo;

void main()
{
    FragColor = texture(Albedo, out_UV);
} 

#endif // FRAGMENT