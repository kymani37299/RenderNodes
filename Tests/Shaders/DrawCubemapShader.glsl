#version 430

#ifdef VERTEX

layout (location = 0) in vec3 in_Pos;

out vec3 out_SkyboxRay;

uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = vec4(in_Pos, 0.0f) * View * Projection;
    out_SkyboxRay = in_Pos;
}

#endif // VERTEX

#ifdef FRAGMENT

out vec4 FragColor;

out vec3 out_SkyboxRay;

layout(binding=0) uniform sampler2D Skybox;

vec2 SprericalToUV(vec3 coords)
{
    const vec2 invAtan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(coords.x, coords.z), asin(coords.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    FragColor = texture(Skybox, SprericalToUV(out_SkyboxRay));
} 

#endif // FRAGMENT