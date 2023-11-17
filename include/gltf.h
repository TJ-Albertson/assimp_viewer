/*-------------------------------------------------------------------------------\
gltf.h

gltf file importer
\-------------------------------------------------------------------------------*/
#ifndef GLTF_H
#define GLTF_H

#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <direct.h>

#include "gltf/gltf_structures.h"
#include "gltf/gltf_print.h"
#include "gltf/gltf_gl.h"
#include "gltf/gltf_process.h"
#include "gltf/gltf_memory.h"

char* gltf_load_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);

    return buffer;
}

// Checks gltf version number and extensions
void gltf_pre_check(cJSON* root)
{
    cJSON* asset = cJSON_GetObjectItem(root, "asset");
    if (asset != NULL) {
        cJSON* generator = cJSON_GetObjectItem(asset, "generator");
        if (generator != NULL && cJSON_IsString(generator)) {
            printf("generator: %s\n", generator->valuestring);
        } else {
            fprintf(stderr, "Error: 'generator' field not found or not a string.\n");
        }

        cJSON* version = cJSON_GetObjectItem(asset, "version");
        if (version != NULL && cJSON_IsString(version)) {
            printf("version: %s\n", version->valuestring);
        } else {
            fprintf(stderr, "Error: 'version' field not found or not a string.\n");
        }
    } else {
        fprintf(stderr, "Error: 'asset' field not found.\n");
    }

    cJSON* extensionsUsed = cJSON_GetObjectItem(root, "extensionsUsed");
    if (extensionsUsed != NULL) {
         printf("extensionsUsed:\n");
         int numExtensions = cJSON_GetArraySize(extensionsUsed);

         for (int i = 0; i < numExtensions; ++i) {
            cJSON* extension = cJSON_GetArrayItem(extensionsUsed, i);
            printf("    %s\n", extension->valuestring);
         }

    } else {
        fprintf(stderr, "Error: 'extensionsUsed' field not found.\n");
    }
}

int gltf_parse(const char* jsonString, g_Model& model)
{
    cJSON* root = cJSON_Parse(jsonString);
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return 0;
    }

    gltf_pre_check(root);

    int defaultSceneIndex = -1;
    if (cJSON_GetObjectItem(root, "scene")) {
        defaultSceneIndex = cJSON_GetObjectItem(root, "scene")->valueint;
    }

    // "scenes": []
    cJSON* scenes = cJSON_GetObjectItem(root, "scenes");
   
    cJSON* defaultScene = cJSON_GetArrayItem(scenes, defaultSceneIndex);
    cJSON* name = cJSON_GetObjectItem(defaultScene, "name");
    
    cJSON* rootNodeIndexes = cJSON_GetObjectItem(defaultScene, "nodes");

    int numRootNodes = cJSON_GetArraySize(rootNodeIndexes);

    gltfScene gltf_scene;
    strncpy(gltf_scene.m_Name, name->valuestring, sizeof(gltf_scene.m_Name));
    gltf_scene.m_NumNodes = numRootNodes;
    gltf_scene.m_Nodes = (gltfNode*)malloc(numRootNodes * sizeof(gltfNode));

    // "nodes": []
    cJSON* nodes = cJSON_GetObjectItem(root, "nodes");

    for (int i = 0; i < numRootNodes; i++) {
        int rootNodeIndex = cJSON_GetArrayItem(rootNodeIndexes, i)->valueint;
        cJSON* rootNode = cJSON_GetArrayItem(nodes, rootNodeIndex);
        gltfNode gltfNode = gltf_traverse_node(rootNode);
        gltf_scene.m_Nodes[i] = gltfNode;
    }


    // "animations": []
    

    // "materials: []
    cJSON* materials = cJSON_GetObjectItem(root, "materials");
    int numMaterials = cJSON_GetArraySize(materials);
    gltfMaterial* gltfMaterials = (gltfMaterial*)malloc(numMaterials * sizeof(gltfMaterial));

    for (int i = 0; i < numMaterials; ++i) {
        cJSON* material = cJSON_GetArrayItem(materials, i);
        gltfMaterials[i] = gltf_process_material(material);
    }


    // "meshes": []
    cJSON* meshes = cJSON_GetObjectItem(root, "meshes");
    int numMeshes = cJSON_GetArraySize(meshes);
    gltfMesh* gltfMeshes = (gltfMesh*)malloc(numMeshes * sizeof(gltfMesh));

    for (int i = 0; i < numMeshes; ++i) {
        cJSON* mesh = cJSON_GetArrayItem(meshes, i);
        gltfMeshes[i] = gltf_process_mesh(mesh);
    }

    
    // "textures": []
    cJSON* textures = cJSON_GetObjectItem(root, "textures");
    int numTextures = cJSON_GetArraySize(textures);
    gltfTexture* gltfTextures = (gltfTexture*)malloc(numTextures * sizeof(gltfTexture));

    for (int i = 0; i < numTextures; ++i) {
        cJSON* texture = cJSON_GetArrayItem(textures, i);
        gltfTextures[i] = gltf_process_texture(texture);
    }

    
    // "images": []
    cJSON* images = cJSON_GetObjectItem(root, "images");
    int numImages = cJSON_GetArraySize(images);
    gltfImage* gltfImages = (gltfImage*)malloc(numImages * sizeof(gltfImage));
        
    for (int i = 0; i < numImages; ++i) {
        cJSON* texture = cJSON_GetArrayItem(images, i);
        gltfImages[i] = gltf_process_image(texture);
    }
    

    // "accessors": []
    cJSON* accessors = cJSON_GetObjectItem(root, "accessors");
    int numAccessors = cJSON_GetArraySize(accessors);
    gltfAccessor* gltfAccessors = (gltfAccessor*)malloc(numAccessors * sizeof(gltfAccessor));

    for (int i = 0; i < numAccessors; ++i) {
        cJSON* accesssor = cJSON_GetArrayItem(accessors, i);
        gltfAccessors[i] = gltf_process_acccessor(accesssor);
    }


    // "bufferViews: []
    cJSON* bufferViews = cJSON_GetObjectItem(root, "bufferViews");
    int numbufferViews = cJSON_GetArraySize(accessors);

    gltfBufferView* gltfBufferViews = (gltfBufferView*)malloc(numAccessors * sizeof(gltfBufferView));

    for (int i = 0; i < numAccessors; ++i) {
        cJSON* bufferView = cJSON_GetArrayItem(bufferViews, i);
        gltfBufferViews[i] = gltf_process_bufferView(bufferView);
    }


    // "samplers: []
    cJSON* samplers = cJSON_GetObjectItem(root, "samplers");
    int numSamplers = cJSON_GetArraySize(samplers);
    gltfSampler* gltfSamplers = (gltfSampler*)malloc(numAccessors * sizeof(gltfSampler));

    for (int i = 0; i < numSamplers; ++i) {
        cJSON* sampler = cJSON_GetArrayItem(samplers, i);
        gltfSamplers[i] = gltf_process_sampler(sampler);
    }


    // "buffers: []
    cJSON* buffers = cJSON_GetObjectItem(root, "buffers");
    int numBuffers = cJSON_GetArraySize(buffers);
    gltfBuffer* gltfBuffers = (gltfBuffer*)malloc(numBuffers * sizeof(gltfBuffer));

    for (int i = 0; i < numBuffers; ++i) {
        cJSON* buffer = cJSON_GetArrayItem(buffers, i);
        gltfBuffers[i] = gltf_process_buffer(buffer);
    }

    cJSON_Delete(root);

    //print_gltf_scene(gltf_scene);
    //print_gltf_materials(gltfMaterials, numMaterials);
    //print_gltf_meshes(gltfMeshes, numMeshes);
    //print_gltf_textures(gltfTextures, numTextures);
    //print_gltf_images(gltfImages, numImages);
    //print_gltf_accessors(gltfAccessors, numAccessors);
    //print_gltf_bufferViews(gltfBufferViews, numbufferViews);
    //print_gltf_samplers(gltfSamplers, numSamplers);
    //print_gltf_buffers(gltfBuffers, numBuffers);


    // Reading binary files into buffers
    int totalBufferSize = 0;

    for (int i = 0; i < numBuffers; ++i) {
        gltfBuffer gltf_buffer = gltfBuffers[i];

        totalBufferSize += gltf_buffer.m_ByteLength;
    }

    char** allocatedBuffers = (char**)malloc(totalBufferSize);

    for (int i = 0; i < numBuffers; ++i) {
        gltfBuffer gltf_buffer = gltfBuffers[i];

        char* buffer = (char*)malloc(gltf_buffer.m_ByteLength);

        char relativePath[FILENAME_MAX];
        sprintf(relativePath, "%s/%s", currentDirectory, gltf_buffer.m_URI);

        printf("relativePath: %s\n", relativePath);

        FILE* ptr;
        ptr = fopen(relativePath, "rb");

        if (ptr == NULL) {
            fprintf(stderr, "Error: Unable to open file %s\n", relativePath);
            break;
        }

        fread(buffer, 1, gltf_buffer.m_ByteLength, ptr);

        allocatedBuffers[i] = buffer;

        fclose(ptr);
    }
    
    // Loading Meshes/Materials
    model.m_NumMeshes = numMeshes;
    model.m_NumMaterials = numMaterials;

    model.m_Meshes = (g_Mesh*)malloc(numMeshes * sizeof(g_Mesh));
    model.m_Materials = (Material*)malloc(numMaterials * sizeof(g_Mesh));
    model.m_Scene = gltf_scene;

    for (int i = 0; i < numMaterials; ++i) 
    {
        model.m_Materials[i] = gltf_load_material(gltfMaterials[i], gltfImages, gltfSamplers, gltfTextures);
    }

    for (int i = 0; i < numMeshes; ++i) 
    {
        model.m_Meshes[i] = gltf_load_mesh(gltfMeshes[i], gltfAccessors, gltfBufferViews, allocatedBuffers);
    }
    
    gltf_free_materials(gltfMaterials, numMaterials);
    gltf_free_meshes(gltfMeshes, numMeshes);
    
    free(gltfTextures);
    free(gltfImages);
    free(gltfAccessors);
    free(gltfBufferViews);
    free(gltfSamplers);

    for (int i = 0; i < numBuffers; ++i)
    {
        free(allocatedBuffers[i]);
    }
    free(allocatedBuffers);

    

    return 1;
}

int gltf_load_model(const char* filename, g_Model& model)
{
    char* jsonString = gltf_load_file(filename);

    const char* lastSlash = strrchr(filename, '/');

    if (lastSlash != NULL) {
        size_t length = lastSlash - filename;

        strncpy(currentDirectory, filename, length);
        currentDirectory[length] = '\0';
    } else {
        printf("No '/' found in filename.\n");
    }

    if (jsonString) {
        gltf_parse(jsonString, model);
        free(jsonString);
    }

    return 0;
}




#endif