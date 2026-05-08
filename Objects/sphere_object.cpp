/****************************************************************************
*
* krt - Ken's Raytracer - Coursework Edition. (C) Copyright 1993-2022.
*
* I've put a lot of time and effort into this code. For the last decade
* it's been used to introduce hundreds of students at multiple universities
* to raytracing. It forms the basis of your coursework but you are free
* to continue using/developing forever more. However, I ask that you don't
* share the code or your derivitive versions publicly. In order to continue
* to be used for coursework and in particular assessment it's important that
* versions containing solutions are not searchable on the web or easy to
* download.
*
* If you want to show off your programming ability, instead of releasing
* the code, consider generating an incredible image and explaining how you
* produced it.
*/

#include "sphere_object.h"
#include <math.h>


Sphere::Sphere(Vertex c, float r)
{
	center = c;
	radius = r;
}

bool Sphere::intersection(Ray ray, Hit &hit)
{

	Vector ro;

	// offset ray by sphere position
	// equivalent to transforming ray into local sphere space

	ro.x = ray.position.x - center.x;
	ro.y = ray.position.y - center.y;
	ro.z = ray.position.z - center.z;

	float a = (float)ray.direction.dot(ray.direction);
	float b = (float)(2.0 * ray.direction.dot(ro));
	float c = (float)ro.dot(ro) - radius*radius;

	float disc = b*b - 4 * a*c;

	if (disc < 0.0f)
	{
		return false; // a negative value indicates no intersection.
	}

	// an intersection has been found, record details in hit objects

	float ds = sqrtf(disc);

	float t0 = (-b - ds) / 2.0f;
	float t1 = (-b + ds) / 2.0f;

	if (t0 < 0.0f)
	{
		if (t1 < 0.0f)
		{
			return false;
		}
		else
		{
			t0 = t1;
		}
	}

	hit.what = this;
	hit.t = t0;

	hit.position = ray.position + hit.t * ray.direction;
	hit.normal = hit.position - center;
	hit.normal.normalise();

	float ro_length = Vertex(ro.x, ro.y, 0).length();

	if (fabs(radius - ro_length) < 0.1f) {
		hit.contour = true;
	}

	if (hit.normal.dot(ray.direction) > 0.0)
	{
		hit.normal.negate();
	}

	return true;
}

void Sphere::apply_transform(Transform& transform)
{
	transform.apply(center);
}

bool Sphere::check_for_contour(Vertex hit_position, Ray hit_ray) {
	Vertex center2d = Vertex(center.x, center.y, 0);
	Vertex hit_pos_2d = Vertex(hit_position.x, hit_position.y, 0);

	float dist = (center2d - hit_pos_2d).length();

	if (radius - dist < 0.1f) {
		return true;
	}
	return false;
}