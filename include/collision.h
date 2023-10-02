﻿#ifndef COLLISION_H
#define COLLISION_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

const float EPSILON = 1e-5;

// Define Point and Vector as glm types
typedef glm::vec3 Point;
typedef glm::vec3 Vector;


float scaleFactor = 1;


struct Polygon {
    std::vector<Point> vertices;
    Vector normal;
};

struct Sphere {
    Point c;
    float r;
};

struct Triangle {
    Point vertices[3];
};
struct Plane {
    Vector n; // Plane normal. Points x on the plane satisfy Dot(n,x) = d
    float d; // d = dot(n,p) for a given point p on the plane
};

std::vector<Polygon> potentialColliders;

glm::vec3 collisionBallPosition;
glm::vec3 sphereIntersectionPointPos;

Point ClosestPtPointTriangle(Point p, Point a, Point b, Point c);
int IntersectRaySphere(Point p, Vector d, Point center, float radius, float& t, Point& q);

int IntersectMovingSpherePlane(Sphere s, Vector v, Plane p, float& t, Point& q);
int PointInTriangle(Point p, Point a, Point b, Point c);
int IntersectSegmentCylinder(Point sa, Point sb, Point p, Point q, float r, float& t);
int IntersectRaySphere(Point p, Vector d, Sphere s, float& t, Point& q);

int CollisionDetection(Sphere s, Vector v);

void print_colliders(glm::vec3 player_move_vec, glm::vec3 player_center)
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

void create_hitbox(std::string const& path, glm::vec3 translation, glm::vec3 scale)
{
    const char* file_path = path.c_str();

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    int vertexCount = 0;
    int normalCount = 0;
    int faceCount = 0;

    glm::vec3 vertices[1000];
    glm::vec3 normals[1000];

    Polygon polygons[12];

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') {
                float x, y, z;
                sscanf(line, "v %f %f %f", &x, &y, &z);
                vertices[vertexCount] = (glm::vec3(x, y, z) + translation) * scale;
                vertexCount++;
            } else if (line[1] == 'n') {
                float nx, ny, nz;
                sscanf(line, "vn %f %f %f", &nx, &ny, &nz);
                normals[normalCount] = glm::vec3(nx, ny, nz);
                normalCount++;
            }
        } else if (line[0] == 'f') {
            int point1, point2, point3;
            sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &point1, &point2, &point3);

            polygons[faceCount].vertices.push_back(vertices[point1 - 1]);
            polygons[faceCount].vertices.push_back(vertices[point2 - 1]);
            polygons[faceCount].vertices.push_back(vertices[point3 - 1]);
            
            if (normalCount == 1) {
                polygons[faceCount].normal = normals[0];
            } else if (faceCount > 5) {
                polygons[faceCount].normal = normals[faceCount - 6];
            } else {
                polygons[faceCount].normal = normals[faceCount];
            }

            faceCount++;
        }
    }

    fclose(file);

    /*
    for (int i = 0; i < faceCount; ++i) {
        printf("Face: %d\n", i);
        printf("    Vertices: ");

        for (int j = 0; j < 4; ++j) {
            glm::vec3 vertex = polygons[i].vertices[j];
            printf("{%.2f,%.2f,%.2f} ", vertex.x, vertex.y, vertex.z);
        }
        printf("\n    Normal: %.2f %.2f %.2f\n", normals[i].x, normals[i].y, normals[i].z);
    }
    printf("\n");
    */


    for (int i = 0; i < faceCount; ++i)
    {
        potentialColliders.push_back(polygons[i]);
    }
    
    return;
}


// this method breaks when sphere is moving parallel to tri because there is no point on plane to find
Point ClosestPtPointTriangle(Point p, Point a, Point b, Point c)
{
    // Check if P in vertex region outside A
    Vector ab = b - a;
    Vector ac = c - a;
    Vector ap = p - a;
    float d1 = glm::dot(ab, ap);
    float d2 = glm::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
        return a; // barycentric coordinates (1,0,0)

    // Check if P in vertex region outside B
    Vector bp = p - b;
    float d3 = glm::dot(ab, bp);
    float d4 = glm::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3)
        return b; // barycentric coordinates (0,1,0)

    // Check if P in edge region of AB, if so return projection of P onto AB
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return a + v * ab; // barycentric coordinates (1-v,v,0)
    }

    // Check if P in vertex region outside C
    Vector cp = p - c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6)
        return c; // barycentric coordinates (0,0,1)

    // Check if P in edge region of AC, if so return projection of P onto AC
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return a + w * ac; // barycentric coordinates (1-w,0,w)
    }

    // Check if P in edge region of BC, if so return projection of P onto BC
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b); // barycentric coordinates (0,1-w,w)
    }

    // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f-v-w
}


// Intersect sphere s with movement vector v with plane p. If intersecting
// return time t of collision and point q at which sphere hits plane
int IntersectMovingSpherePlane(Sphere s, Vector v, Plane p, float& t, Point& q)
{
    // Compute distance of sphere center to plane
    float dist = glm::dot(p.n, s.c) - p.d;
    if (glm::abs(dist) <= s.r) {
        // The sphere is already overlapping the plane. Set time of
        // intersection to zero and q to sphere center
        t = 0.0f;
        q = s.c;
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
            float r = dist > 0.0f ? s.r : -s.r;
            t = (r - dist) / denom;
            q = s.c + t * v - r * p.n;
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
    Vector m = p - s.c;
    float b = glm::dot(m, d);
    float c = glm::dot(m, m) - s.r * s.r;

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
void CollisionResponse(Vector& velocity, Sphere sphere, Point collision_point)
{
    if (glm::length(velocity) < EPSILON) {
        return;
    }

    Vector sliding_plane_normal = sphere.c - collision_point;
    sliding_plane_normal = glm::normalize(sliding_plane_normal);

    Plane sliding_plane;
    sliding_plane.n = sliding_plane_normal;
    sliding_plane.d = glm::dot(sliding_plane_normal, collision_point);

    float distance = DistPointPlane(sphere.c + velocity, sliding_plane);

    Vector newDestinationPoint = sphere.c + velocity - distance * sliding_plane_normal;
    Vector newVelocityVector = newDestinationPoint - collision_point;

    if (glm::length(newVelocityVector) < EPSILON) {
        velocity = glm::vec3(0);
        return;
    }

    velocity = newVelocityVector;
}


int CollisionDetection(Sphere s, Vector& v, Point& collision_point)
{

    for (int j = 0; j < potentialColliders.size(); ++j) {

        Plane p;
        p.n = potentialColliders[j].normal;
        p.d = glm::dot(p.n, potentialColliders[j].vertices[0]);

        Triangle t;
        t.vertices[0] = potentialColliders[j].vertices[0];
        t.vertices[1] = potentialColliders[j].vertices[1];
        t.vertices[2] = potentialColliders[j].vertices[2];

        float time_of_collision;

        // 1. sphere-plane test
        int planeCollision = IntersectMovingSpherePlane(s, v, p, time_of_collision, collision_point);
        //printf("time_of_collision: %f\n", time_of_collision);

        if (!planeCollision || time_of_collision > 1) {
            continue;
        }

        // 2. face test
        int point_in_triangle = PointInTriangle(collision_point, t.vertices[0], t.vertices[1], t.vertices[2]);

        if (point_in_triangle) {
            // earliest intersection point found
            printf("Collision with Polygon %d Face\n", j);
            CollisionResponse(v, s, collision_point);
            continue;
        }

        // 3. edge test
        bool edge_collision = false;
        int edge = -1;
        
        float least_time = 1;

        Point a, b;

        for (int i = 0; i < 3; ++i) {

            int vertex1 = i;
            int vertex2 = (i + 1) % 3;

            float time;

            int edge_intersect = IntersectSegmentCylinder(s.c, s.c+v, t.vertices[vertex1], t.vertices[vertex2], s.r, time);

            if (edge_intersect && time <= least_time) {
                edge_collision = true;
                least_time = time;
                time_of_collision = time;
                edge = i;
                a = t.vertices[vertex1];
                b = t.vertices[vertex2];
            }
        }


        if (edge_collision) {

            float time;
            Point d;

            ClosestPtPointSegment(s.c+v, a, b, time, d);
            collision_point = d;
            CollisionResponse(v, s, collision_point);
            printf("Collision with Polygon %d Edge %d\n", j, edge);
            continue;
        }

        // 4. vertice test
        bool vertex_collision = false;
        
        int vertex = -1;
        least_time = 1;

        for (int i = 0; i < 3; ++i) {

            Sphere temp;
            temp.c = t.vertices[i];
            temp.r = s.r;
            
            float time;
            Point point;

            int vertex_intersection = IntersectRaySphere(s.c, v, temp, time, point);

            //if plane collision then time == 0 so need to reset
            if (vertex_intersection && time < least_time) {
                vertex_collision = true;
                least_time = time;
                time_of_collision = time;

                collision_point = t.vertices[i];
                vertex = i;
            }
        }

        if (vertex_collision) {
            printf("Collision with Polygon %d Vertex %d\n", j, vertex);
            CollisionResponse(v, s, collision_point);
            continue;
        }

        // 5. No collision
    }
    collisionBallPosition = collision_point;
    return 0;
}

#endif