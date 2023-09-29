#ifndef COLLISION_H
#define COLLISION_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

const float EPSILON = 1e-2;

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

float intersect(const Point& pOrigin, const Vector& pNormal, const Point& rOrigin, const Vector& rVector);
float intersectSphere(const Point& rayOrigin, const Vector& rayVector, const Point& sphereOrigin, float sphereRadius);

void collisionDetection(Point& sourcePoint, Vector& velocityVector, const Vector& gravityVector, double radiusVector);
void collideWithWorld(Point& sourcePoint, Vector& velocityVector, double radiusVector);

void scalePotentialColliders(double radiusVector);
bool pointWithinPolygon(const Polygon& polygon, const Point& point);
Point nearestPointOnPolygonPerimeter(const Polygon& polygon, const Point& point);

Point ClosestPtPointTriangle(Point p, Point a, Point b, Point c);
int IntersectRaySphere(Point p, Vector d, Point center, float radius, float& t, Point& q);

void collideWithWorldTwo(Point& sourcePoint, Vector& velocityVector);


int IntersectMovingSpherePlane(Sphere s, Vector v, Plane p, float& t, Point& q);
int PointInTriangle(Point p, Point a, Point b, Point c);
int IntersectSegmentCylinder(Point sa, Point sb, Point p, Point q, float r, float& t);
int IntersectRaySphere(Point p, Vector d, Sphere s, float& t, Point& q);

int TriangleCollisionDetection(Sphere s, Vector v);



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

// Function to find the distance between point and plane
float intersect(const Point& pOrigin, const Vector& pNormal, const Point& rOrigin, const Vector& rVector)
{
    float d = -glm::dot(pNormal, pOrigin);
    float numer = glm::dot(pNormal, rOrigin) + d;
    float denom = glm::dot(pNormal, rVector);

    return -(numer / denom);
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

// Function to find the intersection between a ray and a sphere
float intersectSphere(const Point& rayOrigin, const Vector& rayVector, const Point& sphereOrigin, float sphereRadius)
{

    printf("    sphereOrigin: %f %f %f\n", sphereOrigin.x, sphereOrigin.y, sphereOrigin.z);

    //vector points from rayOrigin to sphereOrigin
    Vector Q = sphereOrigin - rayOrigin;
    printf("    rayVector: %f %f %f\n", rayVector.x, rayVector.y, rayVector.z);
    printf("    Vector Q: %f %f %f\n", Q.x, Q.y, Q.z);

    float Q_length = glm::length(Q);
    printf("    Q_length: %f\n", Q_length);
    float v = glm::dot(Q, rayVector); 
    printf("    v: %f\n", v);
    float d = (sphereRadius * sphereRadius) - ((Q_length * Q_length) - (v * v));

    printf("    d: %f\n", d);
    if (d < 0.0)
    {
        printf("    no intersection with sphere\n");
        return -1.0;
    }
       
    printf("    Intersection with sphere\n");

    return v - sqrt(d);
}

// Function to perform collision detection
void collisionDetection(Point& sourcePoint, Vector& velocityVector, const Vector& gravityVector, double radiusVector)
{
    // Add gravity
    velocityVector += gravityVector;

    // Scale inputs
    sourcePoint /= radiusVector;
    velocityVector /= radiusVector;

    // Perform collisions
    collideWithWorld(sourcePoint, velocityVector, radiusVector);
    //collideWithWorldTwo(sourcePoint, velocityVector);

    // Un-scale output
    sourcePoint *= radiusVector;
}

// Function to scale potential colliders to ellipsoid space
void scalePotentialColliders(double radiusVector)
{
    for (Polygon& collider : potentialColliders)
    {
        // Scale each vertex of the collider
        for (Point& vertex : collider.vertices)
        {
            // Scale along the x, y, and z axes
            vertex.x *= scaleFactor;
            vertex.y *= scaleFactor;
            vertex.z *= scaleFactor;
        }

        // Recalculate the normal vector of the polygon
        // This assumes that the polygon's normal was originally unit length
        collider.normal = normalize(collider.normal);
    }
}

// Determine whether plane p intersects sphere s
/*
int TestSpherePlane(Sphere s, Plane p)
{
    // For a normalized plane (|p.n| = 1), evaluating the plane equation
    // for a point gives the signed distance of the point to the plane
    float dist = Dot(s.c, p.n) - p.d;
    // If sphere center within +/-radius from plane, plane intersects sphere
    return Abs(dist) <= s.r;
}
*/


/*
* TODO

1. sphere-plane test
        5.5.3 Intersecting Moving Sphere Against Plane

2. face test (is P inside T?)

3. edge test (cylinder, curved surface only)
        5.3.7 Intersecting Ray or Segment Against Cylinder (last bit of test can be omitted, dont need to check endcaps)

4. vertice test (sphere centered at vertice, return closest)
        5.3.2 Intersecting Ray or Segment Against Sphere

5. if all pass then no intersection
*/




// Function to perform collision detection recursively
void collideWithWorld(Point& sourcePoint, Vector& velocityVector, double radiusVector)
{
    //assert(velocityVector != nullptr);

    float distanceToTravel = glm::length(velocityVector);
    
    if (distanceToTravel < EPSILON) {
        //printf("distanceToTravel < EPSILON\n");
        return;
    }
        

    if (potentialColliders.empty())
    {
        //sourcePoint += velocityVector;
        return;
    }

    //scalePotentialColliders(radiusVector);

    bool collisionFound = false;
    float nearestDistance = -1.0;
    Point nearestIntersectionPoint;
    Point nearestPolygonIntersectionPoint;
    bool edge;

    int i = 0;
    
    for (const Polygon& polygon : potentialColliders)
    {
        i++;
        //printf("Face %d\n", i);
        
        const Point& planeOrigin = polygon.vertices[0];
        const Vector& planeNormal = polygon.normal;

        Plane plane;
        plane.n = planeNormal;
        plane.d = glm::dot(planeNormal, planeOrigin);

        //Point closest_point_on_plane = ClosestPtPointPlane(sourcePoint, plane);

        float point_distance_from_plane = DistPointPlane(sourcePoint, plane);

        Point sphereIntersectionPoint; 
        Point planeIntersectionPoint;

        if (point_distance_from_plane < 0.0)
        {
            continue;
        }
        else if (point_distance_from_plane <= 1.0)
        {
            Vector temp = -planeNormal * point_distance_from_plane;
            planeIntersectionPoint = sourcePoint + temp;
            float t = intersect(planeOrigin, planeNormal, sphereIntersectionPoint, glm::normalize(velocityVector));


            if (t < 0.0) {
                continue;
            }
        }
        else
        {
            sphereIntersectionPoint = sourcePoint - planeNormal;
            float t = intersect(planeOrigin, planeNormal, sphereIntersectionPoint, glm::normalize(velocityVector));

            if (t < 0.0)
            {
                continue;
            }

            Vector V = glm::normalize(velocityVector) * t;
            planeIntersectionPoint = sphereIntersectionPoint + V;
        }

        //sphereIntersectionPointPos = sphereIntersectionPoint;

        Point polygonIntersectionPoint = planeIntersectionPoint;

        polygonIntersectionPoint = ClosestPtPointTriangle(sourcePoint, polygon.vertices[1], polygon.vertices[2], polygon.vertices[0]);
        collisionBallPosition = polygonIntersectionPoint;

        Vector negativeVelocityVector = -velocityVector;
        //float t = intersectSphere(polygonIntersectionPoint, negativeVelocityVector, sourcePoint, 1.0f);

        // Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting,
        // returns t value of intersection and intersection point q
        float t;
        Point q;
        Vector Q = glm::normalize(sourcePoint - polygonIntersectionPoint);
        int bruh = IntersectRaySphere(polygonIntersectionPoint, Q, sourcePoint, 1.0f, t, q);

        sphereIntersectionPointPos = q;

        if (t >= 0.0 && t <= distanceToTravel)
        {
            Vector V = negativeVelocityVector * t;
            Point intersectionPoint = q; // polygonIntersectionPoint + V;

            if (!collisionFound || t < nearestDistance)
            {
                nearestDistance = t;
                nearestIntersectionPoint = intersectionPoint;
                nearestPolygonIntersectionPoint = polygonIntersectionPoint;
                collisionFound = true;
            }
        }
    }

    if (!collisionFound)
    {
        sourcePoint += velocityVector;
        return;
    }
        
    
    



    Vector V = glm::normalize(velocityVector) * (nearestDistance - EPSILON);
    //velocityVector = V;
    //sourcePoint += V;
    //printf("    velocityVector: %f %f %f\n", velocityVector.x, velocityVector.y, velocityVector.z);

    V = glm::normalize(V) * (distanceToTravel - nearestDistance);
    Point destinationPoint = nearestPolygonIntersectionPoint + V;

    Point slidePlaneOrigin = nearestPolygonIntersectionPoint;
    Vector slidePlaneNormal = nearestPolygonIntersectionPoint - sourcePoint;

    glm::vec3 dest_point = sourcePoint + velocityVector;
    glm::vec3 ray_plane_intersect_point; //= functionToFindRayIntersectPlane(
    glm::vec3 dest_point_plane; //= functionToFindDistPointPlane(dest_point, )

    float time = intersect(slidePlaneOrigin, slidePlaneNormal, destinationPoint, slidePlaneNormal); 

    slidePlaneNormal = glm::normalize(slidePlaneNormal) * time;

    Vector destinationProjectionNormal = slidePlaneNormal;

    Point newDestinationPoint = destinationPoint + destinationProjectionNormal;
    
    Vector newVelocityVector = newDestinationPoint - nearestPolygonIntersectionPoint;

    if (i == 26) {
        //printf("    newVelocityVector: %f %f %f\n", newVelocityVector.x, newVelocityVector.y, newVelocityVector.z);
    }
    
    // maybe set this gange to same length of vector but in sliding direction
    // 
    // velocity and newVelocityVector are going against each toerh
    // 
    // its gott be dis>>>>>>
    // sourcePoint += new VelocityVector
    //velocityVector = newVelocityVector;
    //newVelocityVector = 
    sourcePoint += glm::vec3(newVelocityVector.x, 0.0f, newVelocityVector.z);
    //collideWithWorld(sourcePoint, newVelocityVector, radiusVector);
}

// Function to check if a point is within a polygon
bool pointWithinPolygon(const Polygon& polygon, const Point& point)
{
    int numVertices = static_cast<int>(polygon.vertices.size());
    bool inside = false;

    for (int i = 0, j = numVertices - 1; i < numVertices; j = i++)
    {
        const Point& vertex1 = polygon.vertices[i];
        const Point& vertex2 = polygon.vertices[j];

        // Check if the point is inside the edge defined by vertex1 and vertex2
        if (((vertex1.y > point.y) != (vertex2.y > point.y)) &&
            (point.x < (vertex2.x - vertex1.x) * (point.y - vertex1.y) / (vertex2.y - vertex1.y) + vertex1.x))
        {
            inside = !inside;
        }
    }

    return inside;
}

// Function to find the nearest point on a polygon's perimeter to a given point
Point nearestPointOnPolygonPerimeter(const Polygon& polygon, const Point& point)
{
    int numVertices = static_cast<int>(polygon.vertices.size());
    double minDistance = distance(polygon.vertices[0], point);
    Point nearestPoint = polygon.vertices[0];

    for (int i = 1; i < numVertices; ++i)
    {
        double dist = distance(polygon.vertices[i], point);

        if (dist < minDistance)
        {
            minDistance = dist;
            nearestPoint = polygon.vertices[i];
        }
    }

    return nearestPoint;
}

int IntersectRaySphere(Point p, Vector d, Point center, float radius, float& t, Point& q)
{
    Vector m = p - center;
    float b = glm::dot(m, d);
    float c = glm::dot(m, m) - radius * radius;
    // Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0f && b > 0.0f)
        return 0;
    float discr = b * b - c;
    // A negative discriminant corresponds to ray missing sphere
    if (discr < 0.0f)
        return 0;
    // Ray now found to intersect sphere, compute smallest t value of intersection
    t = -b - sqrt(discr);
    // If t is negative, ray started inside sphere so clamp t to zero
    if (t < 0.0f)
        t = 0.0f;
    q = p + t * d;
    return 1;
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


/*
* TODO

1. sphere-plane test
        5.5.3 Intersecting Moving Sphere Against Plane

2. face test (is P inside T?)

3. edge test (cylinder, curved surface only)
        5.3.7 Intersecting Ray or Segment Against Cylinder (last bit of test can be omitted, dont need to check endcaps)

4. vertice test (sphere centered at vertice, return closest)
        5.3.2 Intersecting Ray or Segment Against Sphere

5. if all pass then no intersection
*/

int TriangleCollisionDetection(Sphere s, Vector v) {
    


    for (int j = 0; j < potentialColliders.size(); ++j) {

        Plane p;
        p.n = potentialColliders[j].normal;
        p.d = glm::dot(p.n, potentialColliders[j].vertices[0]);

        Triangle t;
        t.vertices[0] = potentialColliders[j].vertices[0];
        t.vertices[1] = potentialColliders[j].vertices[1];
        t.vertices[2] = potentialColliders[j].vertices[2];

        float time_of_collision;
        Point collision_point;

        // 1. sphere-plane test
        int planeCollision = IntersectMovingSpherePlane(s, v, p, time_of_collision, collision_point);
        printf("time_of_collision: %f\n", time_of_collision);

        if (!planeCollision) {
            //printf("NO Collision with Polygon %d Plane\n", j);
            continue;
        }

        // 2. face test
        int point_in_triangle = PointInTriangle(collision_point, t.vertices[0], t.vertices[1], t.vertices[2]);

        if (point_in_triangle) {
            // earliest intersection point found
            printf("Collision with Polygon %d Face\n", j);
            return 1;
        }


        // 3. edge test
        int edge_collision;
        

        for (int i = 0; i < 3; ++i) {

            int vertex1 = i;
            int vertex2 = (i + 1) % 3;

            float time;

            edge_collision = IntersectSegmentCylinder(s.c, v, t.vertices[vertex1], t.vertices[vertex2], s.r, time);

            if (edge_collision && time < time_of_collision) {
                time_of_collision = time;
                //S(t)=sa + t(sb-sa)
                collision_point = s.c + (time * (v - s.c));
               
            }
        }

        if (edge_collision) {
            printf("Collision with Polygon %d Edge\n", j);
            return 1;
        }

        // 4. vertice test
        int vertex_collision;
        int vertex = -1;
        float least_time = 1;

        for (int i = 0; i < 3; ++i) {

            Sphere temp;
            temp.c = t.vertices[i];
            temp.r = s.r;

            
            float time;
            Point point;

            vertex_collision = IntersectRaySphere(s.c, v, temp, time, point);

            //fix finding shortest time

            //if plane collision then time == 0 so need to reset
            if (vertex_collision && time < least_time) {
                least_time = time;
                time_of_collision = time;
                collision_point = point;
                vertex = i;
            }
        }

        if (vertex_collision) {
            printf("Collision with Polygon %d Vertex %d\n", j, vertex);
            return 1;
        }

        // 5. No collision
       // printf("NO Collision with Polygon %d\n", j);
    }

    return 0;
}


#endif