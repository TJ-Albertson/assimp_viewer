#ifndef ANIMATE_FUNCTION
#define ANIMATE_FUNCTION

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>

#include <animation.h>
#include <skeleton.h>

float m_CurrentTime;
float m_DeltaTime;

void CalculateNodeTransform(Animation animation, SkeletonNode* node, glm::mat4* FinalBoneMatrix, glm::mat4 parentTransform)
{
    glm::mat4 nodeTransform = node->m_Transformation;

    bool isBoneNode = (node->id >= 0);
    if (isBoneNode) {
        nodeTransform = FindBoneAndGetTransform(animation, node->m_NodeName, m_CurrentTime);
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    if (isBoneNode) {
        glm::mat4 finalBoneMatrix = globalTransformation * node->m_Offset;
        FinalBoneMatrix[node->id] = finalBoneMatrix;
    }

    for (int i = 0; i < node->m_NumChildren; ++i) {
        CalculateNodeTransform(animation, node->m_Children[i], FinalBoneMatrix, globalTransformation);
    }
}

void AnimateModel(float dt, Animation animation, SkeletonNode* rootNode, glm::mat4* FinalBoneMatrix)
{
    m_DeltaTime = dt;
    m_CurrentTime += animation.m_TicksPerSecond * dt;
    m_CurrentTime = fmod(m_CurrentTime, animation.m_Duration);

    CalculateNodeTransform(animation, rootNode, FinalBoneMatrix, glm::mat4(1.0f));
}

#endif