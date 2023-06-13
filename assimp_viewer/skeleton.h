#ifndef SKELETON_H
#define SKELETON_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> BoneNames;

int nodeId = 0;

aiNode* aiRootNode;

struct SkeletonBone {
    std::string mName;
    int id;
    int mNumChildren;
    std::vector<SkeletonBone> children;
};

void BoneCheckRoot(aiNode* node, const aiScene* scene);
void BoneCheck(aiNode* node, const aiScene* scene);
void BoneCheckParents(aiNode* boneNode, aiNode* meshNode);

void CreateSkeleton(const aiNode* node, SkeletonBone skeleBone);

int main()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/objects/vampire/dancing_vampire.dae", aiProcess_Triangulate);

    // Used in BoneCheck to find Node associated with Bone
    aiRootNode = scene->mRootNode;

    BoneCheckRoot(scene->mRootNode, scene);

    //for (int i = 0; i < BoneNames.size(); i++)
        //std::cout << BoneNames[i] << std::endl;

    SkeletonBone rootBone;

    CreateSkeleton(scene->mRootNode, rootBone);

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

int isStringInBoneVectorAndIndex(const std::string& target) {
    int i = 0;
    for (const auto& str : BoneNames) {
        if (str == target) {
            return i;
        }
        i++;
    }
    return -1;
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
            BoneCheckParents(boneNode->mParent, meshNode);
        }
        else {
            BoneNames.push_back(boneNode->mParent->mName.C_Str());
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
            }
        }
    }
    else {
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
                    aiNode* boneNode = aiRootNode->FindNode(boneNodeName);
                    BoneCheckParents(boneNode, node);
                }
            }
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        BoneCheck(node->mChildren[i], scene);
    }
}

void FindSkeletonRoot();
// idea. same as bonecheck/bonecheckroot

// this is bad. after first skeleton node all following are bones
void CreateSkeleton(const aiNode* node, SkeletonBone skeleBone) {

    int index = isStringInBoneVectorAndIndex(node->mName.C_Str());

    if (index > 0) {

        skeleBone.mName = node->mName.data;
        skeleBone.mNumChildren = node->mNumChildren;
        skeleBone.id = index;

        std::cout << skeleBone.mName << " " << skeleBone.id << std::endl;

        for (int i = 0; i < node->mNumChildren; i++) {

            SkeletonBone newBone;

            CreateSkeleton(node->mChildren[i], newBone);

            skeleBone.children.push_back(newBone);
        }
    }
}



#endif