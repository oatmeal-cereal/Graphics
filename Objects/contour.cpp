#include "contour.h"
#include "hit.h"

using namespace std;

Contour::Contour(Vertex s, Vertex e, Vector dir, Vector sn, Vector en) {
    start = s;
    end = e;
    direction = dir;
}