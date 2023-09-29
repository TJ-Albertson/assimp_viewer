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
    glm::vec3 newVelocityVector = newDestinationPoint - nearestPolygonIntersectionPoint;

    // Recursively slide (without adding gravity)
    collideWithWorld(sourcePoint, newVelocityVector);
}

void scale_potential_colliders_to_ellipsoid_space(glm::vec3 radiusVector) {

}







typedef glm::vec3 Point;
typedef glm::vec3 Vector;

struct Plane {
	Vector n; // Plane normal. Points x on the plane satisfy Dot(n,x) = d
	float d; // d = dot(n,p) for a given point p on the plane
};

struct Sphere {
	Point c;
	float r;
};

struct Triangle {
	Point vertices[3];
};

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
int ClosestPtPointTriangle(Point p, Point a, Point b, Point c)
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
		// Keep intersection if Dot(S(t) - p, S(t) - p) <= r?2
		return k + 2 * t * (mn + t * nn) <= 0.0f;
	}

	else if (md + t * nd > dd) {
		// Intersection outside cylinder on ’q’ side
		if (nd >= 0.0f) return 0; // Segment pointing away from endcap
		t = (dd - md) / nd;
		// Keep intersection if Dot(S(t) - q, S(t) - q) <= r?2
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
int CollisionDetection(Sphere s, Vector v, Plane p, Triangle t) {

	float time_of_collision;
	Point collision_point;

	// 1. sphere-plane test
	int planeCollision = IntersectMovingSpherePlane(s, v, p, time_of_collision, collision_point);

	if (!planeCollision) return 0;

	// 2. face test
	int point_in_triangle = ClosestPtPointTriangle(collision_point, t.vertices[0], t.vertices[1], t.vertices[2]);

	if (point_in_triangle) {
		// earliest intersection point found
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
		return 1;
	}

	// 4. vertice test
	int vertex_collision;

	for (int i = 0; i < 3; i++) {

		Sphere temp;
		temp.c = t.vertices[i];
		temp.r = s.r;

		float time;
		Point point;

		vertex_collision = IntersectRaySphere(s.c, v, temp, time, point);

		if (vertex_collision && time < time_of_collision) {
			time_of_collision = time;
			collision_point = point;
		}
	}

	if (vertex_collision) {
		return 1;
	}

	// 5. No collision
	return 0;
}

// Adjust direction and magnitude of velocity vector (using slide plane and time of collision)
void CollisionResponse(Sphere sphere, Vector& vector, Point collision_point) {

	
}

#endif