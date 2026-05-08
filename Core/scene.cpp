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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "scene.h"
#include <vector>

Scene::Scene()
{
	obj_count = 0;
	object_list = 0;
	light_list = 0;
}

// test for intersection with a shadow ray.
// the distance to the light can be used as a limit
bool Scene::shadowtrace(Ray ray, float limit)
{
	Hit hit;
	Object *objects = this->object_list;

	while (objects != 0)
	{
		if (objects->intersection(ray, hit) == true)
		{
		  if ((hit.t > 0.00000001f) && ( hit.t < limit))
		  {
		      return true; // this object is in the path, no need to check any others.
		  }

		}

		objects = objects->next;
	}

	return false; // none of the objects in the scene cast a shadow here.
}

bool Scene::trace(Ray ray, Hit &best_hit)
{
	Hit hit; // this is the hit structure we will pass to intersection tests to fill out
	bool did_hit = false; // did we hit something?
	best_hit.t = 100000000.0f; // assume a non-exisitent hit that's far away.

	Object *objects = this->object_list;

	while (objects != 0)
	{
		if (objects->intersection(ray, hit) == true)
		{
			
			if ((hit.t >= 0.0f) && (hit.t < best_hit.t)) // we've found a better hit, save it
			{
				best_hit.t = hit.t;
				best_hit.what= hit.what;
				best_hit.position = hit.position;
				best_hit.normal = hit.normal;
				best_hit.coordinates = hit.coordinates;
				best_hit.contour = hit.contour;
				did_hit = true; //we hit something, best_hit now valid
			}
		}

		objects = objects->next;
	}

	return did_hit;
}

float quantise_pixelcolour(float c) {
	if (c < 0.1f) {
		return 0.1f;
	} else if (c < 0.25f) {
		return 0.3f;
	} else if (c < 0.5f) {
		return 0.5f;
	} else if (c < 0.75f) {
		return 0.7f;
	} else {
		return 0.9f;
	}
}

/* float quantise_pixelcolour(float c) {
	if (c < 0.1f) {
		return 0.2f;
	} else if (c < 0.25f) {
		return 0.3f;
	} else if (c < 0.5f) {
		return 0.5f;
	} else if (c < 0.9f) {
		return 0.7f;
	} else {
		return 0.9f;
	}
} */

void Scene::raytrace(Ray ray, int recurse, Colour &colour, float &depth)
{
	Hit best_hit;

	Object *objects = object_list;
	Light *lights = light_list;
		
	// a default colour if we hit nothing.
	colour.r = 0.0f;
	colour.g = 0.0f;
	colour.b = 0.0f;
	colour.a = 0.0f;
	depth = 0.0f;

	// first step, find the closest primitive
	// if we found a primitive then compute the colour we should see
	if (this->trace(ray, best_hit) == true)
	{
		depth = best_hit.t;

		//there is a contour
		bool contour = best_hit.contour;

		if (!contour) {
			colour = colour + best_hit.what->material->compute_once(ray, best_hit, recurse); // this will be the global components such as ambient or reflect/refract
		}
		
		// next, compute the light contribution for each light in the scene.
		Light* light = light_list;
		while (light != (Light*)0)
		{
			Vector viewer;
			Vector ldir;

			viewer = ray.direction;

			bool lit;
			lit = light->get_direction(best_hit.position, ldir);

			Vector tolight;
			tolight = ldir;
			tolight.negate();

			if (tolight.dot(best_hit.normal) < 0.0)
			{
				lit = false;//light is facing wrong way.
			}

			// if lit==true and in shadow, set lit=false
			if (lit)
			{
				Ray shadow_ray;

				shadow_ray.direction = tolight;

				shadow_ray.position = best_hit.position + (0.001f * shadow_ray.direction);

				if (this->shadowtrace(shadow_ray, 10000000.0f))
				{
					lit = false; //there's a shadow so no lighting
				}
			}


			if (lit) // still lit, so do per light calculation
			{
				Colour intensity;

				light->get_intensity(best_hit.position, intensity);
				if (!contour) {
					colour = colour + intensity * best_hit.what->material->compute_per_light(viewer, best_hit, ldir); // this is the per light local contrib e.g. diffuse, specular
				
					colour.r = quantise_pixelcolour(colour.r);
					colour.g = quantise_pixelcolour(colour.g);
					colour.b = quantise_pixelcolour(colour.b);
				}
			}

			light = light->next;

		}

	} else {

		colour.r = 0.7f;
		colour.g = 0.7f;
		colour.b = 1.0f;

		colour.a = 1.0f;
	}
}

void Scene::add_object(Object *obj)
{
  obj->next = this->object_list;
  this->object_list = obj;
  obj_count += 1;
}

void Scene::add_light(Light *light)
{
  light->next = this->light_list;
  this->light_list = light;
}
