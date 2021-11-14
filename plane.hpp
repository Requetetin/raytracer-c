#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace glm;
using namespace std;

class Plane : public Figura {
  public:
    vec3 p0;
    vec3 normal;
    Material* material;

    Plane(vec3 point, vec3 n, Material* m) {
      p0 = point;
      normal = norm(n);
      material = m;
    }

    Intersect rayIntersect(vec3 origin, vec3 direction) {
      double denom = dotProd(normal, direction);
      if (denom > 1e-6) {
        vec3 diff = p0 - origin;
        double t = dotProd(diff, normal) / denom;
        if (t > 0) {
          cout << "hi";
          return Intersect(t, normal, origin + direction * (float)t);
        }
      }

      return Intersect(-10000, {0, 0, 0}, {0, 0, 0});
    }
};
