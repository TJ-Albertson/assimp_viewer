/*-------------------------------------------------------------------------------\
aabb.h


\-------------------------------------------------------------------------------*/
#ifndef SPACE_H
#define SPACE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <stack>

//Max size of stack used during AABB tree traversal
#define MAX_SIZE 300

typedef glm::vec3 Vector;
typedef glm::vec3 Point;

typedef struct {
    Vector min;
    Vector max;
} AABB;

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
    struct AABB_node* left;
    struct AABB_node* right;

    // Assigned when loading model. From Vao.
    // Used to visualize colliding AABB's
    unsigned int id;
} AABB_node;

// Stack used for AABB tree traversal
typedef struct StackNode {
    AABB_node a;
    AABB_node b;
    StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode items[MAX_SIZE];
    int top;
} Stack;


std::vector<unsigned int> colliding_aabbs;

// Collision Detection
void AABB_AABB_Collision(AABB_node a, AABB_node b, glm::mat4* a_matrix, glm::mat4* b_matrix);

// AABB Creation
void TopDownABBB_Tree(AABB_node** tree, Triangle triangles[], int numObjects);



// Returns largest axis of AABB. 0 = X. 1 = Y. 2 = Z.
int LongestAxis(AABB aabb) {
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

// Computes AABB based on min/max vertices of an array of triangles
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

float TestMidpoint(Triangle triangle, int axis)
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

int GetNextAxis(int currentAxis)
{
    return (currentAxis + 1) % 3; // 0, 1, 2, 0, 1, 2, ...
}

// Splits triangles based on the given axis and median.
void SplitTriangles(std::vector<Triangle>& triangles, const int axis, const float median, std::vector<Triangle>& left, std::vector<Triangle>& right)
{
    for (const Triangle& triangle : triangles) {
        float mid = TestMidpoint(triangle, axis);
        if (mid < median) {
            left.push_back(triangle);
        } else {
            right.push_back(triangle);
        }
    }
}

/*  Attempts to evenly divide triangles withing AABB bounds.
*   If all triangles are on one side of dividing plane, other axis
*   are tried until triangles are divided. In the case of 2 stuck
*   triangles they are forcefully split.
*/
int PartitionObjects(Triangle triangles[], int numTriangles, int axis, AABB aabb)
{
    std::vector<Triangle> triangleVector(triangles, triangles + numTriangles);
    std::vector<Triangle> left;
    std::vector<Triangle> right;

    int count = 0;
    while (1) {
        count++;

        float median = (aabb.min[axis] + aabb.max[axis]) / 2.0f;
        SplitTriangles(triangleVector, axis, median, left, right);

        if (count == 3)
            break;

        if (left.empty() || right.empty()) {
            left.clear();
            right.clear();
            axis = GetNextAxis(axis);
        } else {
            break;
        }
    }

    // Failed to split on all axi
    if (left.empty() || right.empty()) {
        for (int i = 0; i < triangleVector.size(); i++) {
            triangles[i] = (triangleVector[i]);
        }

        return triangleVector.size() - 1;
    }

    for (int i = 0; i < left.size(); i++) {
        triangles[i] = left[i];
    }

    for (int i = 0; i < right.size(); i++) {
        triangles[left.size() + i] = right[i];
    }

    //printf("left.size(): %d\n", left.size());
    //printf("right.size(): %d\n", right.size());

    int k = left.size();

    return k;
}

// Construct a top-down AABB tree. Rearranges object[] array during construction
void TopDownABBB_Tree(AABB_node** tree, Triangle triangles[], int numObjects)
{
    // Failure. Unbalanced Tree
    assert(numObjects > 0);

    //printf("numObjects: %d\n", numObjects);
    /*
    for (int i = 0; i < numObjects; i++) {
        std::cout << "Triangle " << i << ": " << std::endl;
        std::cout << "     vertices[0]: " << triangles[i].vertices[0].x << " " << triangles[i].vertices[0].y << " " << triangles[i].vertices[0].z << std::endl;
        std::cout << "     vertices[1]: " << triangles[i].vertices[1].x << " " << triangles[i].vertices[1].y << " " << triangles[i].vertices[1].z << std::endl;
        std::cout << "     vertices[2]: " << triangles[i].vertices[2].x << " " << triangles[i].vertices[2].y << " " << triangles[i].vertices[2].z << std::endl;
    }
    */

    const int MIN_OBJECTS_PER_LEAF = 1;
    AABB_node* pNode = (AABB_node*)malloc(sizeof(AABB_node));
    *tree = pNode;
    // Compute a bounding volume for object[0], ..., object[numObjects - 1]
    pNode->aabb = ComputeBoundingVolume(&triangles[0], numObjects);

    if (numObjects <= MIN_OBJECTS_PER_LEAF) {
        pNode->type = LEAF;
        pNode->numObjects = numObjects;
        pNode->object = &triangles[0]; // Pointer to first object in leaf
    } else {
        pNode->type = NODE;
        
        int axis = LongestAxis(pNode->aabb);
        
        // Based on some partitioning strategy, arrange objects into
        // two partitions: object[0..k-1], and object[k..numObjects-1]
        int k = PartitionObjects(&triangles[0], numObjects, axis, pNode->aabb);

        // Recursively construct left and right subtree from subarrays and
        // point the left and right fields of the current node at the subtrees
        TopDownABBB_Tree(&(pNode->left), &triangles[0], k);
        TopDownABBB_Tree(&(pNode->right), &triangles[k], numObjects - k);
    }
}








bool IsLeaf(AABB_node node) {
    return (node.type == LEAF);
}

// Returns surface area
float SizeOfAABB(AABB_node node)
{
    AABB aabb = node.aabb;
    
    float width = aabb.max.x - aabb.min.x;
    float height = aabb.max.y - aabb.min.y;
    float depth = aabb.max.z - aabb.min.z;

    float surface_area = 2 * (width * height + height * depth + width * depth);

    return surface_area;
}

// Descent rule
bool DescendA(AABB_node a, AABB_node b)
{
    // ‘Descend larger’ descent rule
    return IsLeaf(b) || (!IsLeaf(a) && (SizeOfAABB(a) >= SizeOfAABB(b)));

    // ‘Descend A’ descent rule
    //return !IsLeaf(a);

    // ‘Descend B’ descent rule
    //return IsLeaf(b);
}

int IsEmpty(struct Stack* stack)
{
    return stack->top == -1;
}

int isFull(struct Stack* stack)
{
    return stack->top == MAX_SIZE - 1;
}

void Push(struct Stack* stack, AABB_node a, AABB_node b)
{
    if (isFull(stack)) {
        printf("Stack is full. Cannot push.\n");
    } else {
        StackNode node;
        node.a = a;
        node.b = b;

        stack->items[++stack->top] = node;
    }
}

void Pop(struct Stack* stack, AABB_node& a, AABB_node& b)
{
    if (IsEmpty(stack)) {
        printf("Stack is empty. Cannot pop.\n");
    } else {
        StackNode pop = stack->items[stack->top--];

        a = pop.a;
        b = pop.b;
    }
}

// Test AABB pair after positing both in world space
int TestAABBAABB(AABB_node node_a, AABB_node node_b, glm::mat4* a_matrix, glm::mat4* b_matrix)
{
    AABB a = node_a.aabb;
    AABB b = node_b.aabb;

    glm::mat4 amat = (*a_matrix);
    glm::mat4 bmat = (*b_matrix);

    glm::mat3 tab = glm::inverse(amat) * bmat;

    //glm::vec3 b_min = tab * glm::vec3(b.min);
    //glm::vec3 b_max = tab * glm::vec3(b.max);

    glm::vec3 b_min = b.min + glm::vec3(bmat[3]);
    glm::vec3 b_max = b.max + glm::vec3(bmat[3]);

    glm::vec3 a_min = a.min + glm::vec3(amat[3]);
    glm::vec3 a_max = a.max + glm::vec3(amat[3]);

    // Exit with no intersection if separated along an axis
    
    if (a_max[0] < b_min[0] || a_min[0] > b_max[0])
        return 0;
    if (a_max[1] < b_min[1] || a_min[1] > b_max[1])
        return 0;
    if (a_max[2] < b_min[2] || a_min[2] > b_max[2])
        return 0;
    

    // Overlapping on all axes means AABBs are intersecting
    return 1;
}

/*  Test AABB tree pair.
*   Stack-use optimized, non-recursive version.
*/
void AABB_AABB_Collision(/*CollisionResult* r,*/ AABB_node a, AABB_node b, glm::mat4* a_matrix, glm::mat4* b_matrix)
{
    struct Stack s;
    s.top = -1;

    while (1) {
        if (TestAABBAABB(a, b, a_matrix, b_matrix)) {
            if (IsLeaf(a) && IsLeaf(b)) {

                //CollidePrimitives(r, a, b);
                //printf("Leaf Collision\n");
                
                if (std::find(colliding_aabbs.begin(), colliding_aabbs.end(), a.id) == colliding_aabbs.end()) {
                    colliding_aabbs.push_back(a.id);
                }

                if (std::find(colliding_aabbs.begin(), colliding_aabbs.end(), b.id) == colliding_aabbs.end()) {
                    colliding_aabbs.push_back(b.id);
                }
                
                break;
            } else {
                // if a is bigger than b, descend a
                if (DescendA(a, b)) {
                    Push(&s, *a.right, b);
                    a = *a.left;
                    continue;
                // else descend b
                } else {
                    Push(&s, a, *b.right);
                    b = *b.left;
                    continue;
                }
            }
        }

        if (IsEmpty(&s)) {
            colliding_aabbs.clear();
            break;
        }
            
        Pop(&s, a, b);
    }
}

AABB_node updateAABB(AABB_node* node, const glm::mat4 transformationMatrix)
{
    AABB_node newNode = *node;
    if (node == nullptr) {
        return newNode;
    }

    // Update the AABB using the transformation matrix
    Vector min = newNode.aabb.min;
    Vector max = newNode.aabb.max;

    

    glm::vec3 translate = transformationMatrix[3];


    //glm::vec3 inverse = glm::inverse(translate);

    min += translate;
    max += translate;

    newNode.aabb.min = min;
    newNode.aabb.max = max;

    if (node->type == LEAF) {
        return newNode;
    }

    // Recursively update the children
    updateAABB(newNode.left, transformationMatrix);
    updateAABB(newNode.right, transformationMatrix);

    return newNode;
}


// Print AABB tree min/maxes
void printAABBMinMax(AABB_node* node)
{
    if (node == NULL) {
        return;
    }

    printf("Node: Min (%.2f, %.2f, %.2f), Max (%.2f, %.2f, %.2f)\n",
        node->aabb.min.x, node->aabb.min.y, node->aabb.min.z,
        node->aabb.max.x, node->aabb.max.y, node->aabb.max.z);

    if (node->type == LEAF) {
        return;
    }

    printAABBMinMax(node->left);
    printAABBMinMax(node->right);
}

#endif;