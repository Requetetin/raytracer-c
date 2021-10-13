#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace glm;
using namespace std;

class Sphere {
  public: 
    vec3 center;
    double radius;
    vec3 material;

    Sphere(vec3 centro, double radio, Material m) {
      center = centro;
      radius = radio;
      material = m.diffuse;
    }

    Intersect rayIntersect(vec3 origin, vec3 direction) {
      vec3 L = center - origin;
      double tca = dotProd(L, direction);
      double l = vecLength(L);
      double d2 = pow(l, 2) - pow(tca, 2);
      if (d2 > pow(radius, 2)) {
        return Intersect(-10000, {0, 0, 0}, {0, 0, 0});
      }

      double thc = sqrt(pow(radius, 2) - d2);

      double t0 = tca - thc;

      double t1 = tca + thc;

      if (t0 > t1) {
        t0 = t1;
      }
      if (t0 < 0) {
        return Intersect(-10000, {0, 0, 0}, {0, 0, 0});
      }

      vec3 hit = origin + (direction * (float)t0);
      vec3 normal = norm(hit - center);

      return Intersect(t0, normal, hit);
    }
};
