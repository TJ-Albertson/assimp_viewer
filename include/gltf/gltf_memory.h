/*-------------------------------------------------------------------------------\
gltf_memory.h

functions for freeing memory
\-------------------------------------------------------------------------------*/
#ifndef GLTF_MEMORY_H
#define GLTF_MEMORY_H

#include "gltf/gltf_structures.h"

void freeBaseColorTexture(gltfBaseColorTexture* texture)
{
    if (texture) {
        free(texture);
    }
}

void freeMetallicRoughnessTexture(gltfMetallicRoughnessTexture* texture)
{
    if (texture) {
        free(texture);
    }
}

void freeMetallicRoughness(gltfMetallicRoughness* metallicRoughness)
{
    if (metallicRoughness) {
        freeBaseColorTexture(metallicRoughness->m_BaseColorTexture);
        free(metallicRoughness);
    }
}

void freeNormalTexture(gltfNormalTexture* normalTexture)
{
    if (normalTexture) {
        free(normalTexture);
    }
}

void freeOcclusionTexture(gltfOcclusionTexture* occlusionTexture)
{
    if (occlusionTexture) {
        free(occlusionTexture);
    }
}

void freeEmissiveTexture(gltfEmissiveTexture* emissiveTexture)
{
    if (emissiveTexture) {
        free(emissiveTexture);
    }
}

void gltf_free_material(gltfMaterial material)
{
    free(material.m_MetalicRoughness);
    freeNormalTexture(material.m_NormalTexture);
    freeOcclusionTexture(material.m_OcclusionTexture);
    freeEmissiveTexture(material.m_EmissiveTexture);
}

void gltf_free_materials(gltfMaterial* materials, int numMaterials) {

    for (int i = 0; i < numMaterials; ++i)
    {
        gltf_free_material(materials[i]);
    }

    free(materials);
}

void gltf_free_meshes(gltfMesh* meshes, int numMeshes)
{
    for (int i = 0; i < numMeshes; ++i)
    {
        free(meshes[i].m_Primitives);
    }

    free(meshes);
}


#endif
