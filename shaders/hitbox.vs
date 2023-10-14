#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

out vec2 TexCoords;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = aPos;
    TexCoords = tex;  
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}