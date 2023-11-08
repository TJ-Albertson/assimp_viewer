/*-------------------------------------------------------------------------------\
bone_animation.h

Functions:
    get transformation matrix for bone in animation based on time

\-------------------------------------------------------------------------------*/

#ifndef BONE_ANIMATION_H
#define BONE_ANIMATION_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

struct BoneAnimationChannel {
    char* m_NodeName;

    KeyPosition* m_Positions;
    KeyRotation* m_Rotations;
    KeyScale* m_Scales;
    
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;
};

glm::mat4 InterpolatePosition(BoneAnimationChannel* boneAnimationChannel, float animationTime);
glm::mat4 InterpolateRotation(BoneAnimationChannel* boneAnimationChannel, float animationTime);
glm::mat4 InterpolateScaling(BoneAnimationChannel* boneAnimationChannel, float animationTime);

int GetPositionIndex(BoneAnimationChannel* boneAnimationChannel, float animationTime);
int GetRotationIndex(BoneAnimationChannel* boneAnimationChannel, float animationTime);
int GetScaleIndex(BoneAnimationChannel* boneAnimationChannel, float animationTime);


glm::mat4 getBoneAnimationTransformation(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    glm::mat4 translation = InterpolatePosition(boneAnimationChannel, animationTime);

    glm::mat4 rotation = InterpolateRotation(boneAnimationChannel, animationTime);

    glm::mat4 scale = InterpolateScaling(boneAnimationChannel, animationTime);

    return translation * rotation * scale;
}

// get position/rotation/scale based on time
int GetPositionIndex(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    for (int index = 0; index < boneAnimationChannel->m_NumPositions - 1; ++index) {
        if (animationTime < boneAnimationChannel->m_Positions[index + 1].timeStamp)
            return index;
    }
}

int GetRotationIndex(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    for (int index = 0; index < boneAnimationChannel->m_NumRotations - 1; ++index) {
        if (animationTime < boneAnimationChannel->m_Rotations[index + 1].timeStamp)
            return index;
    }
}

int GetScaleIndex(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    for (int index = 0; index < boneAnimationChannel->m_NumScalings - 1; ++index) {
        if (animationTime < boneAnimationChannel->m_Scales[index + 1].timeStamp)
            return index;
    }
}


float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;

    float midWayLength = animationTime - lastTimeStamp;

    float framesDiff = nextTimeStamp - lastTimeStamp;

    scaleFactor = midWayLength / framesDiff;

    return scaleFactor;
}


glm::mat4 InterpolatePosition(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    if (boneAnimationChannel->m_NumPositions == 1)
        return glm::translate(glm::mat4(1.0f), boneAnimationChannel->m_Positions[0].position);

    int p0Index = GetPositionIndex(boneAnimationChannel, animationTime);

    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(boneAnimationChannel->m_Positions[p0Index].timeStamp, boneAnimationChannel->m_Positions[p1Index].timeStamp, animationTime);

    glm::vec3 finalPosition = glm::mix(boneAnimationChannel->m_Positions[p0Index].position, boneAnimationChannel->m_Positions[p1Index].position, scaleFactor);

    return glm::translate(glm::mat4(1.0f), finalPosition);
}
// return glm::translate(glm::mat4(1.0f), boneAnimationChannel->m_Positions[p0Index].position);

glm::mat4 InterpolateRotation(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    if (1 == boneAnimationChannel->m_NumRotations) {
        auto rotation = glm::normalize(boneAnimationChannel->m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(boneAnimationChannel, animationTime);

    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(boneAnimationChannel->m_Rotations[p0Index].timeStamp, boneAnimationChannel->m_Rotations[p1Index].timeStamp, animationTime);

    glm::quat finalRotation = glm::slerp(boneAnimationChannel->m_Rotations[p0Index].orientation, boneAnimationChannel->m_Rotations[p1Index].orientation, scaleFactor);

    finalRotation = glm::normalize(finalRotation);

    //return glm::toMat4(glm::normalize(boneAnimationChannel->m_Rotations[p0Index].orientation));
    return glm::toMat4(finalRotation);
}

glm::mat4 InterpolateScaling(BoneAnimationChannel* boneAnimationChannel, float animationTime)
{
    if (1 == boneAnimationChannel->m_NumScalings)
        return glm::scale(glm::mat4(1.0f), boneAnimationChannel->m_Scales[0].scale);

    int p0Index = GetScaleIndex(boneAnimationChannel, animationTime);

    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(boneAnimationChannel->m_Scales[p0Index].timeStamp,

    boneAnimationChannel->m_Scales[p1Index].timeStamp, animationTime);

    glm::vec3 finalScale = glm::mix(boneAnimationChannel->m_Scales[p0Index].scale, boneAnimationChannel->m_Scales[p1Index].scale, scaleFactor);

    //return glm::scale(glm::mat4(1.0f), boneAnimationChannel->m_Scales[p0Index].scale);
    return glm::scale(glm::mat4(1.0f), finalScale);
}


glm::mat4 BlendInterpolatePosition(BoneAnimationChannel* boneAnimationChannel_1, BoneAnimationChannel* boneAnimationChannel_2, float animationTime, float blend)
{
    int bone1_p0Index = GetPositionIndex(boneAnimationChannel_1, animationTime);

    int bone2_p0Index = GetPositionIndex(boneAnimationChannel_2, animationTime);

    int bone1_p1Index = bone1_p0Index + 1;

    int bone2_p1Index = bone2_p0Index + 1;

    glm::vec3 blendPosition_1 = glm::mix(boneAnimationChannel_1->m_Positions[bone1_p0Index].position, boneAnimationChannel_2->m_Positions[bone2_p0Index].position, blend);

    glm::vec3 blendPosition_2 = glm::mix(boneAnimationChannel_1->m_Positions[bone1_p1Index].position, boneAnimationChannel_2->m_Positions[bone2_p1Index].position, blend);



    float scaleFactor = GetScaleFactor(boneAnimationChannel_1->m_Positions[bone1_p1Index].timeStamp, boneAnimationChannel_2->m_Positions[bone2_p1Index].timeStamp, animationTime);

    glm::vec3 finalPosition = glm::mix(boneAnimationChannel_1->m_Positions[bone1_p1Index].position, boneAnimationChannel_2->m_Positions[bone2_p1Index].position, scaleFactor);

    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 BlendInterpolateScaling(BoneAnimationChannel* boneAnimationChannel_1, BoneAnimationChannel* boneAnimationChannel_2, float animationTime)
{
    int bone1_p0Index = GetScaleIndex(boneAnimationChannel_1, animationTime);

    int bone2_p0Index = GetScaleIndex(boneAnimationChannel_2, animationTime);

    // maybe schange based on speed?
    float blendScaleFactor = GetScaleFactor(boneAnimationChannel_1->m_Scales[bone1_p0Index].timeStamp, boneAnimationChannel_2->m_Scales[bone2_p0Index].timeStamp, animationTime);

    glm::vec3 blendScale = glm::mix(boneAnimationChannel_1->m_Scales[bone1_p0Index].scale, boneAnimationChannel_2->m_Scales[bone2_p0Index].scale, blendScaleFactor);



    int bone1_p1Index = bone1_p0Index + 1;

    int bone2_p1Index = bone2_p0Index + 1;

    float lerpScaleFactor = GetScaleFactor(boneAnimationChannel_1->m_Scales[bone1_p1Index].timeStamp, boneAnimationChannel_2->m_Scales[bone2_p1Index].timeStamp, animationTime);

    glm::vec3 finalScale = glm::mix(boneAnimationChannel_1->m_Scales[bone1_p1Index].scale, boneAnimationChannel_2->m_Scales[bone2_p1Index].scale, lerpScaleFactor);

    return glm::scale(glm::mat4(1.0f), finalScale);
}



glm::mat4 BlendInterpolateRotation(BoneAnimationChannel* boneAnimationChannel_1, BoneAnimationChannel* boneAnimationChannel_2, float animationTime)
{
    int bone1_p0Index = GetRotationIndex(boneAnimationChannel_1, animationTime);

    int bone2_p0Index = GetRotationIndex(boneAnimationChannel_2, animationTime);

    // maybe schange based on speed?
    float blendScaleFactor = GetScaleFactor(boneAnimationChannel_1->m_Rotations[bone1_p0Index].timeStamp, boneAnimationChannel_2->m_Rotations[bone2_p0Index].timeStamp, animationTime);

    glm::quat blendRotation = glm::slerp(boneAnimationChannel_1->m_Rotations[bone1_p0Index].orientation, boneAnimationChannel_2->m_Rotations[bone2_p0Index].orientation, blendScaleFactor);



    int bone1_p1Index = bone1_p0Index + 1;

    int bone2_p1Index = bone2_p0Index + 1;

    float lerpScaleFactor = GetScaleFactor(boneAnimationChannel_1->m_Rotations[bone1_p1Index].timeStamp, boneAnimationChannel_2->m_Rotations[bone2_p1Index].timeStamp, animationTime);

    glm::quat finalRotation = glm::slerp(boneAnimationChannel_1->m_Rotations[bone1_p1Index].orientation, boneAnimationChannel_2->m_Rotations[bone2_p1Index].orientation, lerpScaleFactor);

    return glm::toMat4(finalRotation);
}

#endif