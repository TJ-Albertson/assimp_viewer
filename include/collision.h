/*-------------------------------------------------------------------------------\
collision.h


\-------------------------------------------------------------------------------*/
#ifndef COLLISION_H
#define COLLISION_H

#include <glad/glad.h>s

#include <vector>

#include <aabb.h>

const float EPSILON = 1e-7;
float scaleFactor = 1;

struct Polygon {
    std::vector<Point> vertices;
    Vector normal;
    glm::mat4* matrix;
};

struct Sphere {
    Point center;
    float radius;
};

struct Plane {
    Vector n; // Plane normal. Points x on the plane satisfy Dot(n,x) = d
    float d;  // d = dot(n,p) for a given point p on the plane
};

struct Hitbox {
    glm::mat4* m_Matrix;
    AABB_node* rootAABB;
};

std::vector<Polygon> potentialColliders;
std::vector<AABB_node*> root_AABB_nodes;
std::vector<Hitbox> hitboxes;

glm::vec3 collisionBallPosition;
glm::vec3 vectorPosition;
glm::vec3 newDestinationPointBall;

int CollisionDetection(Sphere sphere, Vector& velocity, Point& collision_point);
void CollisionResponse(Vector& originalVelocity, Sphere sphere, Point collision_point, Plane collision_plane);

AABB_node* CreateHitbox(std::string const& path, glm::mat4 matrix);

// Primitive Equations
Point ClosestPtPointPlane(Point q, Plane p);
float DistPointPlane(Point q, Plane p);
int IntersectMovingSpherePlane(Sphere s, Vector v, Plane p, float& t, Point& q);
int PointInTriangle(Point p, Point a, Point b, Point c);
int IntersectSegmentCylinder(Point sa, Point sb, Point p, Point q, float r, float& t);
int IntersectRaySphere(Point p, Vector d, Sphere s, float& t, Point& q);
void ClosestPtPointSegment(Point c, Point a, Point b, float& t, Point& d);

// Utility
void PrintColliders(glm::vec3 player_move_vec, glm::vec3 player_center);


    /*
1. sphere-plane test
        5.5.3 Intersecting Moving Sphere Against Plane

2. face test (is P inside T?)

3. edge test (cylinder, curved surface only)
        5.3.7 Intersecting Ray or Segment Against Cylinder (last bit of test can be omitted, dont need to check endcaps)

4. vertice test (sphere centered at vertice, return closest)
        5.3.2 Intersecting Ray or Segment Against Sphere

5. if all pass then no intersection
*/
int CollisionDetection(Sphere sphere, Vector& velocity, Point& collision_point)
{
    bool collisionFlag = false;

    for (int j = 0; j < potentialColliders.size(); ++j) {

        Plane p;
        p.n = potentialColliders[j].normal;
        p.d = glm::dot(p.n, potentialColliders[j].vertices[0]);

        Triangle t;
        t.vertices[0] = potentialColliders[j].vertices[0];
        t.vertices[1] = potentialColliders[j].vertices[1];
        t.vertices[2] = potentialColliders[j].vertices[2];

        float time_of_collision;
        bool sphere_embedded = false;

        // 1. sphere-plane test
        // collision_point represents sphere.center at time of collision
        int planeCollision = IntersectMovingSpherePlane(sphere, velocity, p, time_of_collision, collision_point);

        if (!planeCollision || time_of_collision > 1) {
            continue;
        }

        // 
        Point plane_collision_point = ClosestPtPointPlane(sphere.center, p);
        float distance = glm::distance(sphere.center, plane_collision_point);
        collision_point = plane_collision_point;

        if (distance < sphere.radius) {
            sphere_embedded = true;
        }

        // 2. face test
        int point_in_triangle = PointInTriangle(collision_point, t.vertices[0], t.vertices[1], t.vertices[2]);

        if (point_in_triangle) {
            // earliest intersection point found
            collisionFlag = true;
            CollisionResponse(velocity, sphere, collision_point, p);
            continue;
        }

        // 3. edge test
        bool edge_collision = false;
        float least_time = 1;

        Point a, b;

        for (int i = 0; i < 3; ++i) {
            int vertex1 = i;
            int vertex2 = (i + 1) % 3;

            float time;

            int edge_intersect = IntersectSegmentCylinder(sphere.center, sphere.center + velocity, t.vertices[vertex1], t.vertices[vertex2], sphere.radius, time);

            if (edge_intersect && time <= least_time) {
                edge_collision = true;
                least_time = time_of_collision = time;
                a = t.vertices[vertex1];
                b = t.vertices[vertex2];
            }
        }

        if (edge_collision) {
            float time;
            Point edge_point;
            collisionFlag = true;

            if (sphere_embedded) {
                Point destination = sphere.center + velocity; 
                ClosestPtPointSegment(destination, a, b, time, edge_point);
                collision_point = collisionBallPosition = edge_point;
            }
            CollisionResponse(velocity, sphere, collision_point, p);

            continue;
        }

        // 4. vertice test
        bool vertex_collision = false;

        int vertex = -1;
        least_time = 1;

        for (int i = 0; i < 3; ++i) {

            Sphere temp;
            temp.center = t.vertices[i];
            temp.radius = sphere.radius;

            float time;
            Point point;

            int vertex_intersection = IntersectRaySphere(sphere.center, velocity, temp, time, point);

            // if plane collision then time == 0 so need to reset
            if (vertex_intersection && time < least_time) {
                vertex_collision = true;
                least_time = time;
                time_of_collision = time;

                collision_point = t.vertices[i];
                vertex = i;
            }
        }

        if (vertex_collision) {
            collisionFlag = true;
            CollisionResponse(velocity, sphere, collision_point, p);
            continue;
        }

        // 5. No collision
    }
    //collisionBallPosition = collision_point;
    if (collisionFlag)
        return 1;

    return 0;
}

// Find slide plane and project velocity vector onto it. If velocity is too small then don't modify.
void CollisionResponse(Vector& originalVelocity, Sphere sphere, Point collision_point, Plane collision_plane)
{
    if (glm::length(originalVelocity) < EPSILON) {
        return;
    }

    Vector sliding_plane_normal = sphere.center - collision_point;
    sliding_plane_normal = glm::normalize(sliding_plane_normal);

    Plane sliding_plane;
    sliding_plane.n = sliding_plane_normal;
    sliding_plane.d = glm::dot(sliding_plane_normal, collision_point);

    float distance = DistPointPlane(sphere.center + originalVelocity, sliding_plane);

    Vector newDestinationPoint = sphere.center + originalVelocity - distance * sliding_plane_normal;
    newDestinationPointBall = newDestinationPoint;
    Vector newVelocityVector = newDestinationPoint - collision_point;

    //newVelocity = originalVelocity - (orignialVelocity * planeNormal) * planeNormal - (orignialVelocity * planeNormal) * planeNormal;
    //newVelocity = v - vn - vn

    // bounciness
    float k = 0.0f;

    glm::vec3 velocityN = (originalVelocity * sliding_plane_normal) * sliding_plane_normal;
    glm::vec3 velocityP = originalVelocity - velocityN;

    glm::vec3 newVelocity = velocityP - k * velocityN;

    //friction
    float friction = 0.01f;
    newVelocity *= (1.0f - friction);

    if (glm::length(newVelocityVector) < EPSILON) {
        originalVelocity = glm::vec3(0);
        return;
    }

    vectorPosition = originalVelocity = newVelocity;

    //vectorPosition = originalVelocity = newVelocityVector;
}


//Create hitbox from .obj and add to hitbox array. Assumes triangulated
AABB_node* CreateHitbox(std::string const& path, glm::mat4 matrix)
{
    const char* file_path = path.c_str();

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    int vertexCount = 0;
    int normalCount = 0;
    int faceCount = 0;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Polygon> polygons;

    char line[256];

    glm::mat4 normalMatrix = glm::transpose(glm::inverse(matrix));

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') {
                float x, y, z;
                sscanf(line, "v %f %f %f", &x, &y, &z);
                vertices.push_back(glm::vec3(matrix * glm::vec4(x, y, z, 1.0f)));
                //vertices.push_back(glm::vec3(x, y, z));
                //verticeCount++;
            } else if (line[1] == 'n') {
                float nx, ny, nz;
                sscanf(line, "vn %f %f %f", &nx, &ny, &nz);
                normals.push_back(glm::normalize(glm::vec3(normalMatrix * glm::vec4(nx, ny, nz, 1.0))));
                //normals.push_back(glm::vec3(nx, ny, nz));
                
                //normalCount++;
            }
        } else if (line[0] == 'f') {
            int vertex_1, vertex_2, vertex_3, vertex_normal;

            int basic = sscanf(line, "f %d/%*d/%d %d/%*d/%*d %d/%*d/%*d", &vertex_1, &vertex_normal, &vertex_2, &vertex_3);
            int with_color = sscanf(line, "f %d//%d %d//%*d %d//%*d", &vertex_1, &vertex_normal, &vertex_2, &vertex_3);

            if (basic == 4) {
                sscanf(line, "f %d/%*d/%d %d/%*d/%*d %d/%*d/%*d", &vertex_1, &vertex_normal, &vertex_2, &vertex_3);
            } else if (with_color == 4) {
                sscanf(line, "f %d//%d %d//%*d %d//%*d", &vertex_1, &vertex_normal, &vertex_2, &vertex_3);
            }


            Polygon polygon;

            polygon.vertices.push_back(vertices[vertex_1 - 1]);
            polygon.vertices.push_back(vertices[vertex_2 - 1]);
            polygon.vertices.push_back(vertices[vertex_3 - 1]);
            
            polygon.normal = normals[vertex_normal - 1];

            polygons.push_back(polygon);

            //faceCount++;
        }
    }

    fclose(file);

    Triangle* triangles = (Triangle*)malloc(sizeof(Triangle) * polygons.size());
        

    for (int i = 0; i < polygons.size(); ++i) {
        potentialColliders.push_back(polygons[i]);

        triangles[i].vertices[0] = polygons[i].vertices[0];
        triangles[i].vertices[1] = polygons[i].vertices[1];
        triangles[i].vertices[2] = polygons[i].vertices[2];
    }


    AABB_node* rootAABBnode;
    
    TopDownABBB_Tree(&rootAABBnode, triangles, polygons.size());

    //printAABBMinMax(rootAABBnode);

    root_AABB_nodes.push_back(rootAABBnode);

    //updateAABB(rootAABBnode, matrix);

    for (int i = 0; i < polygons.size(); i++) {
        //hitbox.m_Polygons[i] = polygons[i];
    }

    //hitboxes.push_back(hitbox);

    return rootAABBnode;
}

Point ClosestPtPointPlane(Point q, Plane p)
{
    float t = (glm::dot(p.n, q) - p.d) / glm::dot(p.n, p.n);
    return q - t * p.n;
}

float DistPointPlane(Point q, Plane p)
{
    // return Dot(q, p.n) - p.d; if plane equation normalized (||p.n||==1)
    return (glm::dot(p.n, q) - p.d) / glm::dot(p.n, p.n);
}

// Intersect sphere s with movement vector v with plane p. If intersecting
// return time t of collision and point q at which sphere hits plane
int IntersectMovingSpherePlane(Sphere s, Vector v, Plane p, float& t, Point& q)
{
    // Compute distance of sphere center to plane
    float dist = glm::dot(p.n, s.center) - p.d;
    if (glm::abs(dist) <= s.radius) {
        // The sphere is already overlapping the plane. Set time of
        // intersection to zero and q to sphere center
        t = 0.0f;
        q = s.center;
        return 1;
    }
    else {
        float denom = glm::dot(p.n, v);
        if (denom * dist >= 0.0f) {
            // No intersection as sphere moving parallel to or away from plane
            return 0;
        }
        else {
            // Sphere is moving towards the plane
            // Use +r in computations if sphere in front of plane, else -r
            float r = dist > 0.0f ? s.radius : -s.radius;
            t = (r - dist) / denom;
            q = s.center + t * v - r * p.n;
            return 1;
        }
    }
}

// Modified to check if Point is in Triangle. May be more efficient way to do it.
int PointInTriangle(Point p, Point a, Point b, Point c)
{
    // Check if P in vertex region outside A
    Vector ab = b - a;
    Vector ac = c - a;
    Vector ap = p - a;
    float d1 = glm::dot(ab, ap);
    float d2 = glm::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return 0;

    // Check if P in vertex region outside B
    Vector bp = p - b;
    float d3 = glm::dot(ab, bp);
    float d4 = glm::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return 0;

    // Check if P in edge region of AB
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        return 0;
    }

    // Check if P in vertex region outside C
    Vector cp = p - c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return 0;

    // Check if P in edge region of AC
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        return 0;
    }

    // Check if P in edge region of BC
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        return 0;
    }

    // P inside face region. 
    return 1;
}

// Intersect segment S(t)=sa+t(sb-sa), 0<=t<=1 against cylinder specified by p, q and r
int IntersectSegmentCylinder(Point sa, Point sb, Point p, Point q, float r, float& t)
{
    Vector d = q - p, m = sa - p, n = sb - sa;
    float md = glm::dot(m, d);
    float nd = glm::dot(n, d);
    float dd = glm::dot(d, d);

    // Test if segment fully outside either endcap of cylinder
    if (md < 0.0f && md + nd < 0.0f) return 0; // Segment outside ’p’ side of cylinder
    if (md > dd && md + nd > dd) return 0; // Segment outside ’q’ side of cylinder
    float nn = glm::dot(n, n);
    float mn = glm::dot(m, n);
    float a = dd * nn - nd * nd;
    float k = glm::dot(m, m) - r * r;
    float c = dd * k - md * md;

    if (glm::abs(a) < EPSILON) {
        // Segment runs parallel to cylinder axis
        if (c > 0.0f) return 0; // ’a’ and thus the segment lie outside cylinder
        // Now known that segment intersects cylinder; figure out how it intersects
        if (md < 0.0f) t = -mn / nn; // Intersect segment against ’p’ endcap
        else if (md > dd) t = (nd - mn) / nn; // Intersect segment against ’q’ endcap
        else t = 0.0f; // ’a’ lies inside cylinder
        return 1;
    }

    float b = dd * mn - nd * md;
    float discr = b * b - a * c;
    if (discr < 0.0f) return 0; // No real roots; no intersection
    t = (-b - glm::sqrt(discr)) / a;

    if (t < 0.0f || t > 1.0f) return 0; // Intersection lies outside segment

    if (md + t * nd < 0.0f) {
        // Intersection outside cylinder on ’p’ side
        if (nd <= 0.0f) return 0; // Segment pointing away from endcap
        t = -md / nd;
        // Keep intersection if Dot(S(t) - p, S(t) - p) <= r∧2
        return k + 2 * t * (mn + t * nn) <= 0.0f;
    }

    else if (md + t * nd > dd) {
        // Intersection outside cylinder on ’q’ side
        if (nd >= 0.0f) return 0; // Segment pointing away from endcap
        t = (dd - md) / nd;
        // Keep intersection if Dot(S(t) - q, S(t) - q) <= r∧2
        return k + dd - 2 * md + t * (2 * (mn - nd) + t * nn) <= 0.0f;
    }

    // Segment intersects cylinder between the endcaps; t is correct
    return 1;
}

// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting,
// returns t value of intersection and intersection point q
int IntersectRaySphere(Point p, Vector d, Sphere s, float& t, Point& q)
{
    Vector m = p - s.center;
    float b = glm::dot(m, d);
    float c = glm::dot(m, m) - s.radius * s.radius;

    // Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0f && b > 0.0f) return 0;
    float discr = b * b - c;

    // A negative discriminant corresponds to ray missing sphere
    if (discr < 0.0f) return 0;

    // Ray now found to intersect sphere, compute smallest t value of intersection
    t = -b - glm::sqrt(discr);

    // If t is negative, ray started inside sphere so clamp t to zero
    if (t < 0.0f) t = 0.0f;
    q = p + t * d;
    return 1;
}

// Given segment ab and point c, computes closest point d on ab.
// Also returns t for the parametric position of d, d(t)=a+ t*(b - a)
void ClosestPtPointSegment(Point c, Point a, Point b, float& t, Point& d)
{
    Vector ab = b - a;
    // Project c onto ab, but deferring divide by Dot(ab, ab)
    t = glm::dot(c - a, ab);
    if (t <= 0.0f) {
        // c projects outside the [a,b] interval, on the a side; clamp to a
        t = 0.0f;
        d = a;
    } else {
        float denom = glm::dot(ab, ab); // Always nonnegative since denom = ||ab||∧2
        if (t >= denom) {
            // c projects outside the [a,b] interval, on the b side; clamp to b
            t = 1.0f;
            d = b;
        } else {
            // c projects inside the [a,b] interval; must do deferred divide now
            t = t / denom;
            d = a + t * ab;
        }
    }
}

void PrintColliders(glm::vec3 player_move_vec, glm::vec3 player_center)
{
    printf("\nPLAYER MOVE\n");
    printf("playerCenter: %f  %f  %f\n", player_center.x, player_center.y, player_center.z);
    printf("player_move_vec: %f  %f  %f\n", player_move_vec.x, player_move_vec.y, player_move_vec.z);

    for (size_t i = 0; i < potentialColliders.size(); ++i) {
        printf("Face: %d\n", i);
        printf("    Vertices: ");

        for (int j = 0; j < 4; ++j) {
            glm::vec3 vertex = potentialColliders[i].vertices[j];
            printf(" {%.2f,%.2f,%.2f} ", vertex.x, vertex.y, vertex.z);
        }

        printf("\n    Normal: %.2f %.2f %.2f\n", potentialColliders[i].normal.x, potentialColliders[i].normal.y, potentialColliders[i].normal.z);
    }
}

#endif