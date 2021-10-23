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

vec3 reflect(vec3 I, vec3 N) {
    return norm(I - N * 2.f * dot(N, I));
}

vec3 refract(vec3 I, vec3 N, double refractIndex) {
    double cosi = -std::max(-1.f, std::min(1.f, dot(I, N)));
    double etai = 1;
    double etat = refractIndex;

    if (cosi < 0) {
        cosi *= -1;
        etai = etat;
        etat = 1;
        N = N * -1.f;
    }

    double eta = etai / etat;
    double k = 1 - pow(eta, 2) * (1 - pow(cosi, 2));

    if (k < 0) {
        return {0, 0, 0};
    }

    double cost = sqrt(k);
    double factor = (eta * cosi) + cost;

    return norm((I * (float)eta) + (N * (float)factor));
}

class Material {
    public:
        vec3 diffuse;
        vec4 albedo;
        double specular;
        double refractI;

        Material(vec3 difuso, vec4 albedoo, double specularr, double refract_index) {
            diffuse = difuso;
            albedo = albedoo;
            specular = specularr;
            refractI = refract_index;
        }

        Material() {

        }

};

class Intersect {
    public:
        double distance;
        vec3 point;
        vec3 normal;

        Intersect(double distancia, vec3 normall, vec3 punto) {
            distance = distancia;
            normal = normall;
            point = punto;
        }

        Intersect() {

        }
};

class Light {
    public:
        vec3 position;
        double intensity;
        vec3 color;

        Light(vec3 posicion, double intensidad, vec3 colorr) {
            position = posicion;
            intensity = intensidad;
            color = colorr;
        }
};

class Clash {
    public:
        Material material;
        Intersect intersect;

        Clash(Material m, Intersect i) {
            material.albedo = m.albedo;
            material.diffuse = m.diffuse;
            material.specular = m.specular;
            intersect.distance = i.distance;
            intersect.normal = i.normal;
            intersect.point = i.point;
        }

        Clash() {

        }
};