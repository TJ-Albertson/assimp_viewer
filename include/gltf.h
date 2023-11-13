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

typedef struct gltfNode {
    char m_Name[256];

    int m_MeshIndex;
    int m_CameraIndex;
    int m_SkinIndex;

    int m_NumChildren;
    gltfNode* m_Children;
} gltfNode;

typedef struct gltfScene {
    char m_Name[256];
    int m_NumNodes;
    gltfNode* m_Nodes;
} gltfScene;

// Indexes go to Accessors[Index]
typedef struct gltfPrimitive {
    int m_PositionIndex;
    int m_NormalIndex;

    int m_IndicesIndex;
    int m_MaterialIndex;

    int m_NumTexCoord;
    int* m_TexCoordIndex;

} gltfPrimitive;

typedef struct glftMesh {
    char m_Name[256];

    int m_NumPrimitives;
    gltfPrimitive* m_Primitives;
    
} glftMesh;


typedef struct gltfBuffer {
    int m_ByteLength;
    char m_URI[256];
} gltfBuffer;

typedef struct gltfBufferView {
    int m_BufferIndex;
    int m_ByteOffset;
    int m_ByteLength;
    int m_ByteStride;
    int m_Target;
} gltfBufferView;

typedef struct gltfAccessor {
    int m_BufferViewIndex;
    int m_ByteOffset;
    char m_Type[256];
    int m_ComponentType;
    int m_Count;
    glm::vec2 min;
    glm::vec2 max;
} gltfAccessor;

/*
* GLTF Material data structs
*/
typedef struct gltfBaseColorTexture {
    int m_Index;
    int m_TexCoord;
} gltfBaseColorTexture;

typedef struct gltfMetallicRoughnessTexture {
    int m_Index;
    int m_TexCoord;
} gltfMetallicRoughnessTexture;

typedef struct gltfMetallicRoughness {
    gltfBaseColorTexture m_BaseColorTexture;

    glm::vec4 m_BaseColorFactor;

    gltfMetallicRoughnessTexture m_MetallicRoughnessTexture;

    float metallicFactor;
    float roughnessFactor;
} gltfMetallicRoughness;


typedef struct gltfNormalTexture {
    int m_Scale;
    int m_Index;
    int m_TexCoord;
} gltfNormalTexture;

typedef struct gltfOcclusionTexture {
    int m_Strength;
    int m_Index;
    int m_TexCoord;
} gltfOcclusionTexture;

typedef struct gltfEmissiveTexture {
    int m_Index;
    int m_TexCoord;
} gltfEmissiveTexture;


typedef struct gltfMaterial {
    gltfMetallicRoughness m_MettalicRoughness;

    gltfNormalTexture m_NormalTexture;
    
    gltfOcclusionTexture m_OcclustionTexture;

    gltfEmissiveTexture m_EmissiveTexture;

    glm::vec3 m_EmissiveFactor;
};





char* loadFile(const char* filename)
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
void gltfPreChecks(cJSON* root)
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

gltfNode gltf_process_node(cJSON* node)
{
    gltfNode gltf_node;

    if (cJSON_GetObjectItem(node, "name")) {
        strncpy(gltf_node.m_Name, cJSON_GetObjectItem(node, "name")->valuestring, sizeof(gltf_node.m_Name));
    } else {
        fprintf(stderr, "Error: 'name' field not found in Node.\n");
    }



    return gltf_node;
}

gltfNode traverse_gltf_node(cJSON* node)
{
    // Process the meshes, cameras, etc., that are
    // attached to this node - discussed later
    gltfNode gltf_node = gltf_process_node(node);


    if (cJSON_GetObjectItem(node, "children")) {
        cJSON* children = cJSON_GetObjectItem(node, "children");
        int numChildren = cJSON_GetArraySize(children);

        gltf_node.m_NumChildren = numChildren;
        gltf_node.
    }
    // Recursively process all children
    /*
    for each (child in node.children) {
        traverse(child);
    }
    */

    return gltf_node;
}

void parseGLTF(const char* jsonString)
{
    cJSON* root = cJSON_Parse(jsonString);
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return;
    }

    gltfPreChecks(root);

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

        gltfNode gltfNode = traverse_gltf_node(rootNode);

        gltf_scene.m_Nodes[i] = gltfNode;
    }

    printf("Scene:\n");
    printf("   Name: %s\n", gltf_scene.m_Name);
    printf("   RootNodes:\n");
    for (int i = 0; i < gltf_scene.m_NumNodes; ++i) {
        printf("        Node %d:\n", i);
        printf("            Name: %s\n", gltf_scene.m_Nodes[i].m_Name);
    }
    

    cJSON_Delete(root);
}

int LoadGLTF(const char* filename)
{
    char* jsonString = loadFile(filename);

    if (jsonString) {
        parseGLTF(jsonString);
        free(jsonString);
    }

    return 0;
}


#endif