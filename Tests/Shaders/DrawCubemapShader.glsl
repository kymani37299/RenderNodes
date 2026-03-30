#version 430

#ifdef VERTEX

layout (location = 0) in vec3 in_Pos;

layout (location = 0) out vec3 out_SkyboxRay;

uniform mat4 View;
uniform mat4 Projection;

void main()
{
    vec4 viewPos = vec4(in_Pos, 0.0f) * View;
    viewPos.w = 1.0;
    gl_Position = viewPos * Projection;

    out_SkyboxRay = in_Pos;
}

#endif // VERTEX

#ifdef FRAGMENT

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 out_SkyboxRay;

void main()
{
    vec3 ray = normalize(out_SkyboxRay);
    float t = 0.5 * (ray.y + 1.0);
    vec3 skyColor = mix(vec3(0.5, 0.7, 1.0), vec3(1.0, 1.0, 1.0), t);
    FragColor = vec4(skyColor, 1.0);
} 

#endif // FRAGMENT