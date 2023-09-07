#ifndef COLLISION_c_H
#define COLLISION_c_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 radiusVector = glm::vec3(1.0f, 0.0f, 1.0f);

float EPSILON = 0.001f;

struct Collider {
	glm::vec3* vertices;
	glm::vec3  normal;
};

Collider*     potentialColliders; // = predetermined list of potential colliders
unsigned int  num_potentialColliders;

// Inputs: plane origin, plane normal, ray origin ray vector.
// NOTE: both vectors are assumed to be normalized
double intersect(glm::vec3 planeOrigin, glm::vec3 planeNormal, glm::vec3 rayOrigin, glm::vec3 rayVector)
{
	double d = -(planeNormal * planeOrigin);
	double numer = planeNormal * rayOrigin + d;
	double denom = planeNormal * rayVector;
	return -(numer / denom);
}


double intersectSphere(glm::vec3 rayOrigin, glm::vec3 rV, glm::vec3 sphereOrigin, double sphereRadius)
{
	glm::vec3 Q = sphereOrigin - rayOrigin;
	double c = Q.length();
	double v = Q * rV;
	double d = sphereRadius * sphereRadius - (c * c - v * v);

	// If there was no intersection, return -1

	if (d < 0.0) return -1.0;

	// Return the distance to the [first] intersecting point

	return v - sqrt(d);
}


// collision detection entry point
void collisionDetection(glm::vec3 sourcePoint, glm::vec3 velocityVector, glm::vec3 gravityVector)
{
	// We need to do any pre-collision detection work here. Such as adding
	// gravity to our velocity vector. We want to do it in this
	// separate routine because the following routine is recursive, and we
	// don't want to recursively add gravity.
	
	// Add gravity

	velocityVector += gravityVector;

	// At this point, we'll scale our inputs to the collision routine

	sourcePoint /= radiusVector;
	velocityVector /= radiusVector;

	collideWithWorld(sourcePoint, velocityVector);

	sourcePoint *= radiusVector;
}


// The collision detection’s recursive routine
void collideWithWorld(glm::vec3 sourcePoint, glm::vec3 velocityVector)
{
	// How far do we need to go?

	float distanceToTravel = velocityVector.length();

	// Do we need to bother?

	if (distanceToTravel < EPSILON) return;

	if (!potentialColliders) {
		sourcePoint += velocityVector;
		return;
	}

	// You'll need to write this routine to deal with your specific data

	scale_potential_colliders_to_ellipsoid_space(radiusVector);

	// Determine the nearest collider from the list potentialColliders
	bool      collisionFound = false;
	float     nearestDistance = -1.0;
	glm::vec3 nearestIntersectionPoint;
	glm::vec3 nearestPolygonIntersectionPoint;

	for (int i = 0; i < num_potentialColliders; i++) 
	{
		// Plane origin/normal
			
		glm::vec3 pOrigin = potentialColliders[i].vertices[0]; //= any vertex from current poly;
		glm::vec3 pNormal = potentialColliders[i].normal; //= surface normal (unit vector_ from current poly;

		// Determine the distance from the plane to the source
		float pDist = intersect(pOrigin, pNormal, sourcePoint, -pNormal);
		glm::vec3 sphereIntersectionPoint;
		glm::vec3 planeIntersectionPoint;

		// Is the source point behind the plane?
		//
		// [note that you can remove this condition if your visuals are not
		// using backface culling]
		if (pDist < 0.0)
		{
			continue;
		}

		// Is the plane embedded (i.e. within the disace of 1.0 for out
		// unit sphere)?
		else if (pDist <= 1.0)
		{
			// Calculate the plane intersection point
			glm::vec3 temp = glm::normalize(-pNormal) * pDist; //= -pNormal with length set to pDist;
			planeIntersectionPoint = sourcePoint + temp;
		}
		else
		{
			// Calculate the sphere intersection point
			sphereIntersectionPoint = sourcePoint - pNormal;

			// Calculate the plane intersection point
			float t = intersect(pOrigin, pNormal, sphereIntersectionPoint, glm::normalize(velocityVector));

			// Are we traveling away from this polygon?
			if (t < 0.0) continue;

			// Calculate the plane intersection point
			glm::vec3 V = glm::normalize(velocityVector) * t; //= velocityVector with length set to t;
			planeIntersectionPoint = sphereIntersectionPoint + V;
		}

		// Unless otherwise noted, our polygonIntersectionPoint is the
		// same point as planeIntersectionPoint

		glm::vec3 polygonIntersectionPoint = planeIntersectionPoint;

		// So, are they the same?
		if (planeIntersectionPoint is not within the current polygon) 
		{
			polygonIntersectionPoint;// =nearest point on polygon's perimeter to planeIntersectionPoint;
		}

		// Invert the velocity vector

		glm::vec3 negativeVelocityVector = -velocityVector;

        // Using the polygonIntersectionPoint, we need to reverse-intersect
        // with the sphere (note: the 1.0 below is the unit-sphere’s
        // radius)
        float t = intersectSphere(sourcePoint, 1.0, polygonIntersectionPoint, negativeVelocityVector);

        // Was there an intersection with the sphere?
        if (t >= 0.0 && t <= distanceToTravel) {
			// Where did we intersect the sphere?
			glm::vec3 V = glm::normalize(negativeVelocityVector) * t;
            glm::vec3 intersectionPoint = polygonIntersectionPoint + V;
            // Closest intersection thus far?
            if (!collisionFound || t < nearestDistance) {
				nearestDistance = t;
				nearestIntersectionPoint = intersectionPoint;
				nearestPolygonIntersectionPoint = polygonIntersectionPoint;
                collisionFound = true;
            }
        }
    }

    // If we never found a collision, we can safely move to the destination
    // and bail
    if (!collisionFound) {
		sourcePoint += velocityVector;
		return;
    }

    // Move to the nearest collision
    //glm::vec3 V = velocityVector with length set to(nearestDistance - EPSILON);
	glm::vec3 V = glm::normalize(velocityVector) * (nearestDistance - EPSILON);
    sourcePoint += V;

    // What's our destination (relative to the point of contact)?
    //Set length of V to(distanceToTravel – nearestDistance);
	V = glm::normalize(V) * (distanceToTravel - nearestDistance);
	glm::vec3 destinationPoint = nearestPolygonIntersectionPoint + V;

    // Determine the sliding plane
    glm::vec3 slidePlaneOrigin = nearestPolygonIntersectionPoint;
    glm::vec3 slidePlaneNormal = nearestPolygonIntersectionPoint - sourcePoint;

    // We now project the destination point onto the sliding plane
    float time = intersect(slidePlaneOrigin, slidePlaneNormal, destinationPoint, slidePlaneNormal);

    //Set length of slidePlaneNormal to time;
	slidePlaneNormal = glm::normalize(slidePlaneNormal) * time;
	glm::vec3 destinationProjectionNormal = slidePlaneNormal;
    glm::vec3 newDestinationPoint = destination + destinationProjectionNormal;

    // Generate the slide vector, which will become our new velocity vector
    // for the next iteration
    glm::vec3 newVelocityVector = newDestinationPoint – nearestPolygonIntersectionPoint;

    // Recursively slide (without adding gravity)
    collideWithWorld(sourcePoint, newVelocityVector);
}

void scale_potential_colliders_to_ellipsoid_space(glm::vec3 radiusVector) {

}

#endif