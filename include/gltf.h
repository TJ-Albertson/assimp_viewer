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
        strncpy(gltf_buffer.m_URI, "null", sizeof(gltf_buffer.m_URI));
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
        cJSON* emmissiveFactor = cJSON_GetObjectItem(materialNode, "emissiveFactor");

        float x = cJSON_GetArrayItem(emmissiveFactor, 0)->valuedouble;
        float y = cJSON_GetArrayItem(emmissiveFactor, 1)->valuedouble;
        float z = cJSON_GetArrayItem(emmissiveFactor, 2)->valuedouble;

        gltf_material.m_EmissiveFactor = glm::vec3(x, y, z);
    } else {
        gltf_material.m_EmissiveFactor = glm::vec3(0.0f);
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

gltfNode gltf_traverse_node(cJSON* node)
{
    gltfNode gltf_node = gltf_process_node(node);

    if (cJSON_GetObjectItem(node, "children")) {
        cJSON* children = cJSON_GetObjectItem(node, "children");
        int numChildren = cJSON_GetArraySize(children);

        gltf_node.m_NumChildren = numChildren;
        gltf_node.m_Children = (gltfNode*)malloc(numChildren * sizeof(gltfNode));

        for (int i = 0; i < numChildren; ++i) {
            cJSON* child = cJSON_GetArrayItem(children, i);

            gltf_node.m_Children[i] = gltf_traverse_node(child);
        }
    } else {
        gltf_node.m_NumChildren = -1;
    }

    return gltf_node;
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

    //print_gltf_materials(gltfMaterials, numMaterials);

    // "meshes": []
    cJSON* meshes = cJSON_GetObjectItem(root, "meshes");
    int numMeshes = cJSON_GetArraySize(meshes);

    gltfMesh* gltfMeshes = (gltfMesh*)malloc(numMeshes * sizeof(gltfMesh));

    for (int i = 0; i < numMeshes; ++i) {
        cJSON* mesh = cJSON_GetArrayItem(meshes, i);

        gltfMeshes[i] = gltf_process_mesh(mesh);
    }

    //print_gltf_meshes(gltfMeshes, numMeshes);




    // "textures": []
    cJSON* textures = cJSON_GetObjectItem(root, "textures");
    int numTextures = cJSON_GetArraySize(textures);

    gltfTexture* gltfTextures = (gltfTexture*)malloc(numTextures * sizeof(gltfTexture));

    for (int i = 0; i < numTextures; ++i) {
        cJSON* texture = cJSON_GetArrayItem(textures, i);

        gltfTextures[i] = gltf_process_texture(texture);
    }

    //print_gltf_textures(gltfTextures, numTextures);

    // "images": []
    cJSON* images = cJSON_GetObjectItem(root, "images");
    int numImages = cJSON_GetArraySize(images);

    gltfImage* gltfImages = (gltfImage*)malloc(numImages * sizeof(gltfImage));
        
    for (int i = 0; i < numImages; ++i) {
        cJSON* texture = cJSON_GetArrayItem(images, i);

        gltfImages[i] = gltf_process_image(texture);
    }

    //print_gltf_images(gltfImages, numImages);

    // "accessors": []
    cJSON* accessors = cJSON_GetObjectItem(root, "accessors");
    int numAccessors = cJSON_GetArraySize(accessors);

    gltfAccessor* gltfAccessors = (gltfAccessor*)malloc(numAccessors * sizeof(gltfAccessor));

    for (int i = 0; i < numAccessors; ++i) {
        cJSON* accesssor = cJSON_GetArrayItem(accessors, i);

        gltfAccessors[i] = gltf_process_acccessor(accesssor);
    }

    //print_gltf_accessors(gltfAccessors, numAccessors);

    // "bufferViews: []
    cJSON* bufferViews = cJSON_GetObjectItem(root, "bufferViews");
    int numbufferViews = cJSON_GetArraySize(accessors);

    gltfBufferView* gltfBufferViews = (gltfBufferView*)malloc(numAccessors * sizeof(gltfBufferView));

    for (int i = 0; i < numAccessors; ++i) {
        cJSON* bufferView = cJSON_GetArrayItem(bufferViews, i);

        gltfBufferViews[i] = gltf_process_bufferView(bufferView);
    }

    //print_gltf_bufferViews(gltfBufferViews, numbufferViews);

    // "samplers: []
    cJSON* samplers = cJSON_GetObjectItem(root, "samplers");
    int numSamplers = cJSON_GetArraySize(samplers);

    gltfSampler* gltfSamplers = (gltfSampler*)malloc(numAccessors * sizeof(gltfSampler));

    for (int i = 0; i < numSamplers; ++i) {
        cJSON* sampler = cJSON_GetArrayItem(samplers, i);

        gltfSamplers[i] = gltf_process_sampler(sampler);
    }

    //print_gltf_samplers(gltfSamplers, numSamplers);

    // "buffers: []
    cJSON* buffers = cJSON_GetObjectItem(root, "buffers");
    int numBuffers = cJSON_GetArraySize(buffers);

    gltfBuffer* gltfBuffers = (gltfBuffer*)malloc(numBuffers * sizeof(gltfBuffer));

    for (int i = 0; i < numBuffers; ++i) {
        cJSON* buffer = cJSON_GetArrayItem(buffers, i);

        gltfBuffers[i] = gltf_process_buffer(buffer);
    }

   // print_gltf_buffers(gltfBuffers, numBuffers);



    // Buffer
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


    model.m_NumMeshes = numMeshes;
    model.m_NumMaterials = numMaterials;

    model.m_Meshes = (g_Mesh*)malloc(numMeshes * sizeof(g_Mesh));
    model.m_Materials = (Material*)malloc(numMaterials * sizeof(g_Mesh));

    for (int i = 0; i < numMaterials; ++i) 
    {
        model.m_Materials[i] = gltf_load_material(gltfMaterials[i], gltfImages, gltfSamplers, gltfTextures);
    }

    for (int i = 0; i < numMeshes; ++i) 
    {
        gltfMesh gltf_mesh = gltfMeshes[i];

        int numIndices;

        unsigned int VAO;
        glGenVertexArrays(1, &VAO);

        glBindVertexArray(VAO);

        for (int j = 0; j < gltf_mesh.m_NumPrimitives; ++j) 
        {
            gltfPrimitive gltf_primitive = gltf_mesh.m_Primitives[j];

            /*
             *   Indices
             */
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

                char* offsetBuffer = allocatedBuffers[bufferIndex] + offset;

                int componentSize = component_size(accessor.m_ComponentType);

                numIndices = count;
                unsigned int* indices = (unsigned int*)malloc(count * sizeof(unsigned int));

                for (int k = 0; k < count; ++k) {
                    unsigned short x;
                    memcpy(&x, offsetBuffer + k * componentSize, componentSize);

                    indices[k] = x;
                }
                
                unsigned int EBO;
                glGenBuffers(1, &EBO);
                   
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
            }

            /*
             *   Attributes
             */
            gltfPrimitiveAttributes attributes = gltf_primitive.m_Attributes;

            if (attributes.m_PositionIndex >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_PositionIndex];

                gltf_bind_attribute(POSITION, accessor, gltfBufferViews, allocatedBuffers);
            }

            if (attributes.m_NormalIndex >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_NormalIndex];

                gltf_bind_attribute(NORMAL, accessor, gltfBufferViews, allocatedBuffers);
            }

            if (attributes.m_TexCoord_0_Index >= 0) {
                gltfAccessor accessor = gltfAccessors[attributes.m_TexCoord_0_Index];

                gltf_bind_attribute(TEXCOORD, accessor, gltfBufferViews, allocatedBuffers);
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
                    memcpy(&x, buffer + k * componentSize, componentSize);
                    memcpy(&y, buffer + k * componentSize, componentSize);
                    memcpy(&z, buffer + k * componentSize, componentSize);
                    memcpy(&w, buffer + k * componentSize, componentSize);

                    printf("    tangent[%d]: %f %f %f %f\n", k, x, y, z, w);

                    tangents[k].x = x;
                    tangents[k].y = y;
                    tangents[k].z = z;
                    tangents[k].w = w;
                }
            }
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        model.m_Meshes[i].m_VAO = VAO;
        model.m_Meshes[i].m_NumIndices = numIndices;
    }


    free(allocatedBuffers[0]);
    free(allocatedBuffers);

    cJSON_Delete(root);

    return 1;
}

int LoadGLTF(const char* filename, g_Model& model)
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
        gltf_parse(jsonString, model);
        free(jsonString);
    }

    return 0;
}




#endif