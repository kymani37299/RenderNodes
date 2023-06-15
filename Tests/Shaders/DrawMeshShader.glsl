#start VERTEX

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_UV;
layout (location = 2) in vec3 in_Normal;

out vec2 out_UV;
out vec3 out_Normal;

void main()
{
    gl_Position = vec4(0.1 * in_Pos, 1.0);
    out_UV = in_UV;
    out_Normal = in_Normal;
}

#start FRAGMENT

out vec4 FragColor;

in vec2 out_UV;
in vec3 out_Normal;

uniform sampler2D Albedo;

void main()
{
    FragColor = texture(Albedo, out_UV);
} 