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

/* This is the top level for the program you need to create for lab three and four.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// these are core raytracing classes
#include "framebuffer.h"
#include "scene.h"
#include "material_colour.h"

// classes that contain our objects to be rendered, all derived from Object
#include "sphere_object.h"
#include "polymesh_object.h"

// classes that contain our lights, all derived from Light
#include "directional_light.h"

// classes that contain the materials applied to an object, all derived from Material
#include "phong_material.h"
#include "falsecolour_material.h"
#include "compound_material.h"
#include "global_material.h"

//classes that contain cameras, all derived from Camera
#include "simple_camera.h"

using namespace std;

// you will find it useful during development/debugging to create multiple functions that fill out the scene.
void build_scene(Scene& scene)
{
	int skull_scale = 1.0f;
	// The following transform allows 4D homogeneous coordinates to be transformed.It moves the supplied teapot model to somewhere visible.
	Transform * transform = new Transform(skull_scale, 0.0f, 0.0f, 0.0f,
										0.0f, 0.0f, skull_scale, 0.0f,
										0.0f, skull_scale, 0.0f, 0.0f,
										0.0f, 0.0f, 0.0f, 1.0f);

	//rotation around the z-axis
	float phi = 75.0f;
	Transform * rotation3 = new Transform(cos(phi), -1.0f * sin(phi), 0.0f, 0.0f,
										sin(phi), cos(phi), 0.0f, 0.0f,
										0.0f, 0.0f, 1.0f, 0.0f,
										0.0f, 0.0f, 0.0f, 1.0f);

	Transform * translation = new Transform(1.0f, 0.0f, 0.0f, 0.0f,
										0.0f, 1.0f, 0.0f, -12.0f,
										0.0f, 0.0f, 1.0f, 50.0f,
										0.0f, 0.0f, 0.0f, 1.0f);

	Phong* bp = new Phong(new MaterialColour(0.1f, 0.1f, 0.1f,1.0f), new MaterialColour(0.3f, 0.3f, 0.3f,1.0f), new MaterialColour(0.6f, 0.6f, 0.6f,1.0f), 120.0f);

	Phong* bp2 = new Phong(new MaterialColour(0.0f, 0.0f, 0.0f,1.0f), new MaterialColour(0.0f, 0.0f, 0.0f,1.0f), new MaterialColour(0.6f, 0.6f, 0.6f,1.0f), 120.0f);

	/* PolyMesh* teapot = new PolyMesh("teapot.obj",true);
        teapot->apply_transform(*transform);
	teapot->set_material(bp);
	scene.add_object(teapot); */

	Phong* flat = new Phong(new MaterialColour(0.1f, 0.1f, 0.1f, 1.0f), new MaterialColour(0.3f, 0.3f, 0.3f, 1.0f), new MaterialColour(0.6f, 0.6f, 0.6f, 1.0f), 5.0f);

	PolyMesh* skull = new PolyMesh("skull-low.obj", true);
	//skull->apply_transform(*rotation2);
	skull->apply_transform(*rotation3);
    skull->apply_transform(*transform);
	skull->apply_transform(*translation);
	skull->set_material(flat);
	scene.add_object(skull);

	CompoundMaterial* cm = new CompoundMaterial(2);

	GlobalMaterial* gm = new GlobalMaterial(&scene, Colour(0.9f,0.9f,0.9f,1.0f), Colour(0.9f,0.9f,0.9f,1.0f), 1.1f);
	
	cm->include_material(bp2);
	cm->include_material(gm);

	Sphere* s2 = new Sphere(Vertex(1.25f,0.0f,10.0f),1.0f);
	s2->set_material(cm);
	scene.add_object(s2);


	DirectionalLight* dl = new DirectionalLight(Vector(-1.0f, -1.0f, 0.5f), Colour(1.0f, 1.0f, 1.0f, 0.0f));

	scene.add_light(dl);
}


// This is the entry point function to the program.
int main(int argc, char *argv[])
{
	int width = 512;
	int height = 512;
	// Create a framebuffer
	FrameBuffer* fb = new FrameBuffer(width, height);
	
	// Create a scene
	Scene scene;
	
	// Setup the scene
	build_scene(scene);
	
	// Declare a camera
	Camera *camera = new SimpleCamera(1.4f);
	
	// Camera generates rays for each pixel in the framebuffer and records colour + depth.
	camera->render(scene,*fb);
	
	// Output the framebuffer colour and depth as two images
	fb->writeRGBFile((char *)"test.ppm");
	fb->writeDepthFile((char *)"depth.ppm");
	
	cerr << "\nDone.\n" << flush;

	return 0;
}
