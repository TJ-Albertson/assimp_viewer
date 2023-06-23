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


/*
void PlayAnimation(Animator* animator, Animation* pAnimation)
{
    animator->m_CurrentAnimation = pAnimation;
    animator->m_CurrentTime = 0.0f;
}
*/
/*



void CalculateNodeTransform(Node* node, glm::mat4 parentTransform) {
   
    if (node == bone) {

        glm::mat4 animationTransfom = getTransform(node.name, time);
        
        transform = animationTransform * node.offset * parentTransform;

        foreach(node.child)
            CalculateBoneNodeTransform(node.child, transform)

    } else {

        glm::mat4 transform = node->transform * parentTransform;

        foreach(node.child)
            CalculateNodeTransform(node.child, transform)
    }
}

Void CalculateBoneNodeTransform(Node* node, glm::mat4 parentTransform) {
    
        glm::mat4 animationTransfom = getTransform(node.name, time);

        transform = animationTransform * node.offset * parentTransform;

        foreach(node.child)
            CalculateBoneNodeTransform(node.child, transform)
}





int main() {

    Node* skeletonRoot;
    int time;


    CalcluateBoneTransform(skeletonRoot, glm::mat4(1.0f));
}


*/

void CalculateBoneNodeTransform(Animation animation, SkeletonNode* node, glm::mat4* FinalBoneMatrix, glm::mat4 parentTransform);

void CalculateNodeTransform(Animation animation, SkeletonNode* node, glm::mat4* FinalBoneMatrix, glm::mat4 parentTransform)
{
    if (node->id > 0) {

        CalculateBoneNodeTransform(animation, node, FinalBoneMatrix, parentTransform);
           
    } else {

        glm::mat4 transform = parentTransform * node->m_Transformation;

        for (int i = 0; i < node->m_NumChildren; ++i)
            CalculateNodeTransform(animation, node->m_Children[i], FinalBoneMatrix, transform);
    }
}

void CalculateBoneNodeTransform(Animation animation, SkeletonNode* node, glm::mat4* FinalBoneMatrix, glm::mat4 parentTransform)
{
    glm::mat4 animationTransform = FindBoneAndGetTransform(animation, node->m_NodeName, m_CurrentTime);

    glm::mat4 transform = parentTransform * animationTransform * node->m_Offset;

    // Temp; Some bones are not associated with Mesh or Animation for some reason; I'm still trying to figure out
    if (node->id > 0)
        FinalBoneMatrix[node->id] = transform;

    for (int i = 0; i < node->m_NumChildren; ++i)
        CalculateBoneNodeTransform(animation, node->m_Children[i], FinalBoneMatrix, transform);
}



void AnimateModel(float dt, Animation animation, SkeletonNode* rootNode, glm::mat4* FinalBoneMatrix)
{

    


    m_DeltaTime = dt;
    m_CurrentTime += animation.m_TicksPerSecond * dt;
    m_CurrentTime = fmod(m_CurrentTime, animation.m_Duration);

   

    CalculateNodeTransform(animation, rootNode, FinalBoneMatrix, glm::mat4(1.0f));
}

#endif