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
} gltfMaterial;






void print_gltf_scene(gltfScene gltf_scene);
void print_gltf_meshes(gltfMesh* gltf_meshes, int numMeshes);
void print_gltf_textures(gltfTexture* gltf_texture, int numTextures);
void print_gltf_images(gltfImage* gltf_images, int numImages);
void print_gltf_accessors(gltfAccessor* gltf_accessors, int numAccessors);

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

        gltfAccessors[i] = gltf_process_bufferView(bufferView);
    }

    print_gltf_bufferViews(gltfAccessors, numAccessors);
    
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

#endif