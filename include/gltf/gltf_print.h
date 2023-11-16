/*-------------------------------------------------------------------------------\
gltf_print.h

print functions
\-------------------------------------------------------------------------------*/
#ifndef GLTF_PRINT_H
#define GLTF_PRINT_H

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

void print_gltf_scene(gltfScene gltf_scene)
{

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
void print_baseColorTexture(gltfBaseColorTexture* texture)
{
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

void print_metallic_roughness(gltfMetallicRoughness* gltfMetallicRoughness)
{

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
void print_gltf_primitive_attributes(gltfPrimitiveAttributes gltf_primitive_attributes)
{
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

void print_gltf_primitive(gltfPrimitive gltf_primitive)
{

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

void print_gltf_mesh(gltfMesh gltf_mesh)
{
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

void print_gltf_texture(gltfTexture gltf_texture)
{
    if (gltf_texture.m_SamplerIndex >= 0) {
        printf("        m_SamplerIndex: %d\n", gltf_texture.m_SamplerIndex);
    }

    if (gltf_texture.m_SourceIndex >= 0) {
        printf("        m_SourceIndex: %d\n", gltf_texture.m_SourceIndex);
    }
    printf("\n");
}

void print_gltf_textures(gltfTexture* gltf_texture, int numTextures)
{

    printf("Textures:\n");

    for (int i = 0; i < numTextures; ++i) {
        print_gltf_texture(gltf_texture[i]);
    }
}

void print_gltf_image(gltfImage gltf_image)
{
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