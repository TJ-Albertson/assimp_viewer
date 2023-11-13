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

typedef struct gltfScene {
    char m_Name[256];
    int m_NumNodes;
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

void traverse_gltf_node()
{
    // Process the meshes, cameras, etc., that are
    // attached to this node - discussed later
    // processElements(node);

    // Recursively process all children
    /*
    for each (child in node.children) {
        traverse(child);
    }
    */
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