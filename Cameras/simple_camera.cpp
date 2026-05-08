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

#include "simple_camera.h"
#include "contour.h"
#include "hit.h"

#include <thread>
#include <vector>

SimpleCamera::SimpleCamera()
{
  fov = 0.5;
}

SimpleCamera::SimpleCamera(float p_f)
{
  fov = p_f;
}

void SimpleCamera::get_ray_pixel(int p_x, int p_y, Ray &p_ray)
{
  float fx = ((float)p_x + 0.5f)/(float)width;
  float fy = ((float)p_y + 0.5f)/(float)height;
  
  p_ray.position.x = 0.0f;
  p_ray.position.y = 0.0f;
  p_ray.position.z = 0.0f;
  p_ray.position.w = 1.0f;
  p_ray.direction.x = (fx-0.5f);
  p_ray.direction.y = (0.5f-fy);
  p_ray.direction.z = fov;
  p_ray.direction.normalise();

}

void SimpleCamera::render_pixel_group(int x_start, int y_start, int x_end, int y_end, Environment &env, FrameBuffer &fb) {
	for (int y = y_start; y < y_end; y += 1) {
		for (int x = x_start; x < x_end; x += 1)
		{
			//default ray
			Ray ray;

			get_ray_pixel(x, y, ray);

			Colour colour;
			float depth;

			env.raytrace(ray, 5, colour, depth);

			fb.plotPixel(x, y, colour.r, colour.g, colour.b);
			fb.plotDepth(x, y, depth);
		}

		cerr << "#" << flush;
	}
};

void SimpleCamera::render(Environment& env, FrameBuffer& fb)
{
	width = fb.width;
	height = fb.height;

	int no_threads = 64;

	vector<thread> all_threads;

	for (int t = 1; t <= no_threads; t++) {
		/* thread tr([this, &t, &no_threads, &env, &fb]() {
			SimpleCamera::render_pixel_group(0, (height*(t-1))/no_threads, width, (height*t)/no_threads, env, fb);
		}); */
		all_threads.emplace_back(&SimpleCamera::render_pixel_group, this, 0, (height*(t-1))/no_threads, width, (height*t)/no_threads, ref(env), ref(fb));
	}

	for (auto& tr : all_threads) {
		if (tr.joinable()) {
			tr.join();
		}
	}
}