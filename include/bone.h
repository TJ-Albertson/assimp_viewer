#pragma once

/* Container for bone data */

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <list>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <assimp_glm_helpers.h>
#include <glm/gtx/quaternion.hpp>


typedef struct {
    unsigned int m_ID;
    glm::mat4 m_LocalTransform;
    glm::mat4 m_OffsetMatrix;
} Bone;

Bone* CreateBone(const std::string& name, int ID, const aiNodeAnim* channel); 
void UpdateBone(Bone* bone, float animationTime);
int GetPositionIndex(float animationTime);
int GetRotationIndex(float animationTime);
int GetScaleIndex(float animationTime);
float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
glm::mat4 InterpolatePosition(float animationTime);
glm::mat4 InterpolateRotation(float animationTime);
glm::mat4 InterpolateScaling(float animationTime);

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

struct BoneChannel {
    glm::mat4 m_LocalTransform;
};

struct animationNode {
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;
};

struct BoneAnimation {
    KeyPosition* m_Positions;   // 8
    KeyRotation* m_Rotations;
    KeyScale* m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;
};

struct abrsuh {
    int positionIndex;
    int rotationIndex;
    int scaleIndex;
    float scaleFactor;
};

Bone* CreateBone(const std::string& name, int ID, const aiNodeAnim* channel)
{
    Bone bone;

    bone.m_ID = ID;
    bone.m_LocalTransform = glm::mat4(1.0f);

    BoneAnimation boneAnimation;





    int m_NumPositions = channel->mNumPositionKeys;

    boneAnimation.m_NumPositions = m_NumPositions;

    KeyPosition* m_Positions;

    m_Positions = (struct KeyPosition*)malloc(m_NumPositions * sizeof(struct KeyPosition));
    

    for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
        data.timeStamp = timeStamp;
        m_Positions[positionIndex] = data;
    }

    boneAnimation.m_Positions = m_Positions;





    int m_NumRotations = channel->mNumRotationKeys;

    boneAnimation.m_NumRotations = m_NumRotations;

    KeyRotation* m_Rotations;

    m_Rotations = (struct KeyRotation*)malloc(m_NumRotations * sizeof(struct KeyRotation));

    for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_Rotations[rotationIndex] = data;
    }

    boneAnimation.m_Rotations = m_Rotations;



    int m_NumScalings = channel->mNumScalingKeys;

    boneAnimation.m_NumScalings = m_NumScalings;

    KeyScale* m_Scales;

    m_Scales = (struct KeyScale*)malloc(m_NumRotations * sizeof(struct KeyScale));

    for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale data;
        data.scale = AssimpGLMHelpers::GetGLMVec(scale);
        data.timeStamp = timeStamp;
        m_Scales[keyIndex] = data;
    }

    boneAnimation.m_Scales = m_Scales;

    return &bone;
}





void UpdateBone(Bone* boneNode, float animationTime)
{
    glm::mat4 translation = InterpolatePosition(animationTime);

    glm::mat4 rotation = InterpolateRotation(animationTime);

    glm::mat4 scale = InterpolateScaling(animationTime);

    boneNode->m_LocalTransform = translation * rotation * scale;
}



struct GetIndexIn {
    int m_IndexCount;   // 4 bytes
    float time;         // 4 bytes
};

struct GetIndexOut {
    int index;
};

//

int GetPositionIndex(animationNode* animationNode, float animationTime)
{
    for (int index = 0; index < animationNode->m_NumPositions - 1; ++index) {

        if (animationTime < animationNode->m_Positions[index + 1].timeStamp)
            return index;
    }
}

// get position/rotation/scale based on time
int GetPositionIndex(animationNode* animationNode, float animationTime)
{
    for (int index = 0; index < animationNode->m_NumPositions - 1; ++index) {
        if (animationTime < animationNode->m_Positions[index + 1].timeStamp)
            return index;
    }
}

int GetRotationIndex(animationNode* animationNode, float animationTime)
{
    for (int index = 0; index < animationNode->m_NumRotations - 1; ++index) {
        if (animationTime < animationNode->m_Rotations[index + 1].timeStamp)
            return index;
    }
}

int GetScaleIndex(animationNode* animationNode, float animationTime)
{
    for (int index = 0; index < animationNode->m_NumScalings - 1; ++index) {
        if (animationTime < animationNode->m_Scales[index + 1].timeStamp)
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

glm::mat4 InterpolatePosition(animationNode* animationNode, float animationTime)
{
    if (1 == animationNode->m_NumPositions)
        return glm::translate(glm::mat4(1.0f), animationNode->m_Positions[0].position);

    int p0Index = GetPositionIndex(animationTime);

    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(animationNode->m_Positions[p0Index].timeStamp, animationNode->m_Positions[p1Index].timeStamp, animationTime);
    
    glm::vec3 finalPosition = glm::mix(animationNode->m_Positions[p0Index].position, animationNode->m_Positions[p1Index].position, scaleFactor);
    
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 InterpolateRotation(animationNode* animationNode, float animationTime)
{
    if (1 == animationNode->m_NumRotations) {
        auto rotation = glm::normalize(animationNode->m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(animationNode->m_Rotations[p0Index].timeStamp, animationNode->m_Rotations[p1Index].timeStamp, animationTime);
    
    glm::quat finalRotation = glm::slerp(animationNode->m_Rotations[p0Index].orientation, animationNode->m_Rotations[p1Index].orientation, scaleFactor);
    
    finalRotation = glm::normalize(finalRotation);
    
    return glm::toMat4(finalRotation);
}

glm::mat4 InterpolateScaling(animationNode* animationNode, float animationTime)
{
    if (1 == animationNode->m_NumScalings)
        return glm::scale(glm::mat4(1.0f), animationNode->m_Scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    
    int p1Index = p0Index + 1;
    
    float scaleFactor = GetScaleFactor(animationNode->m_Scales[p0Index].timeStamp,
    
    animationNode->m_Scales[p1Index].timeStamp, animationTime);

    glm::vec3 finalScale = glm::mix(animationNode->m_Scales[p0Index].scale, animationNode->m_Scales[p1Index].scale, scaleFactor);
    
    return glm::scale(glm::mat4(1.0f), finalScale);
}