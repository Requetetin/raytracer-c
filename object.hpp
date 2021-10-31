#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace glm;
using namespace std;

class Figura {
  public:
    Material material;
    
    virtual Intersect rayIntersect(vec3 origin, vec3 direction) {return Intersect(-10000, {0, 0, 0}, {0, 0, 0});};
    virtual ~Figura() {}
    Figura() {}
};