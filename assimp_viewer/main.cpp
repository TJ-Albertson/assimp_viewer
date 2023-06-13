#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> BoneNames;

aiNode* aiRootNode;

void BoneCheckRoot(aiNode* node, const aiScene* scene);
void BoneCheck(aiNode* node, const aiScene* scene);
void BoneCheckParents(aiNode* boneNode, aiNode* meshNode);

int main()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/objects/vampire/dancing_vampire.dae", aiProcess_Triangulate);

    // GetBoneNames(*scene);

    // std::cout << "Hello World" << std::endl;
    aiRootNode = scene->mRootNode;

    BoneCheckRoot(scene->mRootNode, scene);

    return 1;
}


bool isStringInBoneVector(const std::string& target) {
    for (const auto& str : BoneNames) {
        if (str == target) {
            return true;
        }
    }
    return false;
}

void BoneCheckParents(aiNode* boneNode, aiNode* meshNode)
{
    // if node.parent == mesh node or mesh node.parent then skip
    aiString boneParentName = boneNode->mParent->mName;

    //need new clause to stop if bone already marked
    if (!isStringInBoneVector(boneParentName.C_Str())) {
        if (boneParentName != meshNode->mName && boneParentName != meshNode->mParent->mName)
        {
            BoneNames.push_back(boneNode->mParent->mName.C_Str());
            std::cout << "-> " << boneNode->mParent->mName.C_Str() << std::endl;
            BoneCheckParents(boneNode->mParent, meshNode);
        }
    }
    
}

void BoneCheckRoot(aiNode* node, const aiScene* scene)
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
        BoneCheck(node->mChildren[i], scene);
    }
}

void BoneCheck(aiNode* node, const aiScene* scene)
{
    if (node->mNumMeshes > 0) {

        for (int i = 0; i < node->mNumMeshes; i++) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; j++) {

                aiString boneNodeName = scene->mMeshes[meshIndex]->mBones[j]->mName;

                if (!isStringInBoneVector(boneNodeName.C_Str())) {
                    BoneNames.push_back(boneNodeName.C_Str());
                    std::cout << boneNodeName.C_Str() << std::endl;
                    aiNode* boneNode = aiRootNode->FindNode(boneNodeName);

                    BoneCheckParents(boneNode, node);
                }
            }
        }
    } else {
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        BoneCheck(node->mChildren[i], scene);
    }
}
