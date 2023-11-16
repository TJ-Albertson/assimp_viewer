/*-------------------------------------------------------------------------------\
gltf_gl.h

opengl VA/texture binding functions
\-------------------------------------------------------------------------------*/
#ifndef GLTF_GL_H
#define GLTF_GL_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "gltf_structures.h"

// returns size of attribute based on accessor.type
int gltf_get_size(const char* type)
{
    if (strcmp("SCALAR", type) == 0) {
        return 1;
    }
    if (strcmp("VEC2", type) == 0) {
        return 2;
    }
    if (strcmp("VEC3", type) == 0) {
        return 3;
    }
    if (strcmp("VEC4", type) == 0) {
        return 4;
    }
    if (strcmp("MAT2", type) == 0) {
        return 4;
    }
    if (strcmp("MAT3", type) == 0) {
        return 9;
    }
    if (strcmp("MAT4", type) == 0) {
        return 16;
    }
}

void gltf_bind_attribute(PrimitiveAttribute attribute, gltfAccessor accessor, gltfBufferView* gltfBufferViews, char** allocatedBuffers)
{
    gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

    int bufferIndex = bufferView.m_BufferIndex;

    int offset = 0;

    if (bufferView.m_ByteOffset > 0) {
        offset += bufferView.m_ByteOffset;
    }

    if (accessor.m_ByteOffset > 0) {
        offset += accessor.m_ByteOffset;
    }

    char* offsetBuffer = allocatedBuffers[bufferIndex] + offset;

    int size = gltf_get_size(accessor.m_Type);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferView.m_ByteLength, &offsetBuffer[0], GL_STATIC_DRAW);

    glVertexAttribPointer(attribute, size, accessor.m_ComponentType, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(attribute);
}

unsigned int gltf_load_texture(gltfTexture texture, int type, gltfSampler* gltf_samplers, gltfImage* gltf_images)
{
    gltfImage image = gltf_images[texture.m_SourceIndex];

    char filename[FILENAME_MAX];
    sprintf(filename, "%s/%s", currentDirectory, image.m_URI);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, numChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &numChannels, 0);

    if (data) {
        GLenum format;
        if (numChannels == 1)
            format = GL_RED;
        else if (numChannels == 3)
            format = GL_RGB;
        else if (numChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);

        if (type == 1) {
            // metallness value in "blue" color channel
            for (int i = 0; i < width * height * numChannels; i += numChannels) {
                data[i] = data[i + 2];
                data[i + 1] = data[i + 2];
            }
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        } else if (type == 2) {
            // roughness value in "green" color channel
            for (int i = 0; i < width * height * numChannels; i += numChannels) {
                data[i] = data[i + 1];
                data[i + 2] = data[i + 1];
            }

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        } else if (type == 3) {
            for (int i = 0; i < width * height * numChannels; i += numChannels) {
                data[i + 1] = data[i];
                data[i + 2] = data[i];
            }

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        } else {
            // Load the entire texture for other types (type = 0)
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // THESE IMPORTANT TO INCREASE TEXTURE QUALITY (from distance)
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

Material gltf_load_material(gltfMaterial gltf_material, gltfImage* gltf_images, gltfSampler* gltf_samplers, gltfTexture* textures)
{
    Material material;

    if (gltf_material.m_MetalicRoughness != NULL) {
        gltfMetallicRoughness gltf_metal_rough = *gltf_material.m_MetalicRoughness;

        if (gltf_metal_rough.m_BaseColorTexture != NULL) {
            gltfTexture base_color_texture = textures[gltf_metal_rough.m_BaseColorTexture->m_Index];
            material.m_BaseColorTextureId = gltf_load_texture(base_color_texture, 0, gltf_samplers, gltf_images);
        } else {
            material.m_BaseColorTextureId = 0;
        }

        if (gltf_metal_rough.m_MetallicRoughnessTexture != NULL) {
            gltfTexture metal_rough_texture = textures[gltf_metal_rough.m_MetallicRoughnessTexture->m_Index];
            material.m_MetallicTextureId = gltf_load_texture(metal_rough_texture, 1, gltf_samplers, gltf_images);
            material.m_RoughnessTextureId = gltf_load_texture(metal_rough_texture, 2, gltf_samplers, gltf_images);
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

    if (gltf_material.m_NormalTexture != NULL) {
        gltfTexture normal_texture = textures[gltf_material.m_NormalTexture->m_Index];
        material.m_NormalTextureId = gltf_load_texture(normal_texture, 0, gltf_samplers, gltf_images);
    } else {
        material.m_NormalTextureId = 0; // switch to default
    }

    if (gltf_material.m_OcclusionTexture != NULL) {
        gltfTexture occlusion_texture = textures[gltf_material.m_OcclusionTexture->m_Index];
        material.m_OcclusionTextureId = gltf_load_texture(occlusion_texture, 3, gltf_samplers, gltf_images);
    } else {
        material.m_OcclusionTextureId = 0; // switch to default
    }

    if (gltf_material.m_EmissiveTexture != NULL) {
        gltfTexture emissive_texture = textures[gltf_material.m_EmissiveTexture->m_Index];
        material.m_EmissiveTextureId = gltf_load_texture(emissive_texture, 0, gltf_samplers, gltf_images);
    } else {
        material.m_EmissiveTextureId = 0; // switch to default
    }

    material.m_EmissiveFactor = gltf_material.m_EmissiveFactor;

    return material;
}

void gltf_draw_mesh(unsigned int VAO, Material material, unsigned int numIndices, unsigned int shaderID)
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

//TODO
g_Mesh gltf_load_mesh() {
    g_Mesh mesh;
    mesh.m_NumIndices = 0;
    mesh.m_VAO = 0;

    return mesh;
}



#endif