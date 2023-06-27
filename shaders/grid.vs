#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
    texCoord = position * 0.5 + 0.5;
}