#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace glm;
using namespace std;

class Cube : public Figura {
  public:
    Material* material;
    vec3 vmin;
    vec3 vmax;

    Cube(vec3 min, vec3 max, Material* m) {
      vmin = min;
      vmax = max;
      material = m;
    }

    Intersect rayIntersect(vec3 origin, vec3 direction) {
      double tmin, tmax, tymin, tymax, tzmin, tzmax;
      
      tmin = (vmin.x - origin.x) / direction.x;
      tmax = (vmax.x - origin.x) / direction.x;

      if (tmin > tmax) swap(tmin, tmax);

      tymin = (vmin.y - origin.y) / direction.y;
      tymax = (vmax.y - origin.y) / direction.y;

      if (tymin > tymax) swap(tymin, tymax);

      if ((tmin > tymax) || (tymin > tmax)) {
        return Intersect(-10000, {0, 0, 0}, {0, 0, 0});
      }

      if (tymin > tmin) {
        tmin = tymin;
      }
      if (tymax < tmax) {
        tmax = tymax;
      }

      tzmin = (vmin.z - origin.z) / direction.z;
      tzmax = (vmax.z - origin.z) / direction.z;

      if (tzmin > tzmax) swap(tzmin, tzmax);

      if ((tmin > tzmax) || (tzmin > tmax)) {
        return Intersect(-10000, {0, 0, 0}, {0, 0, 0});
      }

      if (tzmin > tmin) {
        tmin = tzmin;
      }
      if (tzmax < tmax) {
        tmax = tzmax;
      }

      vec3 hit = origin + (direction * (float)tmin);
      vec3 normal;
      double EPSI = 0.01;
      if (abs(hit.x - vmin.x) < EPSI) normal = {-1, 0, 0};
      else if (abs(hit.x - vmax.x) < EPSI) normal = {1, 0, 0};
      else if (abs(hit.y - vmin.y) < EPSI) normal = {0, -1, 0};
      else if (abs(hit.y - vmax.y) < EPSI) normal = {0, 1, 0};
      else if (abs(hit.z - vmin.z) < EPSI) normal = {0, 0, -1};
      else if (abs(hit.z - vmax.z) < EPSI) normal = {0, 0, 1};
      return Intersect(tmin, normal, hit);
    }
};
