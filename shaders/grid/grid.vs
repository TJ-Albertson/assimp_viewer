#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 offset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;

void main()
{
    vec2 newPosition = vec2(position.x + offset.x, position.y + offset.y);

    gl_Position = projection * view * vec4(position.x + offset.x, 0.0, position.y + offset.y, 1.0);
    texCoord = position;
}
