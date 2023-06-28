#ifndef GRID_H
#define GRID_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>

float vertices[] = {
    -10.0f, 10.0f, // Bottom-left vertex
    10.0f, -10.0f, // Bottom-right vertex
    -10.0f, -10.0f,

    -10.0f, 10.0f, // Top-left vertex
    10.0f, -10.0f, // Bottom-right vertex
    10.0f, 10.0f, // Top-right vertex
};

unsigned int LoadGrid()
{
    const int numRows = 5;
    const int numCols = 5;
    glm::vec2 translations[numRows * numCols * 4];

    int index = 0;
    for (int row = -5; row < numRows; row++) {
        for (int col = -5; col < numCols; col++) {
            translations[index] = glm::vec2(row * 10.0f, col * 10.0f);
            index++;
        }
    }

    // std::cout << "index: " << index << std::endl;

    unsigned int VAO, VBO, instanceVBO;
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

#endif // !GRID