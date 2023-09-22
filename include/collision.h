#ifndef COLLISION_H
#define COLLISION_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

const float EPSILON = 1e-6;

// Define Point and Vector as glm types
typedef glm::vec3 Point;
typedef glm::vec3 Vector;


float scaleFactor = 1;


struct Polygon {
    std::vector<Point> vertices;
    Vector normal;
};

std::vector<Polygon> potentialColliders;

float intersect(const Point& pOrigin, const Vector& pNormal, const Point& rOrigin, const Vector& rVector);
float intersectSphere(const Point& rayOrigin, const Vector& rayVector, const Point& sphereOrigin, float sphereRadius);

void collisionDetection(Point& sourcePoint, Vector& velocityVector, const Vector& gravityVector, double radiusVector);
void collideWithWorld(Point& sourcePoint, Vector& velocityVector, double radiusVector);

void scalePotentialColliders(double radiusVector);
bool pointWithinPolygon(const Polygon& polygon, const Point& point);
Point nearestPointOnPolygonPerimeter(const Polygon& polygon, const Point& point);




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

// Function to find the intersection between a ray and a sphere
float intersectSphere(const Point& rayOrigin, const Vector& rayVector, const Point& sphereOrigin, float sphereRadius)
{
    //vector points from rayOrigin to sphereOrigin
    Vector Q = sphereOrigin - rayOrigin;
    printf("    Vector Q: %f %f %f\n", Q.x, Q.y, Q.z);

    float Q_length = Q.length();

    float v = glm::dot(Q, rayVector); 
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

// Function to perform collision detection recursively
void collideWithWorld(Point& sourcePoint, Vector& velocityVector, double radiusVector)
{
    float distanceToTravel = length(velocityVector);

    if (distanceToTravel < EPSILON) {
        printf("distanceToTravel < EPSILON\n");
        return;
    }
        

    if (potentialColliders.empty())
    {
        printf("potentialColliders is Empty\n");
        sourcePoint += velocityVector;
        return;
    }

    //scalePotentialColliders(radiusVector);

    bool collisionFound = false;
    float nearestDistance = -1.0;
    Point nearestIntersectionPoint;
    Point nearestPolygonIntersectionPoint;

    int i = 0;
    printf("\n\n------------------------\n\n");
    for (const Polygon& polygon : potentialColliders)
    {
        i++;
        printf("Face %d\n", i);
        
        const Point& planeOrigin = polygon.vertices[0];
        const Vector& planeNormal = polygon.normal;

        float point_distance_from_plane = intersect(planeOrigin, planeNormal, sourcePoint, -planeNormal);

        printf("    point_distance_from_plane: %f\n", point_distance_from_plane);
        Point sphereIntersectionPoint; 
        Point planeIntersectionPoint;

        if (point_distance_from_plane < 0.0)
        {
            printf("    source point behind plane\n");
            continue;
        }
        else if (point_distance_from_plane <= 1.0)
        {
            
            printf("    within 1.0f\n");
            Vector temp = -planeNormal * point_distance_from_plane;
            planeIntersectionPoint = sourcePoint + temp;
        }
        else
        {
            sphereIntersectionPoint = sourcePoint - planeNormal;
            float t = intersect(planeOrigin, planeNormal, sphereIntersectionPoint, glm::normalize(velocityVector));

            if (t < 0.0)
            {
                printf("    traveling away from this polygon\n");
                continue;
            }
                

            Vector V = glm::normalize(velocityVector) * t;
            planeIntersectionPoint = sphereIntersectionPoint + V;
        }

        Point polygonIntersectionPoint = planeIntersectionPoint;

        printf("    planeIntersectionPoint: %f %f %f\n", planeIntersectionPoint.x, planeIntersectionPoint.y, planeIntersectionPoint.z);

        if (!pointWithinPolygon(polygon, planeIntersectionPoint))
        {
            polygonIntersectionPoint = nearestPointOnPolygonPerimeter(polygon, planeIntersectionPoint);
        }

        printf("    point within polygon\n");

        Vector negativeVelocityVector = -velocityVector;
        //float intersectSphere(const Point& rayOrigin, const Vector& rayVector, const Point& sphereOrigin, float sphereRadius);
        float t = intersectSphere(polygonIntersectionPoint, negativeVelocityVector, sourcePoint, 1.0f);
       // float t = intersectSphere(sourcePoint, 1.0f, polygonIntersectionPoint, 1.0f);

        printf("    t: %f\n", t);
        printf("    distanceToTravel: %f\n", distanceToTravel);

        if (t >= 0.0 && t <= distanceToTravel)
        {
            printf("    t >= 0.0 && t <= distanceToTravel\n");
            Vector V = negativeVelocityVector * t;
            Point intersectionPoint = polygonIntersectionPoint + V;

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
        //printf("Collision Not Found!\n");
        return;
    }

    printf("!!!!!!!!!!!!!!!!!!!Collision Found!!!!!!!!!!!!!!!!!!!\n");

    Vector V = glm::normalize(velocityVector) * (nearestDistance - EPSILON);
    sourcePoint += V;

    V = nearestPolygonIntersectionPoint - sourcePoint;
    Point destinationPoint = nearestPolygonIntersectionPoint + V;

    Point slidePlaneOrigin = nearestPolygonIntersectionPoint;
    Vector slidePlaneNormal = nearestPolygonIntersectionPoint - sourcePoint;
    
    float time = intersect(slidePlaneOrigin, slidePlaneNormal, destinationPoint, slidePlaneNormal);
    
    slidePlaneNormal *= time;
    Vector destinationProjectionNormal = slidePlaneNormal;
    Point newDestinationPoint = destinationPoint + destinationProjectionNormal;
    
    
    Vector newVelocityVector = newDestinationPoint - nearestPolygonIntersectionPoint;
    
    collideWithWorld(sourcePoint, newVelocityVector, radiusVector);
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

    Polygon polygons[6];

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
            int point1, point2, point3, point4;
            sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &point1, &point2, &point3, &point4);

            polygons[faceCount].vertices.push_back(vertices[point1 - 1]);
            polygons[faceCount].vertices.push_back(vertices[point2 - 1]);
            polygons[faceCount].vertices.push_back(vertices[point3 - 1]);
            polygons[faceCount].vertices.push_back(vertices[point4 - 1]);

            polygons[faceCount].normal = normals[faceCount];

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


    for (int i = 0; i < 6; ++i)
    {
        potentialColliders.push_back(polygons[i]);
    }
    
    return;
}

#endif