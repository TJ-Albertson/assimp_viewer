/*-------------------------------------------------------------------------------\
space.h

spacial partioning
\-------------------------------------------------------------------------------*/
#ifndef SPACE_H
#define SPACE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Define a structure for 3D vectors
typedef glm::vec3 Vector;
typedef glm::vec3 Point;


// Define a structure for an AABB (Axis-Aligned Bounding Box)
typedef struct {
    Vector min;  // Minimum corner of the AABB
    Vector max;  // Maximum corner of the AABB
} AABB;

// Define a structure for a triangle (vertices of a triangle)
struct Triangle {
    Point vertices[3];
};

// Define a structure for an AABB tree node
typedef struct AABBTreeNode {
    AABB aabb;
    struct AABBTreeNode* left;   // Pointer to left child node
    struct AABBTreeNode* right;  // Pointer to right child node
} AABBTreeNode;

// Function to compute the AABB of a set of triangles
AABB computeAABB(const Triangle* triangles, int numTriangles)
{
    // Initialize the AABB with the first triangle's vertices
    AABB aabb;
    aabb.min = aabb.max = triangles[0].vertices[0];

    for (int i = 0; i < numTriangles; ++i) {
        for (int j = 0; j < 3; ++j) {
            const Point& vertex = triangles[i].vertices[j];

            // Update the minimum and maximum coordinates of the AABB
            aabb.min.x = std::min(aabb.min.x, vertex.x);
            aabb.min.y = std::min(aabb.min.y, vertex.y);
            aabb.min.z = std::min(aabb.min.z, vertex.z);

            aabb.max.x = std::max(aabb.max.x, vertex.x);
            aabb.max.y = std::max(aabb.max.y, vertex.y);
            aabb.max.z = std::max(aabb.max.z, vertex.z);
        }
    }

    return aabb;
}

// Function to find the longest axis of an AABB
int longestAxis(AABB aabb) {
    Vector diff;
    diff.x = aabb.max.x - aabb.min.x;
    diff.y = aabb.max.y - aabb.min.y;
    diff.z = aabb.max.z - aabb.min.z;

    if (diff.x >= diff.y && diff.x >= diff.z) {
        return 0;
    } else if (diff.y >= diff.x && diff.y >= diff.z) {
        return 1;
    } else {
        return 2;
    }
}

// Function to split a set of triangles into two subsets based on the median
void splitTriangles(Triangle* triangles, int numTriangles, int axis, float median,
                    Triangle** leftSubset, int* leftCount, Triangle** rightSubset, int* rightCount) {
    *leftCount = 0;
    *rightCount = 0;

    for (int i = 0; i < numTriangles; i++) {
        Triangle triangle = triangles[i];

        // Calculate the midpoint of the triangle's projection onto the specified axis
        float mid = 0.0f;
        if (axis == 0) {
            mid = (triangle.vertices[0].x + triangle.vertices[1].x + triangle.vertices[3].x) / 3.0f;
        } else if (axis == 1) {
            mid = (triangle.vertices[0].y + triangle.vertices[1].y + triangle.vertices[3].y) / 3.0f;
        } else {
            mid = (triangle.vertices[0].z + triangle.vertices[1].z + triangle.vertices[3].z) / 3.0f;
        }

        if (mid < median) {
            (*leftSubset)[(*leftCount)++] = triangle;
        } else {
            (*rightSubset)[(*rightCount)++] = triangle;
        }
    }
}

// Function to build the AABB tree recursively
AABBTreeNode* buildAABBTree(Triangle* triangles, int numTriangles) {

    printf("numTriangles: %d\n", numTriangles);

    printf("triangles[0].vertices[0]x: %f\n", triangles[0].vertices[0].x);
    printf("triangles[0].vertices[0]y: %f\n", triangles[0].vertices[0].y);
    printf("triangles[0].vertices[0]z: %f\n", triangles[0].vertices[0].z);
    if (numTriangles == 0) {
        return NULL;
    }

    // Compute the AABB of the current set of triangles
    AABB aabb = computeAABB(triangles, numTriangles);

    // Find the longest axis of the AABB
    int axis = longestAxis(aabb);

    printf("aabb.min: %f %f %f\n", aabb.min.x, aabb.min.y, aabb.min.z);
    printf("aabb.max: %f %f %f\n", aabb.max.x, aabb.max.y, aabb.max.z);

    printf("axis: %d\n", axis);

    // Find the median of the axis
    float median = (aabb.min[axis] + aabb.max[axis]) / 2.0f;

    

    printf("median: %f\n", median);

    // Allocate memory for left and right subsets of triangles
    Triangle* leftSubset = (Triangle*)malloc(sizeof(Triangle) * numTriangles);
    Triangle* rightSubset = (Triangle*)malloc(sizeof(Triangle) * numTriangles);
    int leftCount, rightCount;

    // Split the triangles into two subsets based on the median
    splitTriangles(triangles, numTriangles, axis, median, &leftSubset, &leftCount, &rightSubset, &rightCount);

    // Create a new node for the current AABB
    AABBTreeNode* node = (AABBTreeNode*)malloc(sizeof(AABBTreeNode));
    node->aabb = aabb;

    // Recursively build the left and right subtrees
    node->left = buildAABBTree(leftSubset, leftCount);
    node->right = buildAABBTree(rightSubset, rightCount);

    // Free allocated memory for subsets
    free(leftSubset);
    free(rightSubset);

    return node;
}

/*
int main() {
    // Example data: an array of triangles
    int numTriangles = 4;
    Triangle triangles[4] = {
        {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}},
        {{1, 1, 0}, {1, 0, 0}, {0, 1, 0}},
        {{0, 0, 1}, {1, 0, 1}, {0, 1, 1}},
        {{1, 1, 1}, {1, 0, 1}, {0, 1, 1}},
    };

    // Build the AABB tree
    AABBTreeNode* root = buildAABBTree(triangles, numTriangles);

    // Now you have the AABB tree structure for your triangle polygon soup
    // You can traverse the tree as needed for collision detection or other operations

    return 0;
}
*/



enum nodeType {
    LEAF, // 0
    NODE, // 1
};

typedef struct Node {
    AABB aabb;
    nodeType type;
    int numObjects;
    Triangle* object;
    struct Node* left; // Pointer to left child node
    struct Node* right; // Pointer to right child node
} Node;


AABB ComputeBoundingVolume(const Triangle* triangles, int numTriangles)
{
    // Initialize the AABB with the first triangle's vertices
    AABB aabb;
    aabb.min = aabb.max = triangles[0].vertices[0];

    for (int i = 0; i < numTriangles; ++i) {
        for (int j = 0; j < 3; ++j) {
            const Point& vertex = triangles[i].vertices[j];

            // Update the minimum and maximum coordinates of the AABB
            aabb.min.x = std::min(aabb.min.x, vertex.x);
            aabb.min.y = std::min(aabb.min.y, vertex.y);
            aabb.min.z = std::min(aabb.min.z, vertex.z);

            aabb.max.x = std::max(aabb.max.x, vertex.x);
            aabb.max.y = std::max(aabb.max.y, vertex.y);
            aabb.max.z = std::max(aabb.max.z, vertex.z);
        }
    }

    return aabb;
}

float testMidpoint(Triangle triangle, int axis)
{
    float mid = 0.0f;
    if (axis == 0) {
        mid = (triangle.vertices[0].x + triangle.vertices[1].x + triangle.vertices[2].x) / 3.0f;
    } else if (axis == 1) {
        mid = (triangle.vertices[0].y + triangle.vertices[1].y + triangle.vertices[2].y) / 3.0f;
    } else {
        mid = (triangle.vertices[0].z + triangle.vertices[1].z + triangle.vertices[2].z) / 3.0f;
    }

    return mid;
}

int PartitionObjects(Triangle triangles[], int numTriangles, int axis, float median)
{
    std::vector<Triangle> left;
    std::vector<Triangle> right;


    for (int i = 0; i < numTriangles; i++) {
        Triangle triangle = triangles[i];

        float mid = testMidpoint(triangle, axis);

        if (mid < median) {
            left.push_back(triangle);
        } else {
            right.push_back(triangle);
        }
    };


    //(" left.size(): %d\n", left.size());
    //printf(" right.size(): %d\n", right.size());

    for (int i = 0; i < left.size(); i++) {
        triangles[i] = left[i];
    }

    for (int i = 0; i < right.size(); i++) {
        triangles[left.size() + i] = right[i];
    }

    if (left.size() == 0 && right.size() != 0 || left.size() != 0 && right.size() == 0) {
        return 0;
    }

    int k = left.size();

    return k;
}

// Construct a top-down tree. Rearranges object[] array during construction
void TopDownBVTree(Node** tree, Triangle triangles[], int numObjects)
{
    //assert(numObjects > 0);
     //printf("numObjects: %d\n", numObjects);
    if (numObjects == 0) return;
    /*
    for (int i = 0; i < numObjects; i++) {
        std::cout << "Triangle " << i << ": " << std::endl;
        std::cout << "     vertices[0]: " << triangles[i].vertices[0].x << " " << triangles[i].vertices[0].y << " " << triangles[i].vertices[0].z << std::endl;
        std::cout << "     vertices[1]: " << triangles[i].vertices[1].x << " " << triangles[i].vertices[1].y << " " << triangles[i].vertices[1].z << std::endl;
        std::cout << "     vertices[2]: " << triangles[i].vertices[2].x << " " << triangles[i].vertices[2].y << " " << triangles[i].vertices[2].z << std::endl;
    }
    */

    //printf("numTriangles: %d\n", numObjects);
    const int MIN_OBJECTS_PER_LEAF = 1;
    Node* pNode = new Node;
    *tree = pNode;
    // Compute a bounding volume for object[0], ..., object[numObjects - 1]
    pNode->aabb = ComputeBoundingVolume(&triangles[0], numObjects);

    if (numObjects <= MIN_OBJECTS_PER_LEAF) {
        pNode->type = LEAF;
        pNode->numObjects = numObjects;
        pNode->object = &triangles[0]; // Pointer to first object in leaf
    } else {
        pNode->type = NODE;
        
        int axis = longestAxis(pNode->aabb);
        float median = (pNode->aabb.min[axis] + pNode->aabb.max[axis]) / 2.0f;
            
        // Based on some partitioning strategy, arrange objects into
        // two partitions: object[0..k-1], and object[k..numObjects-1]
        int k = PartitionObjects(&triangles[0], numObjects, axis, median);

        if (k == 0) {
            pNode->type = LEAF;
            pNode->numObjects = numObjects;
            pNode->object = &triangles[0];
            return;
        }

        // Recursively construct left and right subtree from subarrays and
        // point the left and right fields of the current node at the subtrees
        TopDownBVTree(&(pNode->left), &triangles[0], k);
        TopDownBVTree(&(pNode->right), &triangles[k], numObjects - k);
    }
}


void printAABBMinMax(Node* node)
{
    if (node == NULL || node->type == LEAF) {
        return;
    }

    printf("Node: Min (%.2f, %.2f, %.2f), Max (%.2f, %.2f, %.2f)\n",
        node->aabb.min.x, node->aabb.min.y, node->aabb.min.z,
        node->aabb.max.x, node->aabb.max.y, node->aabb.max.z);

    printAABBMinMax(node->left);
    printAABBMinMax(node->right);
}

/*
int main() {
    int numTriangles = 4;
    Triangle triangles[4] = {
        { { 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 } },
        { { 1, 1, 0 }, { 1, 0, 0 }, { 0, 1, 0 } },
        { { 0, 0, 1 }, { 1, 0, 1 }, { 0, 1, 1 } },
        { { 1, 1, 1 }, { 1, 0, 1 }, { 0, 1, 1 } },
    };

    // Build the AABB tree
    Node* root;
    TopDownBVTree(&root, triangles, numTriangles);

    // Now you have the AABB tree structure for your triangle polygon soup
    // You can traverse the tree as needed for collision detection or other operations

    return 0;
    TopDownBVTree
}
*/
#endif;