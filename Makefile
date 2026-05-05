CAMERAS = Cameras/full_camera.o Cameras/simple_camera.o
#CORE = Core/framebuffer.o Core/hit.o Core/scene.o
CORE = Core/framebuffer.o Core/scene.o
LIGHTS = Lights/directional_light.o
MATERIALS = Materials/compound_material.o Materials/global_material.o Materials/phong_material.o Materials/falsecolour_material.o
#OBJECTS = Objects/csg_object.o Objects/plane_object.o Objects/polymesh_object.o Objects/sphere_object.o
#OBJECTS = Objects/polymesh_object.o Objects/sphere_object.o Objects/triangle_object.o
OBJECTS = Objects/polymesh_object.o Objects/sphere_object.o

OBJS = $(CAMERAS) $(CORE) $(LIGHTS) $(MATERIALS) $(OBJECTS)

CPPFLAGS = -I Cameras -I Core -I Lights -I Materials -I Objects

%.o : %.cpp
	g++ -O -c $< -o $@ $(CPPFLAGS)

tracer : main.o $(OBJS)
	g++ -O -o $(@) main.o $(OBJS) -lm
