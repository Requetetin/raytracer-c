#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace glm;
using namespace std;

class Sphere {
  public: 
    vec3 center;
    float radius;
    vec3 material;

    Sphere(vec3 centro, float radio, Material m) {
      center = centro;
      radius = radio;
      material = m.diffuse;
    }

    bool rayIntersect(vec3 origin, vec3 direction) {
      vec3 L = center - origin;
      float tca = dotProd(L, direction);
      float l = vecLength(L);
      float d2 = pow(l, 2) - pow(tca, 2);
      if (d2 > pow(radius, 2)) {
        return false;
      }

      float thc = sqrt(pow(radius, 2) - d2);

      float t0 = tca - thc;

      float t1 = tca + thc;

      if (t0 < 0) {
        t0 = t1;
      }
      if (t0 < 0) {
        return false;
      }

      return true;
    }
};
