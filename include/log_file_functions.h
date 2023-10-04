#ifndef LOG_FILE_FUNCTIONS
#define LOG_FILE_FUNCTIONS

#include <skeleton.h>
#include <model.h>
#include <scene_graph.h>

void LogVertices(int numVertices, VertexData* vertices);

void WriteSkeletonNodes(SkeletonNode* node, std::ofstream& outputFile);
void LogSkeletonNodeHiearchy(SkeletonNode* node);

// C:/Userst/j.albertson.C-P-U/Documents/Output/
// C:/Users/tjalb/OneDrive/Documents/Output/

void LogVertices(int numVertices, VertexData* vertices) {

    std::ofstream outputFile("/assimp_viewer_output.txt");
    if (!outputFile) {
        std::cout << "Failed to /open assimp_viewer_output.txt" << std::endl;
    }

    for (int i = 0; i < numVertices; ++i) {
        outputFile << " " << std::endl;
        outputFile << "Vertice " << i << std::endl;
        outputFile << "{" << std::endl;
        outputFile << "   Position: " << vertices[i].Position.x << ", " << vertices[i].Position.y << ", " << vertices[i].Position.z << std::endl;
        outputFile << "     Normal: " << vertices[i].Normal.x << ", " << vertices[i].Normal.y << ", " << vertices[i].Normal.z << std::endl;
        outputFile << "  TexCoords: " << vertices[i].TexCoords.x << ", " << vertices[i].TexCoords.y << std::endl;
        outputFile << "    Tangent: " << vertices[i].Tangent.x << ", " << vertices[i].Tangent.y << ", " << vertices[i].Tangent.z << std::endl;
        outputFile << "  Bitangent: " << vertices[i].Bitangent.x << ", " << vertices[i].Bitangent.y << ", " << vertices[i].Bitangent.z << std::endl;
        outputFile << "  m_BoneIDs: [" << vertices[i].m_BoneIDs[0] << ", " << vertices[i].m_BoneIDs[1] << ", " << vertices[i].m_BoneIDs[2] << ", " << vertices[i].m_BoneIDs[3] << "]" << std::endl;
        outputFile << "  m_Weights: [" << vertices[i].m_Weights[0] << ", " << vertices[i].m_Weights[1] << ", " << vertices[i].m_Weights[2] << ", " << vertices[i].m_Weights[3] << "]" << std::endl;
        outputFile << "}" << std::endl;
    }

    outputFile.close();
    std::cout << "Data saved to /assimp_viewer_output.txt" << std::endl;
}

void WriteSkeletonNodes(SkeletonNode* node, std::ofstream& outputFile) {

    outputFile << " " << std::endl;
    outputFile << node->m_NodeName << std::endl;
    outputFile << "{" << std::endl;
    outputFile << "                 id: " << node->id << std::endl;
    outputFile << "      m_NumChildren: " << node->m_NumChildren << std::endl;

    outputFile << "   m_Transformation: " << std::endl;
    outputFile << "                         [ " << node->m_Transformation[0][0] << ", " << node->m_Transformation[0][1] << ", " << node->m_Transformation[0][2] << ", " << node->m_Transformation[0][3] << " ]" << std::endl;
    outputFile << "                         [ " << node->m_Transformation[1][0] << ", " << node->m_Transformation[1][1] << ", " << node->m_Transformation[1][2] << ", " << node->m_Transformation[1][3] << " ]" << std::endl;
    outputFile << "                         [ " << node->m_Transformation[2][0] << ", " << node->m_Transformation[2][1] << ", " << node->m_Transformation[2][2] << ", " << node->m_Transformation[2][3] << " ]" << std::endl;
    outputFile << "                         [ " << node->m_Transformation[3][0] << ", " << node->m_Transformation[3][1] << ", " << node->m_Transformation[3][2] << ", " << node->m_Transformation[3][3] << " ]" << std::endl;

    outputFile << "           m_Offset: " << std::endl;
    outputFile << "                         [ " << node->m_Offset[0][0] << ", " << node->m_Offset[0][1] << ", " << node->m_Offset[0][2] << ", " << node->m_Offset[0][3] << " ]" << std::endl;
    outputFile << "                         [ " << node->m_Offset[1][0] << ", " << node->m_Offset[1][1] << ", " << node->m_Offset[1][2] << ", " << node->m_Offset[1][3] << " ]" << std::endl;
    outputFile << "                         [ " << node->m_Offset[2][0] << ", " << node->m_Offset[2][1] << ", " << node->m_Offset[2][2] << ", " << node->m_Offset[2][3] << " ]" << std::endl;
    outputFile << "                         [ " << node->m_Offset[3][0] << ", " << node->m_Offset[3][1] << ", " << node->m_Offset[3][2] << ", " << node->m_Offset[3][3] << " ]" << std::endl;
    outputFile << "}" << std::endl;

    if (node->m_NumChildren > 0) {
        for (int i = 0; i < node->m_NumChildren; ++i)
            WriteSkeletonNodes(node->m_Children[i], outputFile);
    }
}

void LogSkeletonNodeHiearchy(SkeletonNode* node) {

    std::ofstream outputFile("/assimp_viewer_skeleton.txt");
    if (!outputFile) {
        std::cout << "Failed to open /assimp_viewer_skeleton.txt" << std::endl;
    }

    WriteSkeletonNodes(node, outputFile);

    outputFile.close();
    std::cout << "Data saved to /assimp_viewer_skeleton.txt" << std::endl;
}


#endif