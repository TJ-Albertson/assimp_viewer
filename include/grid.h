#ifndef GRID
#define GRID

#include <glad/glad.h>
#include <GLFW/glfw3.h>

float vertices[] = {
    -10.0f,  10.0f,   // Bottom-left vertex
     10.0f, -10.0f,   // Bottom-right vertex
    -10.0f, -10.0f,

    -10.0f,  10.0f,    // Top-left vertex
     10.0f, -10.0f,   // Bottom-right vertex
     10.0f,  10.0f,    // Top-right vertex
    
};

unsigned int LoadGrid() {

    glm::vec2 translations[5];

    float offset = 0.0f;
    for (int i = 0; i < 5; i++) {
        translations[i] = glm::vec2(offset, 0.0f);
        offset += 25.0f;
    }

    unsigned int  VAO, VBO, instanceVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glGenBuffers(1, &instanceVBO);

    // Bind VAO
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(translations), &translations[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);

    // Unbind VAO
    glBindVertexArray(0);

    return VAO;
}

/*

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
    texCoord = newPosition * 0.5 + 0.5;
}






#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;

void main()
{

    gl_Position = projection * view * vec4(position.x, 0.0, position.y, 1.0);
    texCoord = position * 0.5 + 0.5;
}






*/





#endif // !GRID