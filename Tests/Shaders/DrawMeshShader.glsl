#version 430

#ifdef VERTEX

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_UV;
layout (location = 2) in vec3 in_Normal;

out vec2 out_UV;
out vec3 out_Normal;

uniform mat4 Model;

void main()
{
    gl_Position = vec4(in_Pos * 0.1, 1.0f) * Model;
    out_UV = in_UV;
    out_Normal = in_Normal;
}

#endif // VERTEX

#ifdef FRAGMENT

out vec4 FragColor;

in vec2 out_UV;
in vec3 out_Normal;

layout(binding=0) uniform sampler2D Albedo;

void main()
{
    FragColor = texture(Albedo, out_UV);
} 

#endif // FRAGMENT