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

#include "global_material.h"

#include <math.h>

// iniitlaise the material
GlobalMaterial::GlobalMaterial(Environment* p_env, Colour p_reflect_weight, Colour p_refract_weight, float p_ior)
{

	environment = p_env;
	reflect_weight = p_reflect_weight;
	refract_weight = p_refract_weight;
	ior = p_ior;

}

// compute the kr term using fresenel equations
void GlobalMaterial::fresnel(Vector& view, Vector& normal, float etai, float etat, float& kr)
{
	float cosi = view.dot(-normal);

	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));

	if (sint >= 1) {
		kr = 1;

	} else
	{
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
}

// compute the refracted ray
bool GlobalMaterial::refract_ray(Vector& view, Vector& normal, float ior_d, Vector& ray)
{
	float cosi = view.dot(-normal);

	float k = 1 - ior_d * ior_d * (1 - cosi * cosi);

	if (k < 0.0f)
	{
		return false;
	}

	ray = ior_d * view + (ior_d * cosi - sqrtf(k)) * normal;

	return true;
}


// reflection and recursion computation
Colour GlobalMaterial::compute_once(Ray& viewer, Hit& hit, int recurse)
{
	Colour result;


	recurse = recurse - 1;
	if (recurse < 0)
	{
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
		return result;
	}

	// compute fresnel
	float kr;

	if (hit.entering) // I'm not properly using this term
	{
		fresnel(viewer.direction, hit.normal, 1.0f, ior, kr);
	}
	else
	{
		fresnel(viewer.direction, hit.normal, ior, 1.0f, kr);
	}

	float kt = 1.0f - kr;

	if (reflect_weight.a != 0.0f)
	{
		// compute reflection
		Ray reflect;

		hit.normal.reflection(viewer.direction, reflect.direction);

		reflect.direction.normalise();

		reflect.position = hit.position + (0.0001f * reflect.direction);

		float depth; // don't need this but we have to provide somewhere for the result.
		Colour reflect_result;
		environment->raytrace(reflect, recurse, reflect_result, depth);

		Colour scaleby = reflect_weight * kr;
		reflect_result.scale(scaleby);
		result.add(reflect_result);
	}

	if (refract_weight.a != 0.0f)
	{
		// compute refraction

		Colour refract_result;
		float depth;
		
		float ior_d;

		if (!hit.entering)
		{
			ior_d = ior; // entering this object
		}
		else
		{
			ior_d = 1.0f / ior; // exiting this object
		}

		Ray refract;

		if (refract_ray(viewer.direction, hit.normal, ior_d, refract.direction))
		{
			refract.direction.normalise();

			refract.position = hit.position + (0.0001f * refract.direction);

			environment->raytrace(refract, recurse, refract_result, depth);

			Colour scaleby = refract_weight * kt;
			refract_result.scale(scaleby);
			result.add(refract_result);
		}
	}


	return result;
}

// we don't do per light calculations for reflection and refraction
Colour GlobalMaterial::compute_per_light(Vector& viewer, Hit& hit, Vector& ldir)
{
	Colour result;

	result.r=0.0f;
	result.g=0.0f;
	result.b=0.0f;

	return result;
}

