#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


#include <BoneAnimationChannel.h>

struct Animation {
    std::string name;

    float m_Duration;
    int m_TicksPerSecond;

    unsigned int m_NumBoneAnimations;
    std::vector<BoneAnimationChannel> m_BoneAnimations;

};

glm::mat4 FindBoneAndGetTransform(Animation* animation, std::string boneNodeName, float animationTime)
{
    for (int i = 0; i < animation->m_NumBoneAnimations; i++) {
        if (animation->m_NumBoneAnimations[i].m_NodeName == boneNodeName) {
            return getBoneAnimationTransformation(animation->m_NumBoneAnimations[i], animationTime);
        }
    }
}

#endif