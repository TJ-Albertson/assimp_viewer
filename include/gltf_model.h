/*-------------------------------------------------------------------------------\
gltf_model.h

load gltf model for opengl
\-------------------------------------------------------------------------------*/
#ifndef GLTF_MODEL_H
#define GLTF_MODEL_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define MAX_BONE_INFLUENCE 4

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "gltf.h" 



void draw_gltf_mesh(unsigned int VAO, Material material, unsigned int numIndices, unsigned int shaderID) 
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.m_BaseColorTextureId);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.m_NormalTextureId);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material.m_MetallicTextureId);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, material.m_RoughnessTextureId);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, material.m_OcclusionTextureId);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(numIndices), GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}





#endif