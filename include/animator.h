#pragma once

#include <animation.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <bone.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>

// previous private

// this should dole the current time and animate all 


typedef struct {
    std::vector<glm::mat4> m_FinalBoneMatrices;
    float m_CurrentTime;
    float m_DeltaTime;
    Animation* m_CurrentAnimation;
} Animator;

void CalculateBoneTransform(Animator* animator, const AssimpNodeData* node, glm::mat4 parentTransform);

Animator* CreateAnimator(Animation* animation)
{
    Animator* animator;

    animator->m_CurrentTime = 0.0;
    animator->m_CurrentAnimation = animation;
    animator->m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
        animator->m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

    return animator;
}


void PlayAnimation(Animator* animator, Animation* pAnimation)
{
    animator->m_CurrentAnimation = pAnimation;
    animator->m_CurrentTime = 0.0f;
}


//update all currently playing animations

animate(const AssimpNodeData* boneNode, glm::mat4 parentTransform) {

    UpdateBone(Bone, animator->m_CurrentTime);

    glm::mat4 transfrom = Bone.localtransform * parentTransform;

    m_FinalBoneMatricessform[Bone.id] = transfrom * Bone.offset;

    for(int i = 0; i < Bone.nu)
        animate(bone.child, transfrom)
}


//multiple skeletons




void CalculateBoneTransform(Animator* animator, const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* Bone = FindBone(nodeName);

    if (Bone) {
        UpdateBone(Bone, animator->m_CurrentTime);
        nodeTransform = Bone->m_LocalTransform;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = animator->m_CurrentAnimation->m_BoneInfoMap;

    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {

        int index = boneInfoMap[nodeName].id;

        glm::mat4 offset = boneInfoMap[nodeName].offset;

        animator->m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(animator, &node->children[i], globalTransformation);
}


void UpdateAnimation(Animator* animator, float dt)
{
    animator->m_DeltaTime = dt;

    if (animator->m_CurrentAnimation) {

        animator->m_CurrentTime += animator->m_CurrentAnimation->m_TicksPerSecond * dt;

        animator->m_CurrentTime = fmod(animator->m_CurrentTime, animator->m_CurrentAnimation->m_Duration);

        CalculateBoneTransform(animator, &animator->m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
    }
}