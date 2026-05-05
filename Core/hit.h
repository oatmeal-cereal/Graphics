/****************************************************************************
*
* krt - Ken's Raytracer - Coursework Edition. (C) Copyright 1993-2025.
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

// Hit is a class to store and maniplulate information about an intersection
// between a ray and an object.

#pragma once

#include <iostream>
using namespace std;

#include "vertex.h"
#include "vector.h"
#include "tex_coords.h"

#include <vector>

class Object;

class Hit {
public:
	float     t;			// The intersection distance
	Object   *what;		    	// the primitive object that has been hit
	Vertex    position;	    	// the position of intersection
	Vector    normal;		// the interpolated normal at the point of intersection
	TexCoords coordinates;  	// the texture coordinates at the point of intersection
	bool      entering;		// use to track entering and leaving an object.
	bool 	contour; //if it is near a contour and thus should be coloured black
};
