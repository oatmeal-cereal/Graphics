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

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>

#include "polymesh_object.h"

using namespace std;

// turn a string into an integer. I should probably use a library for this.
int PolyMesh::strtoint(string s)
{
    int r=0;
    for(int i = 0;i < s.length();i+=1)
    {
        if ((s[i] == ' ') || (s[i] == '/'))
        {
            return r;
        } else
{
            r = r*10 + (s[i]-'0');
        }
    }
    return r;
}


// When the file is read, only the vertices and the faces are used. The normals and texture coordinates are ignored.
// Normals are re-constructed from the faces.
PolyMesh::PolyMesh(const char* file, bool smooth)
{
    int count;
    int xpos,ypos,zpos;
    ifstream meshfile(file);

    // I don't recommend hard-coding the storage size, but I know how bit the files will be.
    vertex = new Vertex[100000];
    triangle = new TriangleIndex[100000];
    contours = new Contour[100000];
    //vertex_normal = new Vector[100000];

    triangle_face_normals = {};

    vertex_count = 0;
    vertex_normal_count = 0;
    triangle_count = 0;
    contour_count = 0;

    smoothing = smooth;

    cerr << "Opening meshfile: " << file << endl;

    if (!meshfile.is_open())
    {
        cerr << "Problem reading meshfile (not found)." << endl;
        meshfile.close();
        exit(-1);
    }

    string line;

    while(!meshfile.eof())
    {
        try {
            getline(meshfile, line);
        }
        catch (ifstream::failure e)
        {
            cerr << "Problem reading meshfile (getline failed)." << endl;
        }

	if (line.size() == 0)
        {
            // Empty line
            continue;
        }
        switch(line[0])
        {
            case '#': // comment, do nothing
                break;
            case 'f': // face
                if (line[1] != ' ')
                {
                    cerr << "Malformed face line:" << line << endl;
                    break;
                }
                // try to parse faces.
                int values[8];
                int c;
                int np,lp;
                lp = line.find(" ") + 1;
                for (c = 0;;)
                {
                    if ((line[lp] < '0') || (line[lp] >'9')) // not a valid number, so stop.
                    {
                        break;
                    }
                    values[c] = strtoint(line.substr(lp)); // convert to int
                    c += 1;

                    np = line.find(" ", lp); // look for the next space.
                    if (np == std::string::npos) // end of string
                    {
                        break;
                    }
                    lp = np + 1;
                }

                triangle[triangle_count][0] = values[0]-1;
                triangle[triangle_count][1] = values[1]-1;
                triangle[triangle_count][2] = values[2]-1;
                triangle_count += 1;

                if (c > 3)
                {
                    triangle[triangle_count][0] = values[2]-1;
                    triangle[triangle_count][1] = values[3]-1;
                    triangle[triangle_count][2] = values[0]-1;
                    triangle_count += 1;
                }

                break;
            case 'v': // vertex property
                switch(line[1])
                {
                case ' ': // a vertex
                    xpos = line.find(" ")+1;
                    ypos = line.find(" ",xpos)+1;
                    zpos = line.find(" ",ypos)+1;

                    vertex[vertex_count].x = stof(line.substr(xpos,ypos-xpos-1));

                    vertex[vertex_count].y = stof(line.substr(ypos,zpos-ypos-1));

                    vertex[vertex_count].z = stof(line.substr(zpos));// + 50.0f;
                    vertex[vertex_count].w = 1.0;
                    vertex_count += 1;

                    break;
                case 'n': // a normal
                    /* {
                        //updated part of code
                        if (line[3] == ' ') {
                            xpos = 4;
                        } else {
                            xpos = 3;
                        }
                        //old part
                        //xpos = line.find(" ")+1; //start of the xpos
                        ypos = line.find(" ",xpos)+1;
                        zpos = line.find(" ",ypos)+1;

                        Vector vn = Vector(stof(line.substr(xpos, ypos-xpos-1)), stof(line.substr(ypos,zpos-ypos-1)), stof(line.substr(zpos)));
                        vertex_normal[vertex_normal_count] = vn;

                        cout << vertex_normal[vertex_normal_count].x << "\n";

                        vertex_normal_count += 1;
                    } */
                    break;
                case 't': // texture coordinates, ignore them.
                    break;
                default:
                    cerr << "Malformed vertex related line in meshfile:" << line << endl;
                    break;
                };
                break;
            default: // don't know how to process, so skip
                cerr << "Unhandled line in meshfile (ignoring):" << line << endl;
                break;
        };
    }

    cerr << "Found " << vertex_count << " vertices." << endl;
    cerr << "Found " << triangle_count << " triangles." << endl;
    meshfile.close();
    cerr << "Meshfile read." << endl;

    // generate face normals

    face_normal = new Vector[triangle_count];

    for (int i = 0; i < triangle_count; i += 1)
    {
        compute_face_normal(i, face_normal[i]);
    }

    // if required compute vertex normals, as we ignored any supplied in the file.

    if (smooth)
    {
        vertex_normal = new Vector[vertex_count];
        compute_vertex_normals();
    }
    else
    {
        vertex_normal = 0;
    }

    cerr << "Meshfile processed." << endl;
    next = 0;
}

// This is the main traingle/ray intersection test.
// It uses the Moller-Trumbore approach.
// This computes the barycentric coordinates (u,v)
// if they are valid, then we're within the e triangle
// we can then also use them to interpolate values across the triangle
bool PolyMesh::rayTriangleIntersect(const Ray& ray, Vector v0, Vector v1, Vector v2, float& t, float& u, float& v)
{
    Vector p;
    Vector d;
    Vector edge1, edge2, h, s, q;
    float a, f;

    p.x = ray.position.x;
    p.y = ray.position.y;
    p.z = ray.position.z;
    d = ray.direction;

    edge1 = v1 - v0;
    edge2 = v2 - v0;

    // calculare determinate
    d.cross(edge2, h);

    // check we're not parallel to the plane
    a = edge1.dot(h);
    if (a > -0.00001f && a < 0.00001f)
    {
        return false;
    }

    // calculate u
    f = 1 / a;
    s = p - v0;
    u = f * s.dot(h);

    // calculate v
    s.cross(edge1, q);
    v = f * d.dot(q);

    // compute t

    t = f * edge2.dot(q);

    // and check we are in range
    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    // check we are in range for v and third parameter
    if ((v < 0.0f) || ((u + v) > 1.0f))
    {
        return false;
    }

    if (t < 0.00001f) // it's behind you
    {
      return false;
    }

    return true;
}

// compute the vertex normals from the face normals.
void PolyMesh::compute_vertex_normals(void)
{

    // The vertex_normal array is already zeroed.

    // accumulate all the face normals for triangles that share a vertex.
    for (int i = 0; i < triangle_count; i += 1)
    {
        for (int j = 0; j < 3; j += 1)
        {
            vertex_normal[triangle[i][j]] = vertex_normal[triangle[i][j]] + face_normal[i];
        }
    }

    // normalise the result.
    for (int i = 0; i < vertex_count; i += 1)
    {
        vertex_normal[i].normalise();
    }
}

string vertex_string_builder(Vertex v1, Vertex v2, Vertex v3) {
    return to_string(v1.x) + '-' + to_string(v1.y) + '-' + to_string(v1.z) + '-' + to_string(v2.x) + '-' + to_string(v2.y) + '-' + to_string(v2.z) + '-' + to_string(v3.x) + '-' + to_string(v3.y) + '-' + to_string(v3.z);
}

// compute face normals for a triangle.
void PolyMesh::compute_face_normal(int which_triangle, Vector& normal)
{
    int i0 = triangle[which_triangle][0];
    int i1 = triangle[which_triangle][1];
    int i2 = triangle[which_triangle][2];
    Vertex v0 = vertex[i0];
    Vertex v1 = vertex[i1];
    Vertex v2 = vertex[i2];

    Vector v0v1, v0v2;

    v0v1 = v1 - v0;
    //v0v1.normalise();

    v0v2 = v2 - v0;
    //v0v2.normalise();

    v0v1.cross(v0v2, normal);
    normal.normalise();

    //string newvertices = vertex_string_builder(v1, v2, v3);

    //indices to vertices go to normal
    triangle_face_normals[i0][i1][i2] = normal;
    triangle_face_normals[i0][i2][i1] = normal;
    triangle_face_normals[i2][i1][i0] = normal;
    triangle_face_normals[i2][i0][i1] = normal;
    triangle_face_normals[i1][i2][i0] = normal;
    triangle_face_normals[i1][i0][i2] = normal;
}

void PolyMesh::add_contours() {
    for (int tri = 0; tri < triangle_count; tri += 1) {
        Vector current_normal = face_normal[tri];

        int i0 = triangle[tri][0];
        int i1 = triangle[tri][1];
        int i2 = triangle[tri][2];

        Vertex v0 = vertex[i0];
        Vertex v1 = vertex[i1];
        Vertex v2 = vertex[i2];

        for (auto const& [id, othernorm] : triangle_face_normals[i0][i1]) {
            //if the dot product between the adjacent normal and the ray is less than 0, the adjacent face is not visible
            if (othernorm != current_normal) {
                Contour shared_edge1 = Contour(v0, v1, v2, v1 - v0, current_normal, othernorm);
                contours[contour_count] = shared_edge1;
                contour_count += 1;
                /* Vertex other = vertex[id];
                Contour shared_edge2 = Contour(v0, v1, other, v1 - v0, current_normal, othernorm);
                contours[contour_count] = shared_edge2;
                contour_count += 1; */
            }
        }

        for (auto const& [id, othernorm] : triangle_face_normals[i0][i2]) {
            if (othernorm != current_normal) {
                Contour shared_edge1 = Contour(v0, v2, v1, v2 - v0, current_normal, othernorm);
                contours[contour_count] = shared_edge1;
                contour_count += 1;
                /* Vertex other = vertex[id];
                Contour shared_edge2 = Contour(v0, v2, other, v2 - v0, current_normal, othernorm);
                contours[contour_count] = shared_edge2;
                contour_count += 1; */
            }
        }

        for (auto const& [id, othernorm] : triangle_face_normals[i1][i2]) {
            if (othernorm != current_normal) {
                Contour shared_edge1 = Contour(v1, v2, v0, v2 - v1, current_normal, othernorm);
                contours[contour_count] = shared_edge1;
                contour_count += 1;
                /* Vertex other = vertex[id];
                Contour shared_edge2 = Contour(v1, v2, other, v2 - v1, current_normal, othernorm);
                contours[contour_count] = shared_edge2;
                contour_count += 1; */
            }
        }
    }

    cout << contour_count << "\n";
}

float shortest_distance(Vertex hit_position, Ray contour) {
    Vertex hit_pos_2d = Vertex(hit_position.x, hit_position.y, 0);
    Vertex cont_pos_2d = Vertex(contour.position.x, contour.position.y, 0);
    Vector cont_dir_2d = Vector(contour.direction.x, contour.direction.y, 0);
    //need hit position and ray direction to calculate this, maybe only use the (x, y)
    float raylambda = ((hit_pos_2d.dot(cont_dir_2d)) - (cont_dir_2d.dot(cont_pos_2d))) / cont_dir_2d.dot(cont_dir_2d);
    Vertex raycoord = cont_pos_2d + (raylambda * cont_dir_2d);
    //should get the shortest x-y axis distance, rather than the full xyz distance
    //float shortestdist = (raycoord - hit.position).length();
    float shortest2ddist = (raycoord - hit_pos_2d).length();

    return shortest2ddist;
}

//counts the number of vectors that are facing the camera, i.e. 0, 1 or 2
int count_towards_camera(Vector norm1, Vector norm2, Vector hit_norm) {
    int towards = 0;
    if (hit_norm.dot(norm1) > 0.0f) {
        towards += 1;
    }
    if (hit_norm.dot(norm2) > 0.0f) {
        towards += 1;
    }
    return towards;
}

bool find_perm(array<int, 3> item, vector<array<int, 3>> permutations) {
    for (auto const& perm : permutations) {
        if (perm == item) {
            return true;
        }
    }
    return false;
}

void PolyMesh::recursive_face_traversal(int ind0, int ind1, int ind2, Vertex hit_position, Vector hit_normal, vector<array<int, 3>> &perms_seen, int &towards_camera, vector<float> &distances, bool &contour_found, int depth) {
    if (depth < 2) {
        for (auto const& [indt, othernorm] : triangle_face_normals[ind0][ind1]) {
            //check that ind0, ind1, indt in the permutation
            array<int, 3> perm = {ind0, ind1, indt};
            //if the permutation is not already seen
            if (!(find_perm(perm, perms_seen))) {
                perms_seen.push_back({ind0, ind1, ind2});
                perms_seen.push_back({ind0, ind2, ind1});
                perms_seen.push_back({ind1, ind0, ind2});
                perms_seen.push_back({ind1, ind2, ind0});
                perms_seen.push_back({ind2, ind0, ind1});
                perms_seen.push_back({ind2, ind1, ind0});
                Ray contour_ray = Ray(vertex[ind0], vertex[ind1] - vertex[ind0]);
                float dist = shortest_distance(hit_position, contour_ray);
                //othernorm = triangle_face_normals[perm0][perm1][indt]
                int camera_count = count_towards_camera(othernorm, triangle_face_normals[ind0][ind1][ind2], hit_normal);
                towards_camera += camera_count;
                if (camera_count == 1) {
                    distances.push_back(dist);
                    contour_found = true;
                }
                //call functions to continue the recursion
                recursive_face_traversal(ind0, indt, ind1, hit_position, hit_normal, perms_seen, towards_camera, distances, contour_found, depth + 1);
                recursive_face_traversal(ind1, indt, ind0, hit_position, hit_normal, perms_seen, towards_camera, distances, contour_found, depth + 1);
            }
        }
    } else {
        return;
    }
}

bool PolyMesh::check_for_contour(Vertex hit_position, Ray hit_ray, int tri_index) {
    Vector hit_normal = hit_ray.direction;

    //the size of the margin could depend on how many of the faces are towards the camera, and how many are away?
    float margin = 3.0f;

    int i0 = triangle[tri_index][0];
    int i1 = triangle[tri_index][1];
    int i2 = triangle[tri_index][2];

    Vector current_normal = face_normal[tri_index];

    int start_permutations[3][3] = {{i0, i1, i2}, {i0, i2, i1}, {i1, i2, i0}};

    vector<array<int, 3>> permutations_seen = {};

    bool contour_found = false;

    int towards_camera = 0;

    vector<float> contour_distances = {};

    //we are only checking neighbouring contours rather than all
    //it stands for 'index other'
    //if any return contour as true then break out of the loop and return 'contour'
    for (auto const& [perm0, perm1, permother] : start_permutations) {
        //start the recursion here
        recursive_face_traversal(perm0, perm1, permother, hit_position, hit_normal, permutations_seen, towards_camera, contour_distances, contour_found, 0);
    }

    if (!contour_found) {
        return false;
    }

    //for faces with less neighbouring faces towards the camera, we want to be stricter on the margin, so that there is a thicker border around the whole mesh
    for (float dist : contour_distances) {
        if (dist < margin/towards_camera) {
            return true;
        }
    }

    return false;
}

// this function wraps the basic test into something that fills out a hit.
bool PolyMesh::triangle_intersection(Hit *hit, Ray ray, int which_triangle)
{
    Vector fn = face_normal[which_triangle];

    float ndotdir = fn.dot(ray.direction);

    if (fabs(ndotdir) < 0.000000001f)
    {
        // ray is parallel to triangle so does not intersect
        return false;
    }
	
    float t, u, v;

    int i0 = triangle[which_triangle][0];
    int i1 = triangle[which_triangle][1];
    int i2 = triangle[which_triangle][2];

    Vertex v1 = vertex[i0];
    Vertex v2 = vertex[i1];
    Vertex v3 = vertex[i2];
    
    bool flag = rayTriangleIntersect(ray, v1, v2, v3, t, u, v);

    if (flag == false) return false;

    hit->t = t;
    hit->what = this;

    hit->position = ray.position + t * ray.direction;

    /*
    // equivalent using the barycentric coordinates
    // you could uses this for texture coordinates or anytyhing else.

    Vector pos = u * vertex[triangle[which_triangle][1]]
            + v * vertex[triangle[which_triangle][2]]
            + (1 - u - v) * vertex[triangle[which_triangle][0]];

    hit->position.x = pos.x;
    hit->position.y = pos.y;
    hit->position.z = pos.z;
    hit->position.w = 1.0f;

	*/

    if (smoothing)
    {
      // we interploate the three normals
        hit->normal = u * vertex_normal[triangle[which_triangle][1]]
            + v * vertex_normal[triangle[which_triangle][2]]
            + (1 - u - v) * vertex_normal[triangle[which_triangle][0]];
    }
    else
    {
      // just use the face normal
        hit->normal = face_normal[which_triangle];
    }

    hit->normal.normalise();

    if (hit->normal.dot(ray.direction) > 0.0)
    {
        hit->normal.negate();
    }

    return true;
}


bool PolyMesh::intersection(Ray ray, Hit &hit)
{
    int i;
    bool did_hit = false;

    hit.t = 1000000.0; // ~infinity 

    // Check each triangle, record closest +ve hit

    for (i = 0; i < triangle_count; i += 1)
    {
		Hit next_hit;
        bool hitt = triangle_intersection(&next_hit, ray, i);

        if (hitt)
        {
            did_hit = true; // we hit a triangle

            if ((next_hit.t < hit.t) && (next_hit.t > 0.0f)) // if this traingle is closer, save it
            {
				hit.what = this;
				hit.t = next_hit.t;
				hit.position = next_hit.position;
				hit.normal = next_hit.normal;

                //contour stuff with triangle index
                bool contour_found = check_for_contour(hit.position, ray, i);

                hit.contour = contour_found;
            }
        }
    }

    return did_hit; // this will be true if we hit atleast one triangle.
}

void PolyMesh::apply_transform(Transform& trans)
{
    for (int i = 0; i < vertex_count; i += 1)
    {
        trans.apply(vertex[i]);
    }

    Transform ti = trans.inverse().transpose();

    if (smoothing)
    {

        for (int i = 0; i < vertex_count; i += 1)
        {
            ti.apply(vertex_normal[i]);
        }
    }

    for (int i = 0; i < triangle_count; i += 1)
    {
        int i0, i1, i2;
        i0 = triangle[i][0];
        i1 = triangle[i][1];
        i2 = triangle[i][2];
        trans.apply(triangle_face_normals[i0][i1][i2]);
        trans.apply(triangle_face_normals[i0][i2][i1]);
        trans.apply(triangle_face_normals[i1][i0][i2]);
        trans.apply(triangle_face_normals[i1][i2][i0]);
        trans.apply(triangle_face_normals[i2][i1][i0]);
        trans.apply(triangle_face_normals[i2][i0][i1]);

        ti.apply(face_normal[i]);
    }
    return;
}
