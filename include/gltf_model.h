/*-------------------------------------------------------------------------------\
gltf_model.h

load gltf model for opengl
\-------------------------------------------------------------------------------*/
#ifndef GLTF_MODEL_H
#define GLTF_MODEL_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#define MAX_BONE_INFLUENCE 4

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "gltf.h" 

struct VertexData {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    glm::vec3 Color;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};


struct Texture {
    unsigned int m_OpenglId;
    char m_Type[256];
};

struct Mesh {
    unsigned int VAO;
    // unsigned int numVertices;

    // unsigned int* indices;
    unsigned int numIndices;

    Texture* m_Textures;
    unsigned int m_NumTextures;
};

typedef struct Material {
    unsigned int m_BaseColorTextureId;
    unsigned int m_MetallicTextureId;
    unsigned int m_RoughnessTextureId;
    unsigned int m_NormalTextureId;
    unsigned int m_OcclusionTextureId;
    unsigned int m_EmissiveTextureId;
} Material;

struct Model {
    char* m_Name;

    int m_NumMeshes;
    Mesh* m_Meshes;

    int m_NumAnimations;
    Animation* m_Animations;

    glm::mat4* m_FinalBoneMatrices;
    SkeletonNode* rootSkeletonNode;
};

void load_gltf_textures(gltfTexture* gltf_textures, gltfImage* gltf_images, gltfSampler* gltf_samplers)
{

}

void load_gltf_meshes(gltfMesh* gltf_Mesh, gltf)
{

}

void draw_gltf_mesh(Mesh mesh, Material material) 
{


    //setMaterialShaderMat


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

    // draw mesh
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.numIndices), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void draw_gltf_scene() {

}

#endif