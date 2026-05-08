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

// The PolyMesh Object reads and intersects with triangle meshes

#pragma once

#include "object.h"
#include "ray.h"
#include "contour.h"

#include <string>
#include <map>
#include <unordered_map>

typedef int TriangleIndex[3];

class PolyMesh:public Object{
public:
	int vertex_count;
	int vertex_normal_count;
	int triangle_count;
	int contour_count;
    Vertex *vertex;
	Vector *face_normal;
	Vector *vertex_normal;
	Contour *contours;
	TriangleIndex *triangle;
	map<int, map<int, map<int, Vector>>> triangle_face_normals;
	bool smoothing;

	bool triangle_intersection(Hit *hit, Ray ray, int which_triangle);

	void compute_face_normal(int which_triangle, Vector& normal);
	void compute_vertex_normals(void);
	bool rayTriangleIntersect(const Ray& ray, Vector v0, Vector v1, Vector v2, float& t, float& u, float& v);

	void add_contours();

	bool check_for_contour(Vertex position, Ray hit_ray, int tri_index);

	int strtoint(string s);

	bool intersection(Ray ray, Hit &hit);
	void apply_transform(Transform& trans);

	void recursive_face_traversal(int ind0, int ind1, int ind2, Vertex hit_position, Vector hit_normal, vector<array<int, 3>> &perms_seen, int &towards_camera, int &away_camera, vector<float> &distances, bool &contour_found, int &contour_count, int depth, int &depth_of_contour);

    PolyMesh(const char *file, bool smooth);
	~PolyMesh(){}
};
