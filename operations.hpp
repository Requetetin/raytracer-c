#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace glm;

vec3 crossProd(vec3 v0, vec3 v1) {
    //cout << to_string(v0) << " and " << to_string(v1) << endl;
    float x = v0.y * v1.z - v0.z * v1.y;
    float y = v0.z * v1.x - v0.x * v1.z;
    float z = v0.x * v1.y - v0.y * v1.x;
    //cout << x << " " << y << " " << z << endl;
    return {
        x,
        y,
        z
    };
}

float dotProd(vec3 v0, vec3 v1) {
    //cout << to_string(v0) << " and " << to_string(v1) << endl;
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

float vecLength(vec3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 norm(vec3 v) {
    float length = vecLength(v);

    if (!length) {
        return {0, 0, 0};
    }

    return {
        v.x/length, v.y/length, v.z/length
    };
}

vec3 barycentric(vec2 A, vec2 B, vec2 C, vec2 P) {
    vec3 cross = crossProd(
        {
            B.x - A.x, C.x - A.x, A.x - P.x
        }, 
        {
            B.y - A.y, C.y - A.y, A.y - P.y
        }
    );
    //cout << cross.x << " " << cross.y << " " << cross.z << endl;
    if (abs(cross.z) < 1) {
        return {-1, -1, -1};
    }

    float u = cross.x / cross.z;
    float v = cross.y / cross.z;
    float w = 1 - (cross.x + cross.y) / cross.z;
    return {w, v, u};
}

class Material {
    public:
        vec3 diffuse;

        Material(vec3 difuso) {
            diffuse = difuso;
        }

};