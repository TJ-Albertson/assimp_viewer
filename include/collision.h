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

struct Polygon
{
    std::vector<Point> vertices;
    Vector normal;
};

std::vector<Polygon> potentialColliders;

double intersect(const Point& pOrigin, const Vector& pNormal, const Point& rOrigin, const Vector& rVector);
double intersectSphere(const Point& rO, const Vector& rV, const Point& sO, double sR);

void collisionDetection(Point& sourcePoint, Vector& velocityVector, const Vector& gravityVector, double radiusVector);
void collideWithWorld(Point& sourcePoint, Vector& velocityVector, double radiusVector);

void scalePotentialColliders(double radiusVector);
bool pointWithinPolygon(const Polygon& polygon, const Point& point);
Point nearestPointOnPolygonPerimeter(const Polygon& polygon, const Point& point);

// Function to find the intersection point between a ray and a plane
double intersect(const Point& pOrigin, const Vector& pNormal, const Point& rOrigin, const Vector& rVector)
{
    double d = -dot(pNormal, pOrigin);
    double numer = dot(pNormal, rOrigin) + d;
    double denom = dot(pNormal, rVector);

    return -(numer / denom);
}



// Function to find the intersection between a ray and a sphere
double intersectSphere(const Point& rO, const Vector& rV, const Point& sO, double sR)
{
    Vector Q = sO - rO;
    double c = length(Q);
    double v = dot(Q, rV);
    double d = sR * sR - (c * c - v * v);

    if (d < 0.0)
        return -1.0;

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

// Define Polygon as a data structure representing your polygons


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

// Function to perform collision detection recursively
void collideWithWorld(Point& sourcePoint, Vector& velocityVector, double radiusVector)
{
    double distanceToTravel = length(velocityVector);

    if (distanceToTravel < EPSILON)
        return;

    if (potentialColliders.empty())
    {
        sourcePoint += velocityVector;
        return;
    }

    scalePotentialColliders(radiusVector);

    bool collisionFound = false;
    float nearestDistance = -1.0;
    Point nearestIntersectionPoint;
    Point nearestPolygonIntersectionPoint;

    for (const Polygon& polygon : potentialColliders)
    {
        const Point& pOrigin = polygon.vertices[0];
        const Vector& pNormal = polygon.normal;

        float pDist = intersect(pOrigin, pNormal, sourcePoint, -pNormal);
        Point sphereIntersectionPoint;
        Point planeIntersectionPoint;

        if (pDist < 0.0)
        {
            continue;
        }
        else if (pDist <= 1.0)
        {
            Vector temp = -pNormal * pDist;
            planeIntersectionPoint = sourcePoint + temp;
        }
        else
        {
            sphereIntersectionPoint = sourcePoint - pNormal;
            float t = intersect(pOrigin, pNormal, sphereIntersectionPoint, velocityVector);

            if (t < 0.0)
                continue;

            Vector V = velocityVector * t;
            planeIntersectionPoint = sphereIntersectionPoint + V;
        }

        Point polygonIntersectionPoint = planeIntersectionPoint;

        if (!pointWithinPolygon(polygon, planeIntersectionPoint))
        {
            polygonIntersectionPoint = nearestPointOnPolygonPerimeter(polygon, planeIntersectionPoint);
        }

        Vector negativeVelocityVector = -velocityVector;
        float t = intersectSphere(sourcePoint, negativeVelocityVector, polygonIntersectionPoint, 1.0);

        if (t >= 0.0 && t <= distanceToTravel)
        {
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
        return;
    }

    Vector V = glm::normalize(velocityVector) * (nearestDistance - EPSILON);
    sourcePoint += V;

    V = nearestPolygonIntersectionPoint - sourcePoint;
    Point destinationPoint = nearestPolygonIntersectionPoint + V;

    Point slidePlaneOrigin = nearestPolygonIntersectionPoint;
    Vector slidePlaneNormal = nearestPolygonIntersectionPoint - sourcePoint;
    
    double time = intersect(slidePlaneOrigin, slidePlaneNormal, destinationPoint, slidePlaneNormal);
    
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


#endif