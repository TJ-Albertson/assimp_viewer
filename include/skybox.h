/*-------------------------------------------------------------------------------\
skybox.h


\-------------------------------------------------------------------------------*/
#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>

#include <iostream>
#include <vector>
#include <string>
//#include <functional>

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
};

unsigned int skyboxVAO;
unsigned int skyboxVBO;
unsigned int skyboxShader;
unsigned int cubemapTexture;
unsigned int cloudTexture;
unsigned int cloudMapTexture;

void LoadSkybox(std::string (*filepath)(std::string path));
void DrawSkybox(Camera camera, glm::mat4 projection);
unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int loadCubemapAlpha(std::vector<std::string> faces);

void LoadSkybox(std::string (*filepath)(std::string path), std::string skybox)
{
    skyboxShader = createShader(filepath("/shaders/skybox.vs"), filepath("/shaders/skybox.fs"));
    
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    stbi_set_flip_vertically_on_load(false);

    std::vector<std::string> faces {
        filepath("/resources/skybox/" + skybox + "/side.jpg"),
        filepath("/resources/skybox/" + skybox + "/side.jpg"),
        filepath("/resources/skybox/" + skybox + "/top.jpg"),
        filepath("/resources/skybox/" + skybox + "/bottom.jpg"),
        filepath("/resources/skybox/" + skybox + "/side.jpg"),
        filepath("/resources/skybox/" + skybox + "/side.jpg"),
    };
    cubemapTexture = loadCubemap(faces);

    
    std::vector<std::string> cloud {
        filepath("/resources/skybox/skybox5/cloud_right.png"),
        filepath("/resources/skybox/skybox5/cloud_left.png"),
        filepath("/resources/skybox/skybox5/cloud_top.png"),
        filepath("/resources/skybox/skybox5/cloud_top.png"),
        filepath("/resources/skybox/skybox5/cloud_front.png"),
        filepath("/resources/skybox/skybox5/cloud_back.png"),
    };

    cloudMapTexture = loadCubemapAlpha(cloud);

    stbi_set_flip_vertically_on_load(true);

    cloudTexture = TextureFromFile("clouds.png", filepath("/resources/textures"));

    glUseProgram(skyboxShader);
    setShaderInt(skyboxShader, "skybox", 0);
    setShaderInt(skyboxShader, "clouds", 1);
    setShaderInt(skyboxShader, "cloudMap", 2);
}

void DrawSkybox(Camera camera, glm::mat4 view, glm::mat4 projection, float currentTime) 
{
    glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
    glUseProgram(skyboxShader);
    view = glm::mat4(glm::mat3(GetViewMatrix(camera))); // remove translation from the view matrix
    setShaderMat4(skyboxShader, "view", view);
    setShaderMat4(skyboxShader, "projection", projection);

    setShaderFloat(skyboxShader, "time", currentTime);
    // skybox cube
    glBindVertexArray(skyboxVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cloudMapTexture);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
}

unsigned int loadCubemapAlpha(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,  0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}  

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}  

#endif