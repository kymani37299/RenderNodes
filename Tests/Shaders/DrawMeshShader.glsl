#version 430

#ifdef VERTEX

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_UV;
layout (location = 2) in vec3 in_Normal;

out vec2 out_UV;
out vec3 out_Normal;

uniform float Angle;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
    gl_Position = vec4(in_Pos * 0.1, 1.0f) * rotationMatrix(vec3(0.0f, 1.0f, 0.0f), Angle);
    out_UV = in_UV;
    out_Normal = in_Normal;
}

#endif // VERTEX

#ifdef FRAGMENT

out vec4 FragColor;

in vec2 out_UV;
in vec3 out_Normal;

uniform sampler2D Albedo;

void main()
{
    FragColor = texture(Albedo, out_UV);
} 

#endif // FRAGMENT