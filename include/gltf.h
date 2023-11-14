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

char currentDirectory[FILENAME_MAX];

enum ComponentType {
    signed_byte = 5120,
    unsigned_byte = 5121,
    signed_short = 5122,
    unsigned_short = 5123,
    unsigned_int = 5125,
    t_float = 5126
};

// takes componentType returns number of bytes
int component_size(int type) {
    if (type == signed_byte || type == unsigned_byte) {
        return 1;
    }
    if (type == signed_short || type == unsigned_short) {
        return 2;
    }
    if (type == unsigned_int || type == t_float) {
        return 4;
    }
    return -1;
}

// Can be camera sk
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

// Client implementations SHOULD support at least two texture coordinate sets, one vertex color, and one joints/weights set.
typedef struct gltfPrimitiveAttributes {
    int m_PositionIndex;
    int m_NormalIndex;
    int m_TangentIndex;

    int m_TexCoord_0_Index;
    int m_TexCoord_1_Index;

    int m_Color_0_Index;

    int m_Joints_0_Index;
    int m_Weights_0_Index;
} gltfPrimitiveAttributes;

// Indexes go to Accessors[Index]

typedef struct gltfPrimitive {
    gltfPrimitiveAttributes m_Attributes;

    int m_IndicesIndex;
    int m_MaterialIndex;
    int m_Mode;

} gltfPrimitive;

typedef struct gltfMesh {
    char m_Name[256];

    int m_NumPrimitives;
    gltfPrimitive* m_Primitives;
    
} gltfMesh;


typedef struct gltfTexture {
    int m_SamplerIndex;
    
    //Image;
    int m_SourceIndex;
} gltfTexture;

typedef struct gltfImage {
    char m_Name[256];
    char m_MimeType[256];
    char m_URI[256];
};

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
    int m_ComponentType;
    int m_Count;
    char m_Type[256];
} gltfAccessor;

typedef struct gltfSampler {
    int m_MagFilter;
    int m_MinFilter;
} gltfSampler;

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
    gltfBaseColorTexture* m_BaseColorTexture;
    glm::vec4 m_BaseColorFactor;
    gltfMetallicRoughnessTexture* m_MetallicRoughnessTexture;

    float m_MetallicFactor;
    float m_RoughnessFactor;
} gltfMetallicRoughness;


typedef struct gltfNormalTexture {
    float m_Scale;
    int m_Index;
    int m_TexCoord;
} gltfNormalTexture;

typedef struct gltfOcclusionTexture {
    float m_Strength;
    int m_Index;
    int m_TexCoord;
} gltfOcclusionTexture;

typedef struct gltfEmissiveTexture {
    int m_Index;
    int m_TexCoord;
} gltfEmissiveTexture;

typedef struct gltfMaterial {
    char m_Name[256];

    bool m_DoubleSided;

    gltfMetallicRoughness*  m_MetalicRoughness;
    gltfNormalTexture*      m_NormalTexture;
    gltfOcclusionTexture*   m_OcclusionTexture;
    gltfEmissiveTexture*    m_EmissiveTexture;

    glm::vec3 m_EmissiveFactor;
} gltfMaterial;

typedef struct gltfAnimationTarget {
    int m_Node;
    char m_Path[256];
} gltfAnimationTarget;

typedef struct glftChannel {
    int m_AnimationSamplerIndex;
    gltfAnimationTarget m_Target;
} glftChannel;

typedef struct gltfAnimationSampler {
    int m_Input;
    int m_Output;
    char m_Interpolation[256];
};

typedef struct gltfAnimation {

    char m_Name[256];

    glftChannel* m_Channels;
    gltfAnimationSampler* m_Samplers;

} gltfAnimation;

typedef struct gltfSkin {
    char m_Name[256];
    int m_InverseBindMatrices;

    int m_NumJoints;
    int* m_Joints;
} gltfSkin;


void print_gltf_scene(gltfScene gltf_scene);
void print_gltf_meshes(gltfMesh* gltf_meshes, int numMeshes);
void print_gltf_textures(gltfTexture* gltf_texture, int numTextures);
void print_gltf_images(gltfImage* gltf_images, int numImages);
void print_gltf_accessors(gltfAccessor* gltf_accessors, int numAccessors);
void print_gltf_bufferViews(gltfBufferView* gltf_bufferViews, int numBufferViews);
void print_gltf_samplers(gltfSampler* gltf_samplers, int numSamplers);
void print_gltf_buffers(gltfBuffer* gltf_buffers, int numBuffers);
void print_gltf_materials(gltfMaterial* gltf_materials, int numMaterials);


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

gltfBuffer gltf_process_buffer(cJSON* bufferNode)
{
    gltfBuffer gltf_buffer;

    if (cJSON_GetObjectItem(bufferNode, "byteLength")) {
        gltf_buffer.m_ByteLength = cJSON_GetObjectItem(bufferNode, "byteLength")->valueint;
    } else {
        gltf_buffer.m_ByteLength = -1;
    }

    if (cJSON_GetObjectItem(bufferNode, "uri")) {
        strncpy(gltf_buffer.m_URI, cJSON_GetObjectItem(bufferNode, "uri")->valuestring, sizeof(gltf_buffer.m_URI));
    } else {
        strncpy(gltf_buffer.m_URI, "null\0", sizeof(gltf_buffer.m_URI));
    }

    return gltf_buffer;
}

gltfSampler gltf_process_sampler(cJSON* samplerNode)
{
    gltfSampler gltf_sampler;

    if (cJSON_GetObjectItem(samplerNode, "magFilter")) {
        gltf_sampler.m_MagFilter = cJSON_GetObjectItem(samplerNode, "magFilter")->valueint;
    } else {
        gltf_sampler.m_MagFilter = -1;
    }

    if (cJSON_GetObjectItem(samplerNode, "minFilter")) {
        gltf_sampler.m_MinFilter = cJSON_GetObjectItem(samplerNode, "minFilter")->valueint;
    } else {
        gltf_sampler.m_MinFilter = -1;
    }

    return gltf_sampler;
}

gltfBufferView gltf_process_bufferView(cJSON* node)
{
    gltfBufferView gltf_bufferView;

    if (cJSON_GetObjectItem(node, "buffer")) {
        gltf_bufferView.m_BufferIndex = cJSON_GetObjectItem(node, "buffer")->valueint;
    } else {
        gltf_bufferView.m_BufferIndex = -1;
    }

    if (cJSON_GetObjectItem(node, "byteLength")) {
        gltf_bufferView.m_ByteLength = cJSON_GetObjectItem(node, "byteLength")->valueint;
    } else {
        gltf_bufferView.m_ByteLength = -1;
    }

    if (cJSON_GetObjectItem(node, "byteOffset")) {
        gltf_bufferView.m_ByteOffset = cJSON_GetObjectItem(node, "byteOffset")->valueint;
    } else {
        gltf_bufferView.m_ByteOffset = -1;
    }

    if (cJSON_GetObjectItem(node, "target")) {
        gltf_bufferView.m_Target = cJSON_GetObjectItem(node, "target")->valueint;
    } else {
        gltf_bufferView.m_Target = -1;
    }

    return gltf_bufferView;
}

gltfAccessor gltf_process_acccessor(cJSON* node)
{
    gltfAccessor gltf_accessor;

    if (cJSON_GetObjectItem(node, "bufferView")) {
        gltf_accessor.m_BufferViewIndex = cJSON_GetObjectItem(node, "bufferView")->valueint;
    } else {
        gltf_accessor.m_BufferViewIndex = -1;
    }

    if (cJSON_GetObjectItem(node, "byteOffset")) {
        gltf_accessor.m_ByteOffset = cJSON_GetObjectItem(node, "byteOffset")->valueint;
    } else {
        gltf_accessor.m_ByteOffset = -1;
    }

    if (cJSON_GetObjectItem(node, "componentType")) {
        gltf_accessor.m_ComponentType = cJSON_GetObjectItem(node, "componentType")->valueint;
    } else {
        gltf_accessor.m_ComponentType = -1;
    }

    if (cJSON_GetObjectItem(node, "count")) {
        gltf_accessor.m_Count = cJSON_GetObjectItem(node, "count")->valueint;
    } else {
        gltf_accessor.m_Count = -1;
    }

    if (cJSON_GetObjectItem(node, "type")) {
        strncpy(gltf_accessor.m_Type, cJSON_GetObjectItem(node, "type")->valuestring, sizeof(gltf_accessor.m_Type));
    } else {
        strncpy(gltf_accessor.m_Type, "null\0", sizeof(gltf_accessor.m_Type));
    }

    return gltf_accessor;
}

gltfImage gltf_process_image(cJSON* imageNode) {

    gltfImage gltf_image;

    if (cJSON_GetObjectItem(imageNode, "name")) {
        strncpy(gltf_image.m_Name, cJSON_GetObjectItem(imageNode, "name")->valuestring, sizeof(gltf_image.m_Name));
    } else {
        strncpy(gltf_image.m_Name, "null\0", sizeof(gltf_image.m_Name));
    }

    if (cJSON_GetObjectItem(imageNode, "mimeType")) {
        strncpy(gltf_image.m_MimeType, cJSON_GetObjectItem(imageNode, "mimeType")->valuestring, sizeof(gltf_image.m_MimeType));
    } else {
        strncpy(gltf_image.m_MimeType, "null\0", sizeof(gltf_image.m_MimeType));
    }

    if (cJSON_GetObjectItem(imageNode, "uri")) {
        strncpy(gltf_image.m_URI, cJSON_GetObjectItem(imageNode, "uri")->valuestring, sizeof(gltf_image.m_URI));
    } else {
        strncpy(gltf_image.m_URI, "null\0", sizeof(gltf_image.m_URI));
    }

    return gltf_image;
}

gltfTexture gltf_process_texture(cJSON* textureNode)
{
    gltfTexture gltf_texture;

    if (cJSON_GetObjectItem(textureNode, "sampler")) {
        gltf_texture.m_SamplerIndex = cJSON_GetObjectItem(textureNode, "sampler")->valueint;
    } else {
        gltf_texture.m_SamplerIndex = -1;
    }

    if (cJSON_GetObjectItem(textureNode, "source")) {
        gltf_texture.m_SourceIndex = cJSON_GetObjectItem(textureNode, "source")->valueint;
    } else {
        gltf_texture.m_SourceIndex = -1;
    }

    return gltf_texture;
}

gltfPrimitiveAttributes gltf_process_primitive_attributes(cJSON* primAttr)
{
    gltfPrimitiveAttributes gltf_primitive_attr;

    if (cJSON_GetObjectItem(primAttr, "POSITION")) {
        gltf_primitive_attr.m_PositionIndex = cJSON_GetObjectItem(primAttr, "POSITION")->valueint;
    } else {
        gltf_primitive_attr.m_PositionIndex = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "NORMAL")) {
        gltf_primitive_attr.m_NormalIndex = cJSON_GetObjectItem(primAttr, "NORMAL")->valueint;
    } else {
        gltf_primitive_attr.m_NormalIndex = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "TANGENT")) {
        gltf_primitive_attr.m_TangentIndex = cJSON_GetObjectItem(primAttr, "TANGENT")->valueint;
    } else {
        gltf_primitive_attr.m_TangentIndex = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "TEXCOORD_0")) {
        gltf_primitive_attr.m_TexCoord_0_Index = cJSON_GetObjectItem(primAttr, "TEXCOORD_0")->valueint;
    } else {
        gltf_primitive_attr.m_TexCoord_0_Index = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "TEXCOORD_1")) {
        gltf_primitive_attr.m_TexCoord_1_Index = cJSON_GetObjectItem(primAttr, "TEXCOORD_1")->valueint;
    } else {
        gltf_primitive_attr.m_TexCoord_1_Index = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "COLOR_0")) {
        gltf_primitive_attr.m_Color_0_Index = cJSON_GetObjectItem(primAttr, "COLOR_0")->valueint;
    } else {
        gltf_primitive_attr.m_Color_0_Index = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "JOINTS_0")) {
        gltf_primitive_attr.m_Joints_0_Index = cJSON_GetObjectItem(primAttr, "JOINTS_0")->valueint;
    } else {
        gltf_primitive_attr.m_Joints_0_Index = -1;
    }

    if (cJSON_GetObjectItem(primAttr, "WEIGHTS_0")) {
        gltf_primitive_attr.m_Weights_0_Index = cJSON_GetObjectItem(primAttr, "WEIGHTS_0")->valueint;
    } else {
        gltf_primitive_attr.m_Weights_0_Index = -1;
    }

    return gltf_primitive_attr;
}

gltfPrimitive gltf_process_primitive(cJSON* primNode) 
{
    gltfPrimitive gltf_primitive;

    if (cJSON_GetObjectItem(primNode, "attributes")) {
        cJSON* attributes = cJSON_GetObjectItem(primNode, "attributes");

        gltf_primitive.m_Attributes = gltf_process_primitive_attributes(attributes);
    } else {
        fprintf(stderr, "Error: 'attributes' field not found in Primitive.\n");
    }

    if (cJSON_GetObjectItem(primNode, "indices")) {
        gltf_primitive.m_IndicesIndex = cJSON_GetObjectItem(primNode, "indices")->valueint;
    } else {
        gltf_primitive.m_IndicesIndex = -1;
    }

    if (cJSON_GetObjectItem(primNode, "material")) {
        gltf_primitive.m_MaterialIndex = cJSON_GetObjectItem(primNode, "material")->valueint;
    } else {
        gltf_primitive.m_MaterialIndex = -1;
    }

    if (cJSON_GetObjectItem(primNode, "mode")) {
        gltf_primitive.m_Mode = cJSON_GetObjectItem(primNode, "mode")->valueint;
    } else {
        gltf_primitive.m_Mode = -1;
    }

    return gltf_primitive;
}

gltfMesh gltf_process_mesh(cJSON* meshNode)
{
    gltfMesh gltf_mesh;

    if (cJSON_GetObjectItem(meshNode, "name")) {
        strncpy(gltf_mesh.m_Name, cJSON_GetObjectItem(meshNode, "name")->valuestring, sizeof(gltf_mesh.m_Name));
    } else {
        fprintf(stderr, "Error: 'name' field not found in Mesh.\n");
    }

    if (cJSON_GetObjectItem(meshNode, "primitives")) {

        cJSON* primitives = cJSON_GetObjectItem(meshNode, "primitives");
        int numPrimitives = cJSON_GetArraySize(primitives);

        gltf_mesh.m_NumPrimitives = numPrimitives;
        gltf_mesh.m_Primitives = (gltfPrimitive*)malloc(numPrimitives * sizeof(gltfPrimitive));
        
        for (int i = 0; i < numPrimitives; ++i) {
            cJSON* primitive = cJSON_GetArrayItem(primitives, i);

            gltf_mesh.m_Primitives[i] = gltf_process_primitive(primitive);
        }
    } else {
        fprintf(stderr, "Error: 'primitives' field not found in Mesh.\n");
    }

    return gltf_mesh;
}


/*
*   Proccess material start
*/
gltfMetallicRoughness* gltf_process_metallic_roughness(cJSON* pbrNode)
{
    gltfMetallicRoughness* gltf_metal_rough = (gltfMetallicRoughness*)malloc(sizeof(gltfMetallicRoughness));

    if (cJSON_GetObjectItem(pbrNode, "baseColorTexture")) {
        cJSON* baseColorTextureNode = cJSON_GetObjectItem(pbrNode, "baseColorTexture");

        gltfBaseColorTexture* gltf_base_color_texture = (gltfBaseColorTexture*)malloc(sizeof(gltfBaseColorTexture));

        if (cJSON_GetObjectItem(baseColorTextureNode, "index")) {
            gltf_base_color_texture->m_Index = cJSON_GetObjectItem(baseColorTextureNode, "index")->valueint;
        } else {
            gltf_base_color_texture->m_Index = -1;
        }

        if (cJSON_GetObjectItem(baseColorTextureNode, "texCoord")) {
            gltf_base_color_texture->m_TexCoord = cJSON_GetObjectItem(baseColorTextureNode, "texCoord")->valueint;
        } else {
            gltf_base_color_texture->m_TexCoord = -1;
        }

        gltf_metal_rough->m_BaseColorTexture = gltf_base_color_texture;
    } else {
        gltf_metal_rough->m_BaseColorTexture = NULL;
    }

    if (cJSON_GetObjectItem(pbrNode, "baseColorFactor")) {
        cJSON* baseColorFactor = cJSON_GetObjectItem(pbrNode, "baseColorFactor");

        float x = cJSON_GetArrayItem(baseColorFactor, 0)->valuedouble;
        float y = cJSON_GetArrayItem(baseColorFactor, 1)->valuedouble;
        float z = cJSON_GetArrayItem(baseColorFactor, 2)->valuedouble;
        float w = cJSON_GetArrayItem(baseColorFactor, 3)->valuedouble;

        gltf_metal_rough->m_BaseColorFactor = glm::vec4(x, y, z, w);
    } else {
        gltf_metal_rough->m_BaseColorFactor = glm::vec4(1.0f);
    }

    if (cJSON_GetObjectItem(pbrNode, "metallicRoughnessTexture")) {

        cJSON* metallicRoughnessTexture = cJSON_GetObjectItem(pbrNode, "metallicRoughnessTexture");

        gltfMetallicRoughnessTexture* gltf_metal_rough_texture = (gltfMetallicRoughnessTexture*)malloc(sizeof(gltfMetallicRoughnessTexture));

        if (cJSON_GetObjectItem(metallicRoughnessTexture, "index")) {
            gltf_metal_rough_texture->m_Index = cJSON_GetObjectItem(metallicRoughnessTexture, "index")->valueint;
        } else {
            gltf_metal_rough_texture->m_Index = -1;
        }

        if (cJSON_GetObjectItem(metallicRoughnessTexture, "texCoord")) {
            gltf_metal_rough_texture->m_TexCoord = cJSON_GetObjectItem(metallicRoughnessTexture, "texCoord")->valueint;
        } else {
            gltf_metal_rough_texture->m_TexCoord = -1;
        }

        gltf_metal_rough->m_MetallicRoughnessTexture = gltf_metal_rough_texture;
    } else {
        gltf_metal_rough->m_MetallicRoughnessTexture = NULL;
    }

    if (cJSON_GetObjectItem(pbrNode, "metallicFactor")) {
        gltf_metal_rough->m_MetallicFactor = cJSON_GetObjectItem(pbrNode, "metallicFactor")->valuedouble;
    } else {
        gltf_metal_rough->m_MetallicFactor = 0.0f;
    }

    if (cJSON_GetObjectItem(pbrNode, "roughnessFactor")) {
        gltf_metal_rough->m_RoughnessFactor = cJSON_GetObjectItem(pbrNode, "roughnessFactor")->valuedouble;
    } else {
        gltf_metal_rough->m_RoughnessFactor = 0.0f;
    }

    return gltf_metal_rough;
}

gltfNormalTexture* gltf_process_normalTexture(cJSON* node)
{
    gltfNormalTexture* gltf_normal_texture = (gltfNormalTexture*)malloc(sizeof(gltfNormalTexture));
    

    if (cJSON_GetObjectItem(node, "scale")) {
        gltf_normal_texture->m_Scale = cJSON_GetObjectItem(node, "scale")->valuedouble;
    } else {
        gltf_normal_texture->m_Scale = -1;
    }

    if (cJSON_GetObjectItem(node, "index")) {
        gltf_normal_texture->m_Index = cJSON_GetObjectItem(node, "index")->valueint;
    } else {
        gltf_normal_texture->m_Index = -1;
    }

    if (cJSON_GetObjectItem(node, "texCoord")) {
        gltf_normal_texture->m_TexCoord = cJSON_GetObjectItem(node, "texCoord")->valueint;
    } else {
        gltf_normal_texture->m_TexCoord = -1;
    }

    return gltf_normal_texture;
}

gltfOcclusionTexture* gltf_process_occlusionTexture(cJSON* node)
{
    gltfOcclusionTexture* gltf_occlusion_texture = (gltfOcclusionTexture*)malloc(sizeof(gltfOcclusionTexture));

    if (cJSON_GetObjectItem(node, "scale")) {
        gltf_occlusion_texture->m_Strength = cJSON_GetObjectItem(node, "scale")->valuedouble;
    } else {
        gltf_occlusion_texture->m_Strength = -1;
    }

    if (cJSON_GetObjectItem(node, "index")) {
        gltf_occlusion_texture->m_Index = cJSON_GetObjectItem(node, "index")->valueint;
    } else {
        gltf_occlusion_texture->m_Index = -1;
    }

    if (cJSON_GetObjectItem(node, "texCoord")) {
        gltf_occlusion_texture->m_TexCoord = cJSON_GetObjectItem(node, "texCoord")->valueint;
    } else {
        gltf_occlusion_texture->m_TexCoord = -1;
    }

    return gltf_occlusion_texture;
}

gltfEmissiveTexture* gltf_process_emissiveTexture(cJSON* node)
{
    gltfEmissiveTexture* gltf_emissive_texture = (gltfEmissiveTexture*)malloc(sizeof(gltfEmissiveTexture));

    if (cJSON_GetObjectItem(node, "index")) {
        gltf_emissive_texture->m_Index = cJSON_GetObjectItem(node, "index")->valueint;
    } else {
        gltf_emissive_texture->m_Index = -1;
    }

    if (cJSON_GetObjectItem(node, "texCoord")) {
        gltf_emissive_texture->m_TexCoord = cJSON_GetObjectItem(node, "texCoord")->valueint;
    } else {
        gltf_emissive_texture->m_TexCoord = -1;
    }

    return gltf_emissive_texture;
}

gltfMaterial gltf_process_material(cJSON* materialNode)
{
    gltfMaterial gltf_material;

    if (cJSON_GetObjectItem(materialNode, "name")) {
        strncpy(gltf_material.m_Name, cJSON_GetObjectItem(materialNode, "name")->valuestring, sizeof(gltf_material.m_Name));
    } else {
        fprintf(stderr, "Error: 'name' field not found in Material.\n");
    }

    if (cJSON_GetObjectItem(materialNode, "pbrMetallicRoughness")) {
        cJSON* pbrNode = cJSON_GetObjectItem(materialNode, "pbrMetallicRoughness");

        gltf_material.m_MetalicRoughness = gltf_process_metallic_roughness(pbrNode);
        
    } else {
        fprintf(stderr, "Error: 'pbrMetallicRoughness' field not found in Material.\n");
    }

    if (cJSON_GetObjectItem(materialNode, "normalTexture")) {
        cJSON* normalTexture = cJSON_GetObjectItem(materialNode, "normalTexture");

        gltf_material.m_NormalTexture = gltf_process_normalTexture(normalTexture);
    } else {
        gltf_material.m_NormalTexture = NULL;
    }

    if (cJSON_GetObjectItem(materialNode, "occlusionTexture")) {
        cJSON* occlusionTexture = cJSON_GetObjectItem(materialNode, "occlusionTexture");

        gltf_material.m_OcclusionTexture = gltf_process_occlusionTexture(occlusionTexture);
    } else {
        gltf_material.m_OcclusionTexture = NULL;
    }

    if (cJSON_GetObjectItem(materialNode, "emissiveTexture")) {
        cJSON* emissiveTexture = cJSON_GetObjectItem(materialNode, "emissiveTexture");

        gltf_material.m_EmissiveTexture = gltf_process_emissiveTexture(emissiveTexture);
    } else {
        gltf_material.m_EmissiveTexture = NULL;
    }

    if (cJSON_GetObjectItem(materialNode, "emissiveFactor")) {
        cJSON* emmissiveFactor = cJSON_GetObjectItem(materialNode, "baseColorFactor");

        float x = cJSON_GetArrayItem(emmissiveFactor, 0)->valuedouble;
        float y = cJSON_GetArrayItem(emmissiveFactor, 1)->valuedouble;
        float z = cJSON_GetArrayItem(emmissiveFactor, 2)->valuedouble;
        float w = cJSON_GetArrayItem(emmissiveFactor, 3)->valuedouble;

        gltf_material.m_EmissiveFactor = glm::vec4(x, y, z, w);
    } else {
        gltf_material.m_EmissiveFactor = glm::vec4(0.0f);
    }

    return gltf_material;
}

gltfNode gltf_process_node(cJSON* node)
{
    gltfNode gltf_node;

    if (cJSON_GetObjectItem(node, "name")) {
        strncpy(gltf_node.m_Name, cJSON_GetObjectItem(node, "name")->valuestring, sizeof(gltf_node.m_Name));
    } else {
        fprintf(stderr, "Error: 'name' field not found in Node.\n");
    }

    if (cJSON_GetObjectItem(node, "mesh")) {
        gltf_node.m_MeshIndex = cJSON_GetObjectItem(node, "mesh")->valueint;
    } else {
        gltf_node.m_MeshIndex = -1;
    }

    if (cJSON_GetObjectItem(node, "camera")) {
        gltf_node.m_CameraIndex = cJSON_GetObjectItem(node, "camera")->valueint;
    } else {
        gltf_node.m_CameraIndex = -1;
    }

    if (cJSON_GetObjectItem(node, "skin")) {
        gltf_node.m_SkinIndex = cJSON_GetObjectItem(node, "skin")->valueint;
    } else {
        gltf_node.m_SkinIndex = -1;
    }

    return gltf_node;
}

gltfNode traverse_gltf_node(cJSON* node)
{
    // Process the meshes, cameras, etc., that are
    // attached to this node - discussed later
    gltfNode gltf_node = gltf_process_node(node);

    // Recursively process all children
    if (cJSON_GetObjectItem(node, "children")) {
        cJSON* children = cJSON_GetObjectItem(node, "children");
        int numChildren = cJSON_GetArraySize(children);

        gltf_node.m_NumChildren = numChildren;
        gltf_node.m_Children = (gltfNode*)malloc(numChildren * sizeof(gltfNode));

        for (int i = 0; i < numChildren; ++i) {
            cJSON* child = cJSON_GetArrayItem(children, i);

            gltf_node.m_Children[i] = traverse_gltf_node(child);
        }
    } else {
        gltf_node.m_NumChildren = -1;
    }
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

    print_gltf_scene(gltf_scene);

    // "animations": []
    
    
    // "materials: []
    cJSON* materials = cJSON_GetObjectItem(root, "materials");
    int numMaterials = cJSON_GetArraySize(materials);

    gltfMaterial* gltfMaterials = (gltfMaterial*)malloc(numMaterials * sizeof(gltfMaterial));

    for (int i = 0; i < numMaterials; ++i) {
        cJSON* material = cJSON_GetArrayItem(materials, i);

        gltfMaterials[i] = gltf_process_material(material);
    }

    print_gltf_materials(gltfMaterials, numMaterials);

    // "meshes": []
    cJSON* meshes = cJSON_GetObjectItem(root, "meshes");
    int numMeshes = cJSON_GetArraySize(meshes);

    gltfMesh* gltfMeshes = (gltfMesh*)malloc(numMeshes * sizeof(gltfMesh));

    for (int i = 0; i < numMeshes; ++i) {
        cJSON* mesh = cJSON_GetArrayItem(meshes, i);

        gltfMeshes[i] = gltf_process_mesh(mesh);
    }

    print_gltf_meshes(gltfMeshes, numMeshes);




    // "textures": []
    cJSON* textures = cJSON_GetObjectItem(root, "textures");
    int numTextures = cJSON_GetArraySize(textures);

    gltfTexture* gltfTextures = (gltfTexture*)malloc(numTextures * sizeof(gltfTexture));

    for (int i = 0; i < numTextures; ++i) {
        cJSON* texture = cJSON_GetArrayItem(textures, i);

        gltfTextures[i] = gltf_process_texture(texture);
    }

    print_gltf_textures(gltfTextures, numTextures);

    // "images": []
    cJSON* images = cJSON_GetObjectItem(root, "images");
    int numImages = cJSON_GetArraySize(images);

    gltfImage* gltfImages = (gltfImage*)malloc(numImages * sizeof(gltfImage));
        
    for (int i = 0; i < numImages; ++i) {
        cJSON* texture = cJSON_GetArrayItem(images, i);

        gltfImages[i] = gltf_process_image(texture);
    }

    print_gltf_images(gltfImages, numImages);

    // "accessors": []
    cJSON* accessors = cJSON_GetObjectItem(root, "accessors");
    int numAccessors = cJSON_GetArraySize(accessors);

    gltfAccessor* gltfAccessors = (gltfAccessor*)malloc(numAccessors * sizeof(gltfAccessor));

    for (int i = 0; i < numAccessors; ++i) {
        cJSON* accesssor = cJSON_GetArrayItem(accessors, i);

        gltfAccessors[i] = gltf_process_acccessor(accesssor);
    }

    print_gltf_accessors(gltfAccessors, numAccessors);

    // "bufferViews: []
    cJSON* bufferViews = cJSON_GetObjectItem(root, "bufferViews");
    int numbufferViews = cJSON_GetArraySize(accessors);

    gltfBufferView* gltfBufferViews = (gltfBufferView*)malloc(numAccessors * sizeof(gltfBufferView));

    for (int i = 0; i < numAccessors; ++i) {
        cJSON* bufferView = cJSON_GetArrayItem(bufferViews, i);

        gltfBufferViews[i] = gltf_process_bufferView(bufferView);
    }

    print_gltf_bufferViews(gltfBufferViews, numbufferViews);

    // "samplers: []
    cJSON* samplers = cJSON_GetObjectItem(root, "samplers");
    int numSamplers = cJSON_GetArraySize(samplers);

    gltfSampler* gltfSamplers = (gltfSampler*)malloc(numAccessors * sizeof(gltfSampler));

    for (int i = 0; i < numSamplers; ++i) {
        cJSON* sampler = cJSON_GetArrayItem(samplers, i);

        gltfSamplers[i] = gltf_process_sampler(sampler);
    }

    print_gltf_samplers(gltfSamplers, numSamplers);

    // "samplers: []
    cJSON* buffers = cJSON_GetObjectItem(root, "buffers");
    int numBuffers = cJSON_GetArraySize(buffers);

    gltfBuffer* gltfBuffers = (gltfBuffer*)malloc(numBuffers * sizeof(gltfBuffer));

    for (int i = 0; i < numSamplers; ++i) {
        cJSON* buffer = cJSON_GetArrayItem(buffers, i);

        gltfBuffers[i] = gltf_process_buffer(buffer);
    }

    print_gltf_buffers(gltfBuffers, numBuffers);

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

    for (int i = 0; i < numMeshes; ++i) 
    {
        gltfMesh gltf_mesh = gltfMeshes[i];

        for (int j = 0; j < gltf_mesh.m_NumPrimitives; ++j) 
        {
            gltfPrimitive gltf_primitive = gltf_mesh.m_Primitives[j];

            if (gltf_primitive.m_IndicesIndex >= 0) {
                gltfAccessor accessor = gltfAccessors[gltf_primitive.m_IndicesIndex];

                int count = accessor.m_Count;

                gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

                int bufferIndex = bufferView.m_BufferIndex;

                int offset = 0;

                if (bufferView.m_ByteOffset > 0) {
                    offset += bufferView.m_ByteOffset;
                }

                if (accessor.m_ByteOffset > 0) {
                    offset += accessor.m_ByteOffset;
                }

                char* buffer = allocatedBuffers[bufferIndex];

                buffer = buffer + offset;

                unsigned short* indices = (unsigned short*)malloc(count * sizeof(unsigned short));

                int componentSize = component_size(accessor.m_ComponentType);

                printf("offset: %d\n", offset);
                printf("count: %d\n", count);
                printf("componentSize: %d\n", componentSize);

                printf("Indices:\n");
                for (int k = 0; k < count; ++k) {
                    float x;
                    memcpy(&x, buffer + k * componentSize, sizeof(componentSize));

                    printf("    indice[%d]: %d\n", k, x);

                    indices[k] = x;
                }
            }

            gltfPrimitiveAttributes attributes = gltf_primitive.m_Attributes;

            if (attributes.m_PositionIndex >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_PositionIndex];

                int count = accessor.m_Count;
                
                gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

                int bufferIndex = bufferView.m_BufferIndex;

                int offset = 0;

                if (bufferView.m_ByteOffset > 0) {
                    offset += bufferView.m_ByteOffset;
                }

                if (accessor.m_ByteOffset > 0) {
                    offset += accessor.m_ByteOffset;
                }

                char* buffer = allocatedBuffers[bufferIndex];

                buffer = buffer + offset;
               
                glm::vec3* positions = (glm::vec3*)malloc(count * sizeof(glm::vec3));

                int componentSize = component_size(accessor.m_ComponentType);

                printf("Positions:\n");
                for (int k = 0; k < count; ++k) {
                    float x, y, z;
                    memcpy(&x, buffer + k * componentSize, sizeof(componentSize));
                    memcpy(&y, buffer + k * componentSize, sizeof(componentSize));
                    memcpy(&z, buffer + k * componentSize, sizeof(componentSize));

                    printf("    position[%d]: %f %f %f\n", k, x, y, z);

                    positions[k].x = x;
                    positions[k].y = y;
                    positions[k].z = z;
                }
            }

            if (attributes.m_TexCoord_0_Index >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_TexCoord_0_Index];

                int count = accessor.m_Count;

                gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

                int bufferIndex = bufferView.m_BufferIndex;

                int offset = 0;

                if (bufferView.m_ByteOffset > 0) {
                    offset += bufferView.m_ByteOffset;
                }

                if (accessor.m_ByteOffset > 0) {
                    offset += accessor.m_ByteOffset;
                }

                char* buffer = allocatedBuffers[bufferIndex];

                buffer = buffer + offset;

                glm::vec3* texCoords = (glm::vec3*)malloc(count * sizeof(glm::vec3));

                printf("TexCoords_0:\n");
                for (int k = 0; k < count; ++k) {
                    float x, y;
                    memcpy(&x, buffer + k * 4, sizeof(float));
                    memcpy(&y, buffer + k * 4, sizeof(float));

                    printf("    texCoords[%d]: %f %f\n", k, x, y);

                    texCoords[k].x = x;
                    texCoords[k].y = y;
                }
            }

            if (attributes.m_TexCoord_1_Index >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_TexCoord_1_Index];

                int count = accessor.m_Count;

                gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

                int bufferIndex = bufferView.m_BufferIndex;

                int offset = 0;

                if (bufferView.m_ByteOffset > 0) {
                    offset += bufferView.m_ByteOffset;
                }

                if (accessor.m_ByteOffset > 0) {
                    offset += accessor.m_ByteOffset;
                }

                char* buffer = allocatedBuffers[bufferIndex];

                buffer = buffer + offset;

                glm::vec3* texCoords = (glm::vec3*)malloc(count * sizeof(glm::vec3));

                printf("TexCoords_1:\n");
                for (int k = 0; k < count; ++k) {
                    float x, y;
                    memcpy(&x, buffer + k * 4, sizeof(float));
                    memcpy(&y, buffer + k * 4, sizeof(float));

                    printf("    texCoords[%d]: %f %f\n", k, x, y);

                    texCoords[k].x = x;
                    texCoords[k].y = y;
                }
            }

            if (attributes.m_NormalIndex >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_NormalIndex];

                int count = accessor.m_Count;

                gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

                int bufferIndex = bufferView.m_BufferIndex;

                int offset = 0;

                if (bufferView.m_ByteOffset > 0) {
                    offset += bufferView.m_ByteOffset;
                }

                if (accessor.m_ByteOffset > 0) {
                    offset += accessor.m_ByteOffset;
                }

                char* buffer = allocatedBuffers[bufferIndex];

                buffer = buffer + offset;

                glm::vec3* normals = (glm::vec3*)malloc(count * sizeof(glm::vec3));

                printf("Normals:\n");
                for (int k = 0; k < count; ++k) {
                    float x, y, z;
                    memcpy(&x, buffer + k * 4, sizeof(float));
                    memcpy(&y, buffer + k * 4, sizeof(float));
                    memcpy(&z, buffer + k * 4, sizeof(float));

                    printf("    normal[%d]: %f %f %f\n", k, x, y, z);

                    normals[k].x = x;
                    normals[k].y = y;
                    normals[k].z = z;
                }
            }

            if (attributes.m_TangentIndex >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_TangentIndex];

                int count = accessor.m_Count;

                gltfBufferView bufferView = gltfBufferViews[accessor.m_BufferViewIndex];

                int bufferIndex = bufferView.m_BufferIndex;

                int offset = 0;

                if (bufferView.m_ByteOffset > 0) {
                    offset += bufferView.m_ByteOffset;
                }

                if (accessor.m_ByteOffset > 0) {
                    offset += accessor.m_ByteOffset;
                }

                char* buffer = allocatedBuffers[bufferIndex];

                buffer = buffer + offset;

                glm::vec4* tangents = (glm::vec4*)malloc(count * sizeof(glm::vec4));

                int componentSize = component_size(accessor.m_ComponentType);

                printf("Tangents:\n");
                for (int k = 0; k < count; ++k) {
                    float x, y, z, w;
                    memcpy(&x, buffer + k * componentSize, sizeof(componentSize));
                    memcpy(&y, buffer + k * componentSize, sizeof(componentSize));
                    memcpy(&z, buffer + k * componentSize, sizeof(componentSize));
                    memcpy(&w, buffer + k * componentSize, sizeof(componentSize));

                    printf("    tangent[%d]: %f %f %f %f\n", k, x, y, z, w);

                    tangents[k].x = x;
                    tangents[k].y = y;
                    tangents[k].z = z;
                    tangents[k].w = w;
                }
            }

            
        }
    }
    
    cJSON_Delete(root);

}

int LoadGLTF(const char* filename)
{
    char* jsonString = loadFile(filename);

    const char* lastSlash = strrchr(filename, '/');

    if (lastSlash != NULL) {
        size_t length = lastSlash - filename;

        strncpy(currentDirectory, filename, length);
        currentDirectory[length] = '\0';
    } else {
        printf("No '/' found in filename.\n");
    }

    if (jsonString) {
        parseGLTF(jsonString);
        free(jsonString);
    }

    return 0;
}



void print_gltf_node(gltfNode node)
{
    printf("            Name: %s\n", node.m_Name);

    if (node.m_MeshIndex >= 0) {
        printf("            m_MeshIndex: %d\n", node.m_MeshIndex);
    }
    if (node.m_CameraIndex >= 0) {
        printf("            m_CameraIndex: %d\n", node.m_CameraIndex);
    }
    if (node.m_SkinIndex >= 0) {
        printf("            m_SkinIndex: %d\n", node.m_SkinIndex);
    }
}

void print_gltf_scene(gltfScene gltf_scene) {

    printf("Scene:\n");
    printf("   Name: %s\n", gltf_scene.m_Name);
    printf("   RootNodes:\n");
    for (int i = 0; i < gltf_scene.m_NumNodes; ++i) {
        printf("        Node %d:\n", i);
        print_gltf_node(gltf_scene.m_Nodes[i]);
    }
}

/*
*  Materials print
*/
void print_baseColorTexture(gltfBaseColorTexture* texture) {
    if (texture->m_Index >= 0) {
        printf("            m_Index: %d\n", texture->m_Index);
    }

    if (texture->m_TexCoord >= 0) {
        printf("            m_TexCoord: %d\n", texture->m_TexCoord);
    }
}

void print_metallicRoughnessTexture(gltfMetallicRoughnessTexture* texture)
{
    if (texture->m_Index >= 0) {
        printf("            m_Index: %d\n", texture->m_Index);
    }

    if (texture->m_TexCoord >= 0) {
        printf("            m_TexCoord: %d\n", texture->m_TexCoord);
    }
}

void print_metallic_roughness(gltfMetallicRoughness* gltfMetallicRoughness) {

    printf("    PbrMetallicRoughness:\n");

    if (gltfMetallicRoughness->m_BaseColorTexture != NULL) {
        printf("        BaseColorTexture:\n");
        print_baseColorTexture(gltfMetallicRoughness->m_BaseColorTexture);
    }

    glm::vec4 baseColorFactor = gltfMetallicRoughness->m_BaseColorFactor;

    printf("        m_BaseColorFactor: %f %f %f %f\n", baseColorFactor.x, baseColorFactor.y, baseColorFactor.z, baseColorFactor.w);

    if (gltfMetallicRoughness->m_MetallicRoughnessTexture != NULL) {
        printf("        MetallicRoughness:\n");
        print_metallicRoughnessTexture(gltfMetallicRoughness->m_MetallicRoughnessTexture);
    }
   
    printf("        m_MetallicFactor: %f\n", gltfMetallicRoughness->m_MetallicFactor);
    printf("        m_RoughnessFactor: %f\n", gltfMetallicRoughness->m_RoughnessFactor);
}

void print_gltf_material(gltfMaterial gltf_material)
{
    printf("   Name: %s\n", gltf_material.m_Name);

    print_metallic_roughness(gltf_material.m_MetalicRoughness);
}

void print_gltf_materials(gltfMaterial* gltf_materials, int numMaterials)
{
    printf("Materials:\n");

    for (int i = 0; i < numMaterials; ++i) {
        print_gltf_material(gltf_materials[i]);
    }
}

/*
 *  Mesh print
 */
void print_gltf_primitive_attributes(gltfPrimitiveAttributes gltf_primitive_attributes) {
    if (gltf_primitive_attributes.m_PositionIndex >= 0) {
        printf("            m_PositionIndex: %d\n", gltf_primitive_attributes.m_PositionIndex);
    }

    if (gltf_primitive_attributes.m_NormalIndex >= 0) {
        printf("            m_NormalIndex: %d\n", gltf_primitive_attributes.m_NormalIndex);
    }

    if (gltf_primitive_attributes.m_TangentIndex >= 0) {
        printf("            m_TangentIndex: %d\n", gltf_primitive_attributes.m_TangentIndex);
    }

    if (gltf_primitive_attributes.m_TexCoord_0_Index >= 0) {
        printf("            m_TexCoord_0_Index: %d\n", gltf_primitive_attributes.m_TexCoord_0_Index);
    }

    if (gltf_primitive_attributes.m_TexCoord_1_Index >= 0) {
        printf("            m_TexCoord_1_Index: %d\n", gltf_primitive_attributes.m_TexCoord_1_Index);
    }

    if (gltf_primitive_attributes.m_Color_0_Index >= 0) {
        printf("            m_Color_0_Index: %d\n", gltf_primitive_attributes.m_Color_0_Index);
    }

    if (gltf_primitive_attributes.m_Joints_0_Index >= 0) {
        printf("            m_Joints_0_Index: %d\n", gltf_primitive_attributes.m_Joints_0_Index);
    }

    if (gltf_primitive_attributes.m_Weights_0_Index >= 0) {
        printf("            m_Weights_0_Index: %d\n", gltf_primitive_attributes.m_Weights_0_Index);
    }
}

void print_gltf_primitive(gltfPrimitive gltf_primitive) {

    printf("        Attributes\n");
    print_gltf_primitive_attributes(gltf_primitive.m_Attributes);

    if (gltf_primitive.m_IndicesIndex >= 0) {
        printf("        m_IndicesIndex: %d\n", gltf_primitive.m_IndicesIndex);
    }

    if (gltf_primitive.m_MaterialIndex >= 0) {
        printf("        m_MaterialIndex: %d\n", gltf_primitive.m_MaterialIndex);
    }

    if (gltf_primitive.m_Mode >= 0) {
        printf("        m_Mode: %d\n", gltf_primitive.m_Mode);
    }
}

void print_gltf_mesh(gltfMesh gltf_mesh) {
    printf("    Name: %s\n", gltf_mesh.m_Name);

    printf("    Primitives:\n");
    for (int i = 0; i < gltf_mesh.m_NumPrimitives; ++i) {
        print_gltf_primitive(gltf_mesh.m_Primitives[i]);
    }
}

void print_gltf_meshes(gltfMesh* gltf_meshes, int numMeshes)
{
    printf("Meshes:\n");

    for (int i = 0; i < numMeshes; ++i) {
        print_gltf_mesh(gltf_meshes[i]);
    }
}


void print_gltf_texture(gltfTexture gltf_texture) {
    if (gltf_texture.m_SamplerIndex >= 0) {
        printf("        m_SamplerIndex: %d\n", gltf_texture.m_SamplerIndex);
    }

    if (gltf_texture.m_SourceIndex >= 0) {
        printf("        m_SourceIndex: %d\n", gltf_texture.m_SourceIndex);
    }
    printf("\n");
}

void print_gltf_textures(gltfTexture* gltf_texture, int numTextures) {

    printf("Textures:\n");

    for (int i = 0; i < numTextures; ++i) {
        print_gltf_texture(gltf_texture[i]);
    }
}

void print_gltf_image(gltfImage gltf_image) {
    printf("    m_Name: %s\n", gltf_image.m_Name);
    printf("    m_MimeType: %s\n", gltf_image.m_MimeType);
    printf("    m_URI: %s\n", gltf_image.m_URI);
}

void print_gltf_images(gltfImage* gltf_images, int numImages) 
{
    printf("Images:\n");

    for (int i = 0; i < numImages; ++i) {
        print_gltf_image(gltf_images[i]);
    }
}

void print_gltf_accessor(gltfAccessor gltf_accessor)
{
    if (gltf_accessor.m_BufferViewIndex >= 0) {
        printf("    m_BufferViewIndex: %d\n", gltf_accessor.m_BufferViewIndex);
    }

    if (gltf_accessor.m_ByteOffset >= 0) {
        printf("    m_ByteOffset: %d\n", gltf_accessor.m_ByteOffset);
    }

    if (gltf_accessor.m_ComponentType >= 0) {
        printf("    m_ComponentType: %d\n", gltf_accessor.m_ComponentType);
    }

    if (gltf_accessor.m_Count >= 0) {
        printf("    m_Count: %d\n", gltf_accessor.m_Count);
    }

    printf("    m_Type: %s\n", gltf_accessor.m_Type);
}

void print_gltf_accessors(gltfAccessor* gltf_accessors, int numAccessors)
{
    printf("Accessors:\n");

    for (int i = 0; i < numAccessors; ++i) {
        print_gltf_accessor(gltf_accessors[i]);
        printf("\n");
    }
}

void print_gltf_bufferView(gltfBufferView gltf_buffer)
{
    if (gltf_buffer.m_BufferIndex >= 0) {
        printf("    m_BufferIndex: %d\n", gltf_buffer.m_BufferIndex);
    }

    if (gltf_buffer.m_ByteLength >= 0) {
        printf("    m_ByteLength: %d\n", gltf_buffer.m_ByteLength);
    }

    if (gltf_buffer.m_ByteOffset >= 0) {
        printf("    m_ByteOffset: %d\n", gltf_buffer.m_ByteOffset);
    }

    if (gltf_buffer.m_Target >= 0) {
        printf("    m_Target: %d\n", gltf_buffer.m_Target);
    }
}

void print_gltf_bufferViews(gltfBufferView* gltf_bufferViews, int numBufferViews) 
{
    printf("BufferViews:\n");

    for (int i = 0; i < numBufferViews; ++i) {
        print_gltf_bufferView(gltf_bufferViews[i]);
        printf("\n");
    }
}

void print_gltf_sampler(gltfSampler gltf_sampler)
{
    if (gltf_sampler.m_MagFilter >= 0) {
        printf("    m_MagFilter: %d\n", gltf_sampler.m_MagFilter);
    }

    if (gltf_sampler.m_MinFilter >= 0) {
        printf("    m_MinFilter: %d\n", gltf_sampler.m_MinFilter);
    }
}

void print_gltf_samplers(gltfSampler* gltf_samplers, int numSamplers)
{
    printf("Samplers:\n");

    for (int i = 0; i < numSamplers; ++i) {
        print_gltf_sampler(gltf_samplers[i]);
        printf("\n");
    }
}

void print_gltf_buffer(gltfBuffer gltf_buffer)
{
    if (gltf_buffer.m_ByteLength >= 0) {
        printf("    m_ByteLength: %d\n", gltf_buffer.m_ByteLength);
    }

    printf("    m_URI: %s\n", gltf_buffer.m_URI);
}

void print_gltf_buffers(gltfBuffer* gltf_buffers, int numBuffers)
{
    printf("Buffers:\n");

    for (int i = 0; i < numBuffers; ++i) {
        print_gltf_buffer(gltf_buffers[i]);
        printf("\n");
    }
}

#endif