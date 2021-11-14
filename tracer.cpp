#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "operations.hpp"
#include "object.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "cube.hpp"
#define byte uint8_t
using namespace std;
using namespace glm;

byte bmpfileheader[14] = {'B', 'M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
byte bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
byte framebuffer[4056][4056][3];

byte BLACK[3] = {0, 0, 0};
byte WHITE[3] = {255, 255, 255};
byte RED[3] = {255, 0, 0};
byte GREEN[3] = {0, 255, 0};
byte BLUE[3] = {0, 0, 255};
byte clearColor[3] = {0, 0, 0};
byte color[3] = {255, 255, 255};

int width, height;
double aspectR;
double zbuffer = numeric_limits<double>::infinity();
Light light({0, 0, 0}, 0, {0, 0, 0});
Material* backgroundMaterial = new Material({0, 0, 255}, {0, 0, 0, 0}, 0, 0);
Intersect defaultIntersect(-10000, {0, 0, 0}, {0, 0, 0});
vector<Figura*> scene;

int MAX_RECURSION_DEPTH = 3;

void glVertex(int x, int y){
    framebuffer[y][x][0] = color[2];
    framebuffer[y][x][1] = color[1];
    framebuffer[y][x][2] = color[0];
}

void glClear()
{
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            framebuffer[i][j][0] = clearColor[2];
            framebuffer[i][j][1] = clearColor[1];
            framebuffer[i][j][2] = clearColor[0];
        }
    }
}

void glFinish()
{
    FILE* file;
    file = fopen("b.bmp", "wb");
    fwrite(bmpfileheader, 1, 14, file);
    fwrite(bmpinfoheader, 1, 40, file);
    for(int i = 0; i < height; i++){ 
        for(int j = 0; j < width; j++){
            fwrite(framebuffer[i][j], 1, 3, file);
        }
    }
    fclose(file);
}

void glColor(int r, int g, int b)
{
    color[0] = std::max(0.f, std::min((float)r, 255.f));
    color[1] = std::max(0.f, std::min((float)g, 255.f));
    color[2] = std::max(0.f, std::min((float)b, 255.f));
}

void fillHeader()
{
    int filesize = 54 + 3 * width * height;
    bmpfileheader[ 2] = (byte)(filesize    );
    bmpfileheader[ 3] = (byte)(filesize>> 8);
    bmpfileheader[ 4] = (byte)(filesize>>16);
    bmpfileheader[ 5] = (byte)(filesize>>24);

    bmpinfoheader[ 4] = (byte)(       width    );
    bmpinfoheader[ 5] = (byte)(       width>> 8);
    bmpinfoheader[ 6] = (byte)(       width>>16);
    bmpinfoheader[ 7] = (byte)(       width>>24);
    bmpinfoheader[ 8] = (byte)(       height    );
    bmpinfoheader[ 9] = (byte)(       height>> 8);
    bmpinfoheader[10] = (byte)(       height>>16);
    bmpinfoheader[11] = (byte)(       height>>24);
    bmpinfoheader[20] = (byte)(        3 * width * height    );
    bmpinfoheader[21] = (byte)(        3 * width * height>> 8    );
    bmpinfoheader[22] = (byte)(        3 * width * height>>16    );
    bmpinfoheader[23] = (byte)(        3 * width * height>>24    );
}

void glInit(int w, int h){
  width = w;
  height = h;
  aspectR = width/height;
  fillHeader();
  glClear();
}

Clash* sceneIntersect(vec3 origin, vec3 direction) {
  zbuffer = numeric_limits<double>::infinity();
  Clash* clash = new Clash(*backgroundMaterial, defaultIntersect);
  Intersect hit;
  for (int i=0; i<scene.size(); i++) {
    hit = scene[i]->rayIntersect(origin, direction);
    if (hit.distance > -10000) {
      if (hit.distance < zbuffer) {
        zbuffer = hit.distance;
        Material* temp = scene[i]->material;
        clash = new Clash(*temp, hit); 
      }
    }
  }
  return clash;
}

vec3 castRay(vec3 origin, vec3 direction, int recursion) {
  Clash* hit = sceneIntersect(origin, direction);
  double diffuseIntensity;
  double specularIntensity;
  double shadowIntensity;
  
  if (hit->intersect.distance <= -10000 || recursion > MAX_RECURSION_DEPTH) {
    return backgroundMaterial->diffuse;
  }
  //cout << to_string(hit->material.diffuse) << endl;
  vec3 light_dir = norm(light.position - hit->intersect.point);

  vec3 offset_normal = hit->intersect.normal * 0.1f;
  vec3 shadow_origin;
  if (dot(light_dir, hit->intersect.normal) >= 0) {
    shadow_origin = hit->intersect.point + offset_normal;
  } else {
    shadow_origin = hit->intersect.point - offset_normal;
  }
  Clash* shadowhit = sceneIntersect(shadow_origin, light_dir);

  if (shadowhit->intersect.distance <= -10000) {
    shadowIntensity = 0;
  } else {
    shadowIntensity = 0.99;
  }

  vec3 reflect_color;
  if (hit->material.albedo.z > 0) {
    vec3 reverse_direction = direction * -1.f;
    vec3 reflect_direction = reflect(reverse_direction, hit->intersect.normal);
    vec3 reflect_origin;
    if (dot(reflect_direction, hit->intersect.normal) >= 0) {
      reflect_origin = hit->intersect.point + offset_normal;
    } else {
      reflect_origin = hit->intersect.point - offset_normal;
    }
    reflect_color = castRay(reflect_origin, reflect_direction, recursion + 1);
  } else {
    reflect_color = {0, 0, 0};
  }

  vec3 refract_color;
  if (hit->material.albedo[3] > 0) {
    vec3 refract_direction = refract(direction, hit->intersect.normal, hit->material.refraction);
    if (vecLength(refract_direction) == 0) {
      refract_color = {0, 0, 0};
    } else {
      vec3 refract_origin;
      if (dot(refract_direction, hit->intersect.normal) >= 0) {
        refract_origin = hit->intersect.point + offset_normal;
      } else {
        refract_origin = hit->intersect.point - offset_normal;
      }
      refract_color = castRay(refract_origin, refract_direction, recursion + 1);
    }
  } else {
    refract_color = {0, 0, 0};
  }

  diffuseIntensity = light.intensity * std::max(0.f, dot(light_dir, hit->intersect.normal)) * (1 - shadowIntensity);
  
  if (shadowIntensity > 0) {
    specularIntensity = 0;
  } else {
    vec3 R = reflect(light_dir, hit->intersect.normal);
    specularIntensity = light.intensity * pow(std::max(0.f, -dot(R, direction)), hit->material.specular);
  }

  vec3 diffuse = hit->material.diffuse * (float)diffuseIntensity * hit->material.albedo.x;
  vec3 specular = light.color * (float)specularIntensity * hit->material.albedo.y;
  vec3 reflection = reflect_color * hit->material.albedo.z;
  vec3 refraction = refract_color * hit->material.albedo[3];

  return diffuse + specular + reflection + refraction;
}

void glRender() {
  double fov = glm::half_pi<double>();
  double angle = tan(fov/2);
  double factor = aspectR * angle;
  vec3 direction;
  vec3 returnColor;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double i = (2 * ((x + 0.5) / width) - 1) * factor;
      double j = (1 - 2 * ((y + 0.5) / height)) * angle;

      direction = norm({i, j, -1});
      returnColor = castRay({0, 0, 0}, direction, 0);
      glColor(returnColor.x, returnColor.y, returnColor.z);
      glVertex(x, y);
    }
  }
}

int main() {
  glInit(1000, 1000);

  light.position = {10, 10, 20};
  light.intensity = 1;
  light.color = {255, 255, 255};
  Material* ivory = new Material({100, 100, 80}, {0.9, 0.3, 0.1, 0}, 50, 0);
  Material* rubber = new Material({80, 0, 0}, {0.9, 0.1, 0, 0}, 10, 0);
  Material* mirror = new Material({255, 255, 255}, {0, 10, 0.8, 0}, 1500, 0);
  Material* glass = new Material({255, 255, 255}, {0, 0.5, 0.1, 0.8}, 150, 1.5);

  Figura* s1 = new Sphere({0, -1.5, -10}, 1.5, ivory);
  Figura* s2 = new Sphere({-2, 1, -12}, 2, glass);
  Figura* s3 = new Sphere({1, 1, -7}, 1.7, rubber);
  Figura* s4 = new Sphere({0, 5, -20}, 5, mirror);

  /*scene.push_back(s1);
  scene.push_back(s2);
  scene.push_back(s3);
  scene.push_back(s4);*/

  Plane* p1 = new Plane({0, 0, -10}, {1, 1, -1}, ivory);

  //scene.push_back(p1);

  //R
  /*Figura* c1 = new Cube({6, 6, 7}, {5, 1, 8}, ivory);
  Figura* c2 = new Cube({3, 6, 7}, {5, 5, 8}, ivory);
  Figura* c3 = new Cube({3, 4, 7}, {5, 3, 8}, ivory);
  Figura* c4 = new Cube({3, 4, 7}, {4, 5, 8}, ivory);
  Figura* c5 = new Cube({5, 3, 7}, {4, 2, 8}, ivory);
  Figura* c6 = new Cube({3, 2, 7}, {4, 1, 8}, ivory);
  scene.push_back(c1);
  scene.push_back(c2);
  scene.push_back(c3);
  scene.push_back(c4);
  scene.push_back(c5);
  scene.push_back(c6);*/

  //A
  /*Figura* c7 = new Cube({2, 1, 7}, {1, 6, 8}, ivory);
  Figura* c8 = new Cube({0, 1, 7}, {-1, 6, 8}, ivory);
  Figura* c9 = new Cube({0, 5, 7}, {1, 6, 8}, ivory);
  Figura* c10 = new Cube({0, 4, 7}, {1, 3, 8}, ivory);
  scene.push_back(c7);
  scene.push_back(c8);
  scene.push_back(c9);
  scene.push_back(c10);*/

  //Y
  /*Figura* c11 = new Cube({-2, 5, 7}, {-3, 6, 8}, ivory);
  Figura* c12 = new Cube({-6, 5, 7}, {-7, 6, 8}, ivory);
  Figura* c13 = new Cube({-3, 5, 7}, {-4, 4, 8}, ivory);
  Figura* c14 = new Cube({-6, 5, 7}, {-5, 4, 8}, ivory);
  Figura* c15 = new Cube({-4, 4, 7}, {-5, 1, 8}, ivory);
  scene.push_back(c11);
  scene.push_back(c12);
  scene.push_back(c13);
  scene.push_back(c14);
  scene.push_back(c15);*/

  //C
  /*Figura* c16 = new Cube({6, -6, 7}, {5, -1, 8}, ivory);
  Figura* c17 = new Cube({5, -6, 7}, {3, -5, 8}, ivory);
  Figura* c18 = new Cube({3, -2, 7}, {5, -1, 8}, ivory);
  scene.push_back(c16);
  scene.push_back(c17);
  scene.push_back(c18);*/

  //+
  Figura* c19 = new Cube({1, -5, 7}, {0, -2, 8}, ivory);
  Figura* c20 = new Cube({2, -4, 7}, {1, -3, 8}, ivory);
  Figura* c21 = new Cube({0, -4, 7}, {-1, -3, 8}, ivory);
  scene.push_back(c19);
  scene.push_back(c20);
  scene.push_back(c21);
  Figura* c22 = new Cube({-3, -5, 7}, {-4, -2, 8}, ivory);
  Figura* c23 = new Cube({-2, -4, 7}, {-3, -3, 8}, ivory);
  Figura* c24 = new Cube({-4, -4, 7}, {-5, -3, 8}, ivory);
  scene.push_back(c22);
  scene.push_back(c23);
  scene.push_back(c24);
  
  /*
    Trate de escribir RAY C++
    Pero solo renderiza una sola letra
    O los dos signos + 
  */
  glRender();
  glFinish();
  cout << "done" << endl;
}