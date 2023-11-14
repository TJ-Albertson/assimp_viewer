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


typedef struct Material {
    unsigned int m_BaseColorTextureId;
    unsigned int m_MetallicTextureId;
    unsigned int m_RoughnessTextureId;
    unsigned int m_NormalTextureId;
    unsigned int m_OcclusionTextureId;
    unsigned int m_EmissiveTextureId;

    glm::vec3 m_EmissiveFactor;
    glm::vec4 m_BaseColorFactor;
    
    float m_MetallicFactor;
    float m_RoughnessFactor;
} Material;



unsigned int load_gltf_texture(gltfTexture texture, int type, gltfSampler* gltf_samplers, gltfImage* gltf_images)
{
    gltfImage image = gltf_images[texture.m_SourceIndex];

    char filename[FILENAME_MAX];
    sprintf(filename, "%s/%s", currentDirectory, image.m_URI);

    unsigned int textureID;
    glGenTextures(1, &textureID);
  
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename, &width, &height, &nrComponents, 0);

  
    //metallness value in "blue" color channel
    //roughness value in "green" color channel

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

        // THESE IMPORTANT TO INCREASE TEXTURE QUALITY
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2.0f);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.0f);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        printf("Texture failed to load at path: %s\n", filename);
        stbi_image_free(data);
    }

    return textureID;
}

Material load_gltf_material(gltfMaterial gltf_material, gltfImage* gltf_images, gltfSampler* gltf_samplers, gltfTexture* textures)
{
    Material material;

    if (gltf_material.m_MetalicRoughness != NULL) 
    {
        gltfMetallicRoughness gltf_metal_rough = *gltf_material.m_MetalicRoughness;

        if (gltf_metal_rough.m_BaseColorTexture != NULL)
        {
            gltfTexture base_color_texture = textures[gltf_metal_rough.m_BaseColorTexture->m_Index];
            material.m_BaseColorTextureId = load_gltf_texture(base_color_texture, 0, gltf_samplers, gltf_images);
        } else {
            material.m_BaseColorTextureId = 0;
        }

        if (gltf_metal_rough.m_MetallicRoughnessTexture != NULL)
        {
            gltfTexture metal_rough_texture = textures[gltf_metal_rough.m_MetallicRoughnessTexture->m_Index];
            material.m_MetallicTextureId = load_gltf_texture(metal_rough_texture, 1, gltf_samplers, gltf_images);
            material.m_RoughnessTextureId = load_gltf_texture(metal_rough_texture, 2, gltf_samplers, gltf_images);
        } else {
            material.m_MetallicTextureId = 0;
            material.m_RoughnessTextureId = 0;
        }
        
        material.m_BaseColorFactor = gltf_metal_rough.m_BaseColorFactor;
        material.m_MetallicFactor = gltf_metal_rough.m_MetallicFactor;
        material.m_RoughnessFactor = gltf_metal_rough.m_RoughnessFactor;
    } else {
        printf("No metal roughness\n");
    }

    if (gltf_material.m_NormalTexture != NULL) 
    {
        gltfTexture normal_texture = textures [gltf_material.m_NormalTexture->m_Index];
        material.m_NormalTextureId = load_gltf_texture(normal_texture, 0, gltf_samplers, gltf_images);
    } else {
        material.m_NormalTextureId = 0; // switch to default
    }

    if (gltf_material.m_OcclusionTexture != NULL) 
    {
        gltfTexture occlusion_texture = textures[gltf_material.m_OcclusionTexture->m_Index];
        material.m_OcclusionTextureId = load_gltf_texture(occlusion_texture, 0, gltf_samplers, gltf_images);
    } else {
        material.m_OcclusionTextureId = 0; // switch to default
    }

    if (gltf_material.m_EmissiveTexture != NULL)
    {
        gltfTexture emissive_texture = textures[gltf_material.m_EmissiveTexture->m_Index];
        material.m_EmissiveTextureId = load_gltf_texture(emissive_texture, 0, gltf_samplers, gltf_images);
    } else {
        material.m_EmissiveTextureId = 0; // switch to default
    }

    material.m_EmissiveFactor = gltf_material.m_EmissiveFactor;

    return material;
}

void load_gltf_MESH(gltfMesh* gltf_Mesh)
{

}

unsigned int gltf_LoadMeshVertexData(gltfVertex* vertices, unsigned int* indices, int numVertices, int numIndices)
{
    unsigned int VAO, VBO, EBO;

    // initializes all the buffer objects/arrays
    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(gltfVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers

    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gltfVertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gltfVertex), (void*)offsetof(gltfVertex, m_Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gltfVertex), (void*)offsetof(gltfVertex, m_TexCoord_0));

    glBindVertexArray(0);

    return VAO;
}


void draw_gltf_mesh(unsigned int VAO, Material material, unsigned int numIndices, unsigned int shaderID) 
{


    //setMaterialShaderMat


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.m_BaseColorTextureId);
    glUniform1i(glGetUniformLocation(shaderID, "albedoMap"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.m_NormalTextureId);
    glUniform1i(glGetUniformLocation(shaderID, "normalMap"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material.m_MetallicTextureId);
    glUniform1i(glGetUniformLocation(shaderID, "metallicMap"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, material.m_RoughnessTextureId);
    glUniform1i(glGetUniformLocation(shaderID, "roughnessMap"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, material.m_OcclusionTextureId);
    glUniform1i(glGetUniformLocation(shaderID, "aoMap"), 4);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(numIndices), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}


#endif