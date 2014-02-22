#ifndef VRMLLIB_TYPES_H
#define VRMLLIB_TYPES_H

#include <vector>

namespace vrmllib {

struct vec2 {
    float x, y;
};

struct vec3 {
    float x, y, z;
    vec3() {}
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct col3 {
    float r, g, b;
    col3() {}
    col3(float r_, float g_, float b_) : r(r_), g(g_), b(b_) {}
};

class rot {
public:
    rot() : vector(0,0,1), radians(0) {}
    rot(float x, float y, float z, float r) : vector(x,y,z), radians(r) {}

    vec3 vector;
    float radians;
};

} // namespace vrmllib

#endif
