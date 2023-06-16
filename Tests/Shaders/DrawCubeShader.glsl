#version 430

#ifdef VERTEX

layout (location = 0) in vec3 aPos;
  
out vec4 vertexColor;

void main()
{
    gl_Position = vec4(0.5 * aPos, 1.0);
    vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}

#endif // VERTEX

#ifdef FRAGMENT

out vec4 FragColor;
  
in vec4 vertexColor; 

void main()
{
    FragColor = vertexColor;
} 

#endif // FRAGMENT