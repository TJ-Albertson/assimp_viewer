#ifndef GRID
#define GRID

#include <glad/glad.h>
#include <GLFW/glfw3.h>

float vertices[] = {
    -100.0f, -100.0f,
    -100.0f, 100.0f,
    100.0f, 100.0f,
    100.0f, -100.0f
};

// Define the indices of the two triangles
int indices[] = {
    0, 1, 2,
    0, 2, 3
};

unsigned int LoadGrid() {

    unsigned int  VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Bind VBO and load vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind EBO and load index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);

    return VAO;
}

#endif // !GRID