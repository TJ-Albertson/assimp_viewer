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

enum nodeType {
    LEAF, // 0
    NODE, // 1
};

typedef struct AABB_node {
    AABB aabb;
    nodeType type;
    int numObjects;
    Triangle* object;
    struct AABB_node* left; // Pointer to left child node
    struct AABB_node* right; // Pointer to right child node
} AABB_node;


int TestAABBAABB(AABB_node node_a, AABB_node node_b);


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
void TopDownBVTree(AABB_node** tree, Triangle triangles[], int numObjects)
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
    AABB_node* pNode = new AABB_node;
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








bool IsLeaf(AABB_node* node) {
    return (node->type == LEAF);
}

float SizeOfBV(AABB_node* node)
{
    AABB aabb = node->aabb;
    
    float width = aabb.max.x - aabb.min.x;
    float height = aabb.max.y - aabb.min.y;
    float depth = aabb.max.z - aabb.min.z;

    float surface_area = 2 * (width * height + height * depth + width * depth);

    return surface_area;
}

/*
// ‘Descend A’ descent rule
bool DescendA(AABB_node a, AABB_node b)
{
    return !IsLeaf(&a);
}
// ‘Descend B’ descent rule
bool DescendA(AABB_node a, AABB_node b)
{
    return IsLeaf(&b);
}
*/
// ‘Descend larger’ descent rule
bool DescendA(AABB_node* a, AABB_node* b)
{
    return IsLeaf(b) || (!IsLeaf(a) && (SizeOfBV(a) >= SizeOfBV(b)));
}





typedef struct StackNode {
    AABB_node* a_node;
    AABB_node* b_node;
    StackNode* next;
} StackNode;

// Function to check if the stack is empty
int IsEmpty(StackNode* root)
{
    if (root == NULL) {
        return 1;
    }

    return (root->next == NULL);
}

// Function to push an element onto the stack
void Push( StackNode** root, AABB_node* a, AABB_node* b)
{
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        printf("Memory allocation error.\n");
        exit(1);
    }
    newNode->a_node = a;
    newNode->b_node = b;

    newNode->next = *root;
    *root = newNode;
    printf("%d pushed to the stack\n");
}

// Function to pop an element from the stack
void Pop(StackNode** root, AABB_node* a, AABB_node* b)
{
    if (IsEmpty(*root)) {
        printf("Stack is empty\n");
        exit(1);
    }
    struct StackNode* temp = *root;

    *root = temp->next;
    free(temp);
    return;
}



// Stack-use optimized, non-recursive version
void BVHCollision( /*CollisionResult* r,*/AABB_node* a, AABB_node* b)
{
    StackNode* s = (StackNode*)malloc(sizeof(StackNode));
    s->next = NULL;

    while (1) {
        if (TestAABBAABB(*a, *b)) {
            printf("AABB Collision\n");
            if (IsLeaf(a) && IsLeaf(b)) {
                // At leaf nodes. Perform collision tests on leaf node contents
                //CollidePrimitives(r, a, b);
                // Could have an exit rule here (eg. exit on first hit)
            } else {
                if (DescendA(a, b)) {
                    Push(&s, a->right, b);
                    a = a->left;
                    continue;
                } else {
                    Push(&s, a, b->right);
                    b = b->left;
                    continue;
                }
            }
        }
        if (IsEmpty(s))
            break;
        Pop(&s, a, b);
    }
}








int TestAABBAABB(AABB_node node_a, AABB_node node_b)
{
    AABB a = node_a.aabb;
    AABB b = node_b.aabb;

    // Exit with no intersection if separated along an axis
    if (a.max.x < b.min.x || a.min.x > b.max.x)
        return 0;
    if (a.max.y < b.min[1] || a.min[1] > b.max[1])
        return 0;
    if (a.max[2] < b.min[2] || a.min[2] > b.max[2])
        return 0;
    // Overlapping on all axes means AABBs are intersecting
    return 1;
}





void updateAABB(AABB_node* node, const glm::mat4 transformationMatrix)
{
    if (node == nullptr || node->type == LEAF) {
        return;
    }

    // Update the AABB using the transformation matrix
    Vector min = node->aabb.min;
    Vector max = node->aabb.max;

    // Transform the eight corners of the AABB
    glm::vec4 corners[8] = {
        transformationMatrix * glm::vec4(min.x, min.y, min.z, 1.0f),
        transformationMatrix * glm::vec4(min.x, min.y, max.z, 1.0f),
        transformationMatrix * glm::vec4(min.x, max.y, min.z, 1.0f),
        transformationMatrix * glm::vec4(min.x, max.y, max.z, 1.0f),
        transformationMatrix * glm::vec4(max.x, min.y, min.z, 1.0f),
        transformationMatrix * glm::vec4(max.x, min.y, max.z, 1.0f),
        transformationMatrix * glm::vec4(max.x, max.y, min.z, 1.0f),
        transformationMatrix * glm::vec4(max.x, max.y, max.z, 1.0f)
    };

    // Calculate the new AABB by finding the min and max of the transformed corners
    Vector newMin = glm::vec3(corners[0]);
    Vector newMax = glm::vec3(corners[0]);

    for (int i = 1; i < 8; ++i) {
        newMin = glm::min(newMin, glm::vec3(corners[i]));
        newMax = glm::max(newMax, glm::vec3(corners[i]));
    }

    // Update the AABB of the current node
    node->aabb.min = newMin;
    node->aabb.max = newMax;

    // Recursively update the children
    updateAABB(node->left, transformationMatrix);
    updateAABB(node->right, transformationMatrix);
}








void printAABBMinMax(AABB_node* node)
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

#endif;