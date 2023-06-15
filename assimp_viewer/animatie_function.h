#pragma once

#include <animation.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <bone.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>







float m_CurrentTime;
float m_DeltaTime;

std::vector<glm::mat4> m_FinalBoneMatrices;


void initAnimator(Animation* animation)
{
    m_CurrentTime = 0.0;
    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}


/*
void PlayAnimation(Animator* animator, Animation* pAnimation)
{
    animator->m_CurrentAnimation = pAnimation;
    animator->m_CurrentTime = 0.0f;
}
*/


/*



CalculateBoneTransform(Node* node, glm::mat4 parentTransform) {
    
    glm::mat4 transform = node->transform * parentTransform;

    if (node == bone) {
        glm::mat4 animationTransfom = getTransform(node.name, time);
        
        transform = animationTransform * node.offset * parentTransform;
    }

    foreach(node.child)
        CalculateBoneTransform(node.child, transform)
}


int main() {

    Node* skeletonRoot;
    int time;


    CalcluateBoneTransform(skeletonRoot, glm::mat4(1.0f));
}


*/

void CalculateBoneTransform(Animation* animation, SkeletonNode* node) {
    
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;
    glm::mat4 offset = glm::mat4(1.0f);

    if (bone) {
        nodeTransform = GetBoneTransform(node->name, currentTime);
        offset = node->offset;


        animation->m_FinalBoneMatrices[index] = globalTransformation * offset;



    }
}

void CalculateBoneTransform(Animation* animation, const AssimpNodeData* node, glm::mat4 parentTransform)
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
    m_DeltaTime = dt;

    m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;

    m_CurrentTime = fmod(animator->m_CurrentTime, animator->m_CurrentAnimation->m_Duration);

    CalculateBoneTransform(animator, &animator->m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
}