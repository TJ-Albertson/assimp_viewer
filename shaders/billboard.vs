#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);

    gl_Position /= gl_Position.w;

    gl_Position.xy += pos.xy * vec2(0.25, 0.4);

    TexCoords = tex;
}  