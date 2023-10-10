/*-------------------------------------------------------------------------------\
terrain.h

Functions:


\-------------------------------------------------------------------------------*/
#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_t.h>

//#include <stb_image.h>
#include <model.h>

#include <vector>

const unsigned int NUM_PATCH_PTS = 4;
unsigned int tessHeightMapShader;
unsigned int terrainVAO;
unsigned int rez;
unsigned int texture;

void LoadTerrain(std::string (*filepath)(std::string path), std::string heightmapFile)
{

   const char* heightmap = heightmapFile.c_str();

    GLint maxTessLevel;
    glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
    std::cout << "Max available tess level: " << maxTessLevel << std::endl;

    tessHeightMapShader = CreateShaderT(filepath("/shaders/terrain/8.3.gpuheight.vs"), filepath("/shaders/terrain/8.3.gpuheight.fs"), "nullptr", filepath("/shaders/terrain/8.3.gpuheight.tcs"), filepath("/shaders/terrain/8.3.gpuheight.tes"));

    // usinged int  tessHeightMapShader("8.3.gpuheight.vs", "8.3.gpuheight.fs", nullptr, "8.3.gpuheight.tcs", "8.3.gpuheight.tes"); // if wishing to render as is

    // load and create a texture
    // -------------------------
    
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned short* data = stbi_load_16(heightmap, &width, &height, &nrChannels, STBI_grey);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, data); // Notice GL_UNSIGNED_SHORT

        SetShaderT_Int(tessHeightMapShader, "heightMap", 0);
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices;

    rez = 10;
    for (unsigned i = 0; i <= rez - 1; i++) {
        for (unsigned j = 0; j <= rez - 1; j++) {
            vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v
        }
    }
    std::cout << "Loaded " << rez * rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez * rez * 4 << " vertices in vertex shader" << std::endl;

    // first, configure the cube's VAO (and terrainVBO)
    unsigned int terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    return;
}

void DrawTerrain(glm::mat4 view, glm::mat4 projection)
{
    glUseProgram(tessHeightMapShader);

    SetShaderT_Mat4(tessHeightMapShader, "view", view);
    SetShaderT_Mat4(tessHeightMapShader, "projection", projection);

    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    SetShaderT_Mat4(tessHeightMapShader, "model", model);

    // render the terrain
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(terrainVAO);
    glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS * rez * rez);
}

#endif