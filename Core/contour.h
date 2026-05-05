#pragma once

#include "vector.h"
#include "vertex.h"

class Contour {
    public:
        Vertex start;
        Vertex end;
        Vertex other; //this is the other vertex of the triangle from which the contour is formed
        Vector direction;
        Vector normal1;
        Vector normal2;

        Contour() {}

        Contour(Vertex s, Vertex e, Vertex o, Vector dir, Vector norm1, Vector norm2) {
            start = s;
            end = e;
            other = o;
            direction = dir;
            normal1 = norm1;
            normal2 =  norm2;
        }
};