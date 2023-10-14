/*-------------------------------------------------------------------------------\
space.h

spacial partioning
\-------------------------------------------------------------------------------*/
#ifndef SPACE_H
#define SPACE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef glm::vec3 Point;

struct Object {
    Object* pNextObject; // Embedded link to next hgrid object
    Point pos; // x, y (and z) position for sphere (or top left AABB corner)
    float radius; // Radius for bounding sphere (or width of AABB)
    int bucket; // Index of hash bucket object is in
    int level; // Grid level for the object
    //... Object data
};

// Octree node data structure
struct OctNode {
    Point center;       // Center point of octree node (not strictly needed)
    float halfWidth;    // Half the width of the node volume (not strictly needed)
    OctNode* pChild[8]; // Pointers to the eight children nodes
    Object* pObjList;   // Linked list of objects contained at this node
};

#endif;