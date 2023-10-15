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
AABB computeAABB(Triangle* triangles, int numTriangles) {
    AABB aabb;
    aabb.min = aabb.max = triangles[0].vertices[0];  // Initialize with the first vertex

    for (int i = 0; i < numTriangles; i++) {
        Triangle triangle = triangles[i];

        // Update the AABB's minimum and maximum corners
        aabb.min.x = fminf(aabb.min.x, fminf(fminf(triangle.vertices[0].x, triangle.vertices[1].x), triangle.vertices[2].x));
        aabb.min.y = fminf(aabb.min.y, fminf(fminf(triangle.vertices[0].y, triangle.vertices[1].y), triangle.vertices[2].y));
        aabb.min.z = fminf(aabb.min.z, fminf(fminf(triangle.vertices[0].z, triangle.vertices[1].z), triangle.vertices[2].z));

        aabb.max.x = fmaxf(aabb.max.x, fmaxf(fmaxf(triangle.vertices[0].x, triangle.vertices[1].x), triangle.vertices[2].x));
        aabb.max.y = fmaxf(aabb.max.y, fmaxf(fmaxf(triangle.vertices[0].y, triangle.vertices[1].y), triangle.vertices[2].y));
        aabb.max.z = fmaxf(aabb.max.z, fmaxf(fmaxf(triangle.vertices[0].z, triangle.vertices[1].z), triangle.vertices[2].z));
    }

    return aabb;
}

// Function to find the longest axis of an AABB
int longestAxis(AABB aabb) {
    Vector extent = {aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y, aabb.max.z - aabb.min.z};
    if (extent.x > extent.y && extent.x > extent.z) {
        return 0;  // X-axis is the longest
    } else if (extent.y > extent.z) {
        return 1;  // Y-axis is the longest
    } else {
        return 2;  // Z-axis is the longest
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

    printf("axis: %d\n", axis);

    // Find the median of the axis
    float median = (aabb.min[axis] + aabb.max[axis]) / 2.0f;

    printf("aabb.min[axis]: %f\n", aabb.min[axis]);
    printf("aabb.max[axis]: %f\n", aabb.max[axis]);

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

#endif;