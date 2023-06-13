#pragma once

#include <assimp/scene.h>
#include <functional>
#include <glm/glm.hpp>
// #include <animdata.h>
#include <bone.h>
#include <map>
#include <model_animation.h>
#include <vector>

struct Node {
    std::string m_Name;
    glm::mat4 m_Tansformation;
    int childrenCount;
    std::vector<Node> children;
};

struct SkeletonBone {
    std::string name;
    glm::mat4 transformation;
    int childrenCount;
    std::vector<SkeletonBone> children;
};


typedef struct {
    float m_Duration;
    int m_TicksPerSecond;
    Node m_RootNode;
} Animation;

void CopyNodeTree(Node& dest, const aiNode* src);




std::vector<string> BoneNames;

Animation* CreateAnimation(const std::string& animationPath, Model* model)
{
    Animation anim;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    //assert(scene && scene->mRootNode);

    auto animation = scene->mAnimations[0];

    anim.m_Duration = animation->mDuration;
    anim.m_TicksPerSecond = animation->mTicksPerSecond;
    
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;

    globalTransformation = globalTransformation.Inverse();

    CopyNodeTree(anim.m_RootNode, scene->mRootNode);

    return &anim;
}


void GetBoneNames(aiScene scene) {
    for (int i = 0; i < scene.mNumMeshes; i++) {
        for (int j = 0; j < scene.mMeshes[i]->mNumBones; j++) {
            BoneNames.push_back(scene.mMeshes[i]->mBones[j]->mName.C_Str());
        }
    }
}


void CopyNodeTree(Node& dest, const aiNode* node)
{
    assert(node);

    dest.m_Name = node->mName.data;

    dest.m_Tansformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);

    dest.childrenCount = node->mNumChildren;

    for (int i = 0; i < node->mNumChildren; i++) {

        Node newData;

        CopyNodeTree(newData, node->mChildren[i]);

        dest.children.push_back(newData);
    }
}