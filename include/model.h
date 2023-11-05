/*-------------------------------------------------------------------------------\
model.h

Functions:
        A bunch of functions for loading textures and vertex data of meshes in model

        Last function to all meshes in model

\-------------------------------------------------------------------------------*/

#ifndef MODEL_H
#define MODEL_H

#define MAX_BONE_INFLUENCE 4

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stb_image.h>

#include <assimp_glm_helpers.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <animation.h>
#include <skeleton.h>

#include <collision.h>
#include <aabb.h>

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
    unsigned int id;
    const char* type;
    char* path;
};

struct Mesh {
    unsigned int VAO;
    //unsigned int numVertices;

   // unsigned int* indices;
    unsigned int numIndices;

    Texture* textures;
    unsigned int numTextures;
};

struct Model {
    char* m_Name;

    int m_NumMeshes;
    Mesh* m_Meshes;

    int m_NumAnimations;
    Animation* m_Animations;

    glm::mat4* m_FinalBoneMatrices;
    SkeletonNode* rootSkeletonNode;
};

std::vector<Texture> textures_loaded;
std::string directory;

Model* LoadModel(std::string const& path);

void processNode(aiNode* node, const aiScene* scene, Model* model);
Mesh processMesh(aiMesh* mesh, const aiScene* scene);

unsigned int TextureFromFile(const char* path, const std::string& directory);
void loadMaterialTextures(Texture* textures, int startIndex, int numTextures, aiMaterial* mat, aiTextureType type, const char* typeName);

void SetVertexBoneDataToDefault(VertexData& vertex);
void AssignBoneId(VertexData* vertexData, aiMesh* mesh, const aiScene* scene);

unsigned int LoadMeshVertexData(VertexData* vertices, unsigned int* indices, int numVertices, int numIndices);

void DrawModel(Model* model, unsigned int shaderID);





Model* LoadModel(std::string const& path)
{

    // I want textures_loaded to remain global for now. In future I will make sure to only load every texture once in case different models share textures.
    textures_loaded.clear();

    size_t lastSlashPos = path.find_last_of('/');
    std::string substring = path.substr(lastSlashPos + 1);
    size_t dotPos = substring.find('.');
    std::string nameFromPath = substring.substr(0, dotPos);

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return NULL;
    }

    Model* newModel = (Model*)malloc(sizeof(Model));

    newModel->m_Name = (char*)malloc(nameFromPath.size() * sizeof(char));

    std::strcpy(newModel->m_Name, nameFromPath.c_str());

    newModel->m_NumMeshes = scene->mNumMeshes;
    newModel->m_Meshes = (Mesh*)malloc(scene->mNumMeshes * sizeof(Mesh));

    newModel->m_NumAnimations = scene->mNumAnimations;

    if (scene->mNumAnimations > 0) {
        newModel->m_Animations = LoadAnimations(scene->mNumAnimations, scene->mAnimations);
    } else {
        newModel->m_Animations = nullptr;
    }

    newModel->rootSkeletonNode = LoadSkeleton(scene);
    newModel->m_FinalBoneMatrices = (glm::mat4*)malloc(100 * sizeof(glm::mat4));

    for (int i = 0; i < 100; ++i) {
        newModel->m_FinalBoneMatrices[i] = glm::mat4(1.0f);
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene, newModel);

    return newModel;
}

void processNode(aiNode* node, const aiScene* scene, Model* model)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        model->m_Meshes[i] = processMesh(mesh, scene);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, model);
    }
}

void SetVertexBoneDataToDefault(VertexData& vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene)
{
    int numVertices = mesh->mNumVertices;

    VertexData* vertices = (VertexData*)malloc(numVertices * sizeof(VertexData));

    int numFaces = mesh->mNumFaces;

    int numIndices = 0;
    for (int i = 0; i < numFaces; ++i)
        numIndices += mesh->mFaces[i].mNumIndices;

    unsigned int* indices = (unsigned int*)malloc(numIndices * sizeof(unsigned int));

    for (unsigned int i = 0; i < numVertices; i++) {

        VertexData vertexData;

        SetVertexBoneDataToDefault(vertexData);

        vertexData.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
        vertexData.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

        vertexData.Color = glm::vec3(0.0f, 0.0f, 0.0f);
        if (mesh->HasVertexColors(0)) {
            aiColor4D color = mesh->mColors[0][i];
            // std::cout << "Vertex " << i << " has color (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;

            vertexData.Color = glm::vec3(color.r, color.g, color.b);
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertexData.TexCoords = vec;
        } else {
            vertexData.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices[i] = vertexData;
    }

    int index = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices[index++] = face.mIndices[j];
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    int numDiffuse = aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE);
    int numSpecular = aiGetMaterialTextureCount(material, aiTextureType_SPECULAR);
    int numHeight = aiGetMaterialTextureCount(material, aiTextureType_HEIGHT);
    int numAmbient = aiGetMaterialTextureCount(material, aiTextureType_AMBIENT);
    int numEmissive = aiGetMaterialTextureCount(material, aiTextureType_EMISSIVE);

    int numTextures = numDiffuse + numSpecular + numHeight + numAmbient + numEmissive;

    Texture* textures = (Texture*)malloc(numTextures * sizeof(Texture));

    int textures_index = 0;
    loadMaterialTextures(textures, textures_index, numDiffuse, material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures_index += numDiffuse;
    loadMaterialTextures(textures, textures_index, numSpecular, material, aiTextureType_SPECULAR, "texture_specular");
    textures_index += numSpecular;
    loadMaterialTextures(textures, textures_index, numHeight, material, aiTextureType_HEIGHT, "texture_normal");
    textures_index += numHeight;
    loadMaterialTextures(textures, textures_index, numAmbient, material, aiTextureType_AMBIENT, "texture_height");
    textures_index += numAmbient;
    loadMaterialTextures(textures, textures_index, numEmissive, material, aiTextureType_EMISSIVE, "texture_emissive");

    AssignBoneId(vertices, mesh, scene);

    unsigned int VAO = LoadMeshVertexData(vertices, indices, numVertices, numIndices);

    Mesh newMesh = { VAO, numIndices, textures, numTextures };

    return newMesh;
}

void AssignBoneId(VertexData* vertexData, aiMesh* mesh, const aiScene* scene)
{
    for (int i = 0; i < mesh->mNumBones; ++i) {

        aiBone* bone = mesh->mBones[i];

        int boneID = -1;
        std::string boneName = bone->mName.C_Str();

        if (BoneMap.find(boneName) != BoneMap.end()) {
            boneID = BoneMap[boneName].ID;
        }

        int numWeights = bone->mNumWeights;

        // each weight
        for (int j = 0; j < numWeights; ++j) {

            int vertexId = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;

            // each vertex weight is effecting
            for (int k = 0; k < MAX_BONE_INFLUENCE; ++k) {
                if (vertexData[vertexId].m_BoneIDs[k] < 0) {
                    vertexData[vertexId].m_Weights[k] = weight;
                    vertexData[vertexId].m_BoneIDs[k] = boneID;
                    break;
                }
            }
        }
    }
}

unsigned int LoadMeshVertexData(VertexData* vertices, unsigned int* indices, int numVertices, int numIndices)
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
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexData), (void*)offsetof(VertexData, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, m_Weights));

    // vertex colors
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Color));

    glBindVertexArray(0);

    return VAO;
}

unsigned int TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
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
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture structs
void loadMaterialTextures(Texture* textures, int startIndex, int numTextures, aiMaterial* mat, aiTextureType type, const char* typeName)
{
    for (unsigned int i = 0; i < numTextures; ++i) {

        aiString str;
        mat->GetTexture(type, i, &str);

        // printf("GetTexture(): %s; typeName: %s\n", str.C_Str(), typeName);

        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); ++j) {
            if (std::strcmp(textures_loaded[j].path, str.C_Str()) == 0) {
                textures[startIndex + i] = textures_loaded[j];
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }

        if (!skip) { // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = (char*)malloc(strlen(str.C_Str()) + 1);
            strcpy(texture.path, str.C_Str());
            // printf("texture: %s; typeName: %s; index: %d\n", texture.path, texture.type, startIndex + i);

            textures[startIndex + i] = texture;
            textures_loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
}


std::vector<unsigned int> leaf_nodes;

void DrawModel(Model* model, unsigned int shaderID)
{
    for (unsigned int i = 0; i < model->m_NumMeshes; i++) {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int emissNr = 1;

        Mesh mesh = model->m_Meshes[i];

       

        for (unsigned int i = 0; i < mesh.numTextures; i++) {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding

            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            //printf("mesh.numTextures %d\n", mesh.numTextures);
            std::string name = std::string(mesh.textures[i].type);

            if (name == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
                name = "diffuse";
            } else if (name == "texture_specular") {
                number = std::to_string(specularNr++);
                name = "specular";
            } else if (name == "texture_normal") {
                number = std::to_string(normalNr++);
                name = "normal";
            } else if (name == "texture_height") {
                number = std::to_string(heightNr++);
                name = "height";
            } else if (name == "texture_emissive") {
                number = std::to_string(emissNr++);
                name = "emission";
            }

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shaderID, ("material." + name).c_str()), i);

            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }

        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.numIndices), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }
}

void DrawHitbox(unsigned int VAO, unsigned int shaderID)
{
    unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

unsigned int CreateHitbox()
{

    // Define cube vertices
    float vertices[] = {
        // Front face
        -0.5f,
        -0.5f,
        0.5f,
        0.5f,
        -0.5f,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        -0.5f,
        0.5f,
        0.5f,
        // Back face
        -0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        -0.5f,
    };

    unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return VAO;
}

void DrawAABB_Model(Model* model, unsigned int shaderID)
{
    for (int i = 0; i < model->m_NumMeshes; i++) {

        Mesh mesh = model->m_Meshes[i];

        /*
        glLineWidth(3.0f);
        glm::vec3 redColor = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 blueColor = glm::vec3(0.0f, 0.0f, 1.0f);
        float t = static_cast<float>(i) / static_cast<float>(model->m_NumMeshes - 1);
        glm::vec3 interpolatedColor = glm::mix(redColor, blueColor, t);
        glm::vec4 red = glm::vec4(interpolatedColor, 1.0f);
        glUniform4fv(glGetUniformLocation(shaderID, "color"), 1, &red[0]);
        */
        glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        glUniform4fv(glGetUniformLocation(shaderID, "color"), 1, &red[0]);

        glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        bool collision = false;

        //collising_aabbs
        for (const unsigned int& element : colliding_aabbs) {
            if (element == mesh.VAO) {
                collision = true;
                color = glm::vec4(1.0f / i, 1.0f, 0.0f, 1.0f);
            }
        }

        if (collision) {
            glLineWidth(3.0f);
            glUniform4fv(glGetUniformLocation(shaderID, "color"), 1, &color[0]);
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_LINES, sizeof(unsigned int[24]) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
           
        } 

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_LINES, sizeof(unsigned int[24]) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
 
        if (collision) {
            glLineWidth(0.5f);
        }
    }
}

void MarkAABBs(AABB_node* node, std::vector<unsigned int>& ids)
{
    MarkAABBs(node->left, ids);
    MarkAABBs(node->right, ids);
}


void LoadHitboxVAOs(AABB_node* node, std::vector<unsigned int>& vaos)
{
    if (node == NULL) {
        return;
    }

    //printf("node->type: %d\n", node->type);

    AABB aabb = node->aabb;
    

    float vertices[] = {
        aabb.min.x,
        aabb.min.y,
        aabb.max.z,

        aabb.max.x,
        aabb.min.y,
        aabb.max.z,

        aabb.max.x,
        aabb.max.y,
        aabb.max.z,

        aabb.min.x,
        aabb.max.y,
        aabb.max.z,

        aabb.min.x,
        aabb.min.y,
        aabb.min.z,

        aabb.max.x,
        aabb.min.y,
        aabb.min.z,

        aabb.max.x,
        aabb.max.y,
        aabb.min.z,

        aabb.min.x,
        aabb.max.y,
        aabb.min.z
    };

    unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    vaos.push_back(VAO);

    //aabb_map[VAO] = *node;
    node->id = VAO;

    if (node->type == LEAF) {
        leaf_nodes.push_back(VAO);
        return;
    }

    if (node->left != NULL) {
        LoadHitboxVAOs(node->left, vaos);
    }
    if (node->right != NULL) {
        LoadHitboxVAOs(node->right, vaos);
    }
    
    
}

Model* LoadAABB_Model(AABB_node* node)
{
    Model* model = (Model*)malloc(sizeof(Model));

    std::vector<unsigned int> vaos;

    LoadHitboxVAOs(node, vaos);

    Mesh* meshes = (Mesh*)malloc(vaos.size() * sizeof(Mesh));

    for (int i = 0; i < vaos.size(); i++) {
        meshes[i].VAO = vaos[i];
        meshes[i].numIndices = 24;

        meshes[i].numTextures = 0;
    }

    model->m_NumMeshes = vaos.size();
    model->m_Meshes = meshes;

    return model;
}


#endif