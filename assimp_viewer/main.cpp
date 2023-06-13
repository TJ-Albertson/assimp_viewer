#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> BoneNames;

aiNode* aiRootNode;

void GetBoneNames(aiScene scene);
void CopyRoot(aiNode* node, const aiScene* scene);
void CopyNodesWithMeshes(aiNode* node, const aiScene* scene);

int main()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("C:/Users/tjalb/source/repos/game/resources/objects/vampire/dancing_vampire.dae", aiProcess_Triangulate);

    // GetBoneNames(*scene);

    // std::cout << "Hello World" << std::endl;
    aiRootNode = scene->mRootNode;

    CopyRoot(scene->mRootNode, scene);

    return 1;
}

void GetBoneNames(const aiScene scene)
{
    for (int i = 0; i < scene.mNumMeshes; i++) {

        if (scene.mMeshes[i]->mNumBones == 0)
            continue;

        for (int j = 0; j < scene.mMeshes[i]->mNumBones; j++) {

            BoneNames.push_back(scene.mMeshes[i]->mBones[j]->mName.C_Str());
            std::cout << scene.mMeshes[i]->mBones[j]->mName.C_Str() << std::endl;
        }
    }
}


void CheckParents(aiNode* boneNode, aiNode* meshNode)
{
    // if node.parent == mesh node or mesh node.parent then skip
    aiString boneParentName = boneNode->mParent->mName;

    //need new clause to stop if bone already marked

    if (boneParentName != meshNode->mName && boneParentName != meshNode->mParent->mName)
    {
        BoneNames.push_back(boneNode->mParent->mName.C_Str());
        std::cout << "-> " << boneNode->mParent->mName.C_Str() << std::endl;
        CheckParents(boneNode->mParent, meshNode);
    }
    
    
}

void CopyRoot(aiNode* node, const aiScene* scene)
{
    if (node->mNumMeshes > 0) {

        for (int i = 0; i < node->mNumMeshes; i++) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; j++) {

                BoneNames.push_back(scene->mMeshes[meshIndex]->mBones[j]->mName.C_Str());
                std::cout << scene->mMeshes[meshIndex]->mBones[j]->mName.C_Str() << std::endl;
            }
        }
    } else {
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        CopyNodesWithMeshes(node->mChildren[i], scene);
    }
}

void CopyNodesWithMeshes(aiNode* node, const aiScene* scene)
{
    if (node->mNumMeshes > 0) {

        for (int i = 0; i < node->mNumMeshes; i++) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; j++) {

                aiString boneNodeName = scene->mMeshes[meshIndex]->mBones[j]->mName;

                BoneNames.push_back(boneNodeName.C_Str());
                std::cout << boneNodeName.C_Str() << std::endl;

                aiNode* boneNode = aiRootNode->FindNode(boneNodeName);

                //std::cout << "Mesh Name " << scene->mMeshes[meshIndex]->mName.C_Str() << std::endl;

                CheckParents(boneNode, node);
            }
        }
    } else {
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        CopyNodesWithMeshes(node->mChildren[i], scene);
    }
}
