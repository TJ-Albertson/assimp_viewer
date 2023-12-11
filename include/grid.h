#ifndef GRID_H
#define GRID_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>

unsigned int grid_texture_id;
unsigned int grid_VAO;

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

/* Example */
/*

unsigned int grid_VAO = LoadGrid();
unsigned int gridShader = createShader(filepath("/shaders/grid.vs"), filepath("/shaders/grid.fs"));

// Draw Grid, with instance array
        glUseProgram(gridShader);
        setShaderMat4(gridShader, "projection", projection);
        setShaderMat4(gridShader, "view", view);
        glBindVertexArray(grid_VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
        glBindVertexArray(0);
*/

void load_textured_grid(std::string grid_texture)
{
    const char* filename = grid_texture.c_str();

    float vertices[] = {
        // Position         // Texture Coords
        0.5f, 0.0f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.0f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f,
        -0.5f, 0.0f, 0.5f, 0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3 // second triangle
    };



    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    grid_VAO = VAO;


    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // THESE IMPORTANt TO INCREASE TEXTURE QUALITY
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2.0f);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.0f);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << grid_texture << std::endl;
        stbi_image_free(data);
    }

    grid_texture_id = textureID;
}


void draw_textured_grid(unsigned int shaderId)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grid_texture_id);

    glUseProgram(shaderId);
    glBindVertexArray(grid_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

#endif // !GRID