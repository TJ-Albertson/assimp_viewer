#ifndef COLLISION_H
#define COLLISION_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Inputs: plane origin, plane normal, ray origin ray vector.
// NOTE: both vectors are assumed to be normalized
double intersect(glm::vec3 planeOrigin, glm::vec3 planeNormal, glm::vec3 rayOrigin, glm::vec3 rayVector)
{
	double d = -(planeNormal * planeOrigin);
	double numer = planeNormal * rayOrigin + d;
	double denom = planeNormal * rayVector;
	return -(numer / denom)
}


double intersectSphere(glm::vec3 rO, glm::vec3 rV, glm::vec3 sO, double sR)
{
	glm::vec3 Q = sO - rO;
	double c = Q.length();
	double v = Q * rV;
	double d = sR * sR - (c * c - v * v);

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

	sourcePoint *= radiusVector
}


// The collision detection’s recursive routine
void collideWithWorld(glm::vec3 sourcePoint, glm::vec3 velocityVector)
{
	// How far do we need to go?

	double distanceToTravel = velocityVector.length();

	// Do we need to bother?

	if (distanceToTravel < EPSILON) return;

	// Whom might we collide with?

	unsigned int* potentialColliders[10]; // = predetermined list of potential colliders
	unsigned int potentialCollidersSize;

	if (!potentialColliders) {
		sourcePoint += velocityVector;
		return;
	}

	// You'll need to write this routine to deal with your specific data

	scale_potential_colliders_to_ellipsoid_space(radiusVector);

	// Determine the nearest collider from the list potentialColliders

	bool      collisionFound = false;
	double    nearestDistance = -1.0;
	glm::vec3 nearestIntersectionPoint;
	glm::vec3 nearestPolygonIntersectionPoint;

	for (int i = 0; i < potentialCollidersSize; i++) {
		// Plane origin/normal

		glm::vec3 pOrigin; //= any vertex from current poly;
		glm::vec3 pNormal; //= surface noraml (unit vector_ from current poly;

		// Determine the istance from the plane to the source

		double pDist = intersect(pOrigin, pNormal, source, -pNormal);
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
			glm::vec3 temp; //= -pNormal with length set to pDist;
			planeIntersectionPoint = source + temp;
		}
		else
		{
			// Calculate the sphere intersection point

			sphereIntersectionPoint = source - pNormal;

			// Calculate the plane intersection point

			double t = intersect(pOrigin, pNormal, sphereIntersectionPoint /* , Velocity with normalized length*/);

			// Are we traveling away from this polygon?

			if (t < 0.0) continue;

			// Calculate the plane intersection point

			glm::vec3 V; //= velocityVector with length set to t;
			planeIntersectionPoint = sphereIntersectionPoint + V;
		}

		// Unless otherwise noted, out polygonIntersectionPoint is the
		// same point as planeIntersectionPoint

		glm::vec3 polygonIntersectionPoint = planeIntersectionPoint;

		// So.. are they the same?

		if (planeIntersectionPoint is not within the current polygon) 
		{
			polygonIntersectionPoint =// nearest point on polygon's perimeter to planeIntersectionPoint;
		}

		// Invert the velocity vector

		glm::vec3 negativeVelocityVector = -velocityVector;
                // Using the polygonIntersectionPoint, we need to reverse-intersect
                // with the sphere (note: the 1.0 below is the unit-sphere’s
                // radius)
                double t = intersectSphere(sourcePoint, 1.0,
                    polygonIntersectionPoint, negativeVelocityVector);
                // Was there an intersection with the sphere?
                if (t >= 0.0 && t <= distanceToTravel) {
                        // Where did we intersect the sphere?
                        glm::vec3 V = negativeVelocityVector with length set to t;
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
        glm::vec3 V = velocityVector with length set to(nearestDistance - EPSILON);
        sourcePoint += V;
        // What's our destination (relative to the point of contact)?
        Set length of V to(distanceToTravel – nearestDistance);
        glm::vec3 destinationPoint = nearestPolygonIntersectionPoint + V;
        // Determine the sliding plane
        glm::vec3 slidePlaneOrigin = nearestPolygonIntersectionPoint;
        glm::vec3 slidePlaneNormal = nearestPolygonIntersectionPoint - sourcePoint;
        // We now project the destination point onto the sliding plane
        double time = intersect(slidePlaneOrigin, slidePlaneNormal,
            destinationPoint, slidePlaneNormal);
        Set length of slidePlaneNormal to time;
        glm::vec3 destinationProjectionNormal = slidePlaneNormal;
        glm::vec3 newDestinationPoint = destination + destinationProjectionNormal;
        // Generate the slide vector, which will become our new velocity vector
        // for the next iteration
        glm::vec3 newVelocityVector = newDestinationPoint – nearestPolygonIntersectionPoint;
        // Recursively slide (without adding gravity)
        collideWithWorld(sourcePoint, newVelocityVector);
	}
	
}



#endif