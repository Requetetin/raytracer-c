#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "operations.hpp"
#include "sphere.hpp"
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
Material currentMaterial({0, 0, 0}, {0, 0, 0, 0}, 0);
Material backgroundMaterial({0, 0, 0}, {0, 0, 0, 0}, 0);
Intersect currentIntersect(0, {0, 0, 0}, {0, 0, 0});
vector<Sphere> scene;
vec3 reflectColor = {0, 0, 0};

Intersect shadowIntersect(0, {0, 0, 0}, {0, 0, 0});
Material shadowMaterial({0, 0, 0}, {0, 0, 0, 0}, 0);

double diffuseIntensity;
double specularIntensity;
double shadowIntensity;

int max_recursion_depth = 3;

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
    color[0] = r;
    color[1] = g;
    color[2] = b;
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

void sceneIntersect(vec3 origin, vec3 direction) {
  zbuffer = numeric_limits<double>::infinity();
  for (int i=0; i<scene.size(); i++) {
    currentIntersect = scene[i].rayIntersect(origin, direction);
    if (currentIntersect.distance != -10000) {
      if (currentIntersect.distance < zbuffer) {
        zbuffer = currentIntersect.distance;
        currentMaterial = scene[i].material;
        return;
      }
    }
   else {
      currentMaterial.diffuse = backgroundMaterial.diffuse;
    }
  }
}

void shadowSceneIntersect(vec3 origin, vec3 direction) {
  zbuffer = numeric_limits<double>::infinity();
  for (int i=0; i<scene.size(); i++) {
    shadowIntersect = scene[i].rayIntersect(origin, direction);
    if (shadowIntersect.distance != -10000) {
      if (shadowIntersect.distance < zbuffer) {
        zbuffer = shadowIntersect.distance;
        shadowMaterial = scene[i].material;
        return;
      }
    }
   else {
      shadowMaterial.diffuse = backgroundMaterial.diffuse;
    }
  }
}

void castRay(vec3 origin, vec3 direction, int recursion) {
  sceneIntersect(origin, direction);
  if (!vecLength(currentMaterial.diffuse) || recursion >= max_recursion_depth) {
    glColor(0, 0, 0);
    return;
  }

  vec3 light_dir = norm(light.position - currentIntersect.point);

  vec3 offset_normal = currentIntersect.normal * 1.1f;
  vec3 shadow_origin;
  if (dot(light_dir, currentIntersect.normal) > 0) {
    shadow_origin = currentIntersect.point + offset_normal;
  } else {
    shadow_origin = currentIntersect.point - offset_normal;
  }
  shadowSceneIntersect(shadow_origin, light_dir);

  if (!vecLength(shadowMaterial.diffuse)) {
    shadowIntensity = 0;
  } else {
    shadowIntensity = 0.9;
  }

  if (currentMaterial.albedo.z > 0) {
    vec3 reverseDirection = direction * -1.f;
    vec3 reflect_dir = reflect(reverseDirection, currentIntersect.normal);
    vec3 reflect_origin;
    if (dot(reverseDirection, currentIntersect.normal) >= 0) {
      reflect_origin = currentIntersect.point + offset_normal;
    } else {
      reflect_origin = currentIntersect.point - offset_normal;
    }
    castRay(reflect_origin, reflect_dir, recursion + 1);
  } else {
    reflectColor = {0, 0, 0};
  }

  diffuseIntensity = light.intensity * std::max(0.f, dot(light_dir, currentIntersect.normal)) * (1 - shadowIntensity);
  
  if (shadowIntensity > 0) {
    specularIntensity = 0;
  } else {
    vec3 R = reflect(light_dir, currentIntersect.normal);
    specularIntensity = light.intensity * pow(std::max(0.f, dot(R, direction)), currentMaterial.specular);
  }

  vec3 diffuse = currentMaterial.diffuse * (float)diffuseIntensity * currentMaterial.albedo.x;
  vec3 specular = light.color * (float)specularIntensity * currentMaterial.albedo.y;
  vec3 reflection = reflectColor * currentMaterial.albedo.z;

  vec3 finalColor = diffuse + specular + reflection;
  glColor(finalColor.x, finalColor.y, finalColor.z);
}

void glRender() {
  double fov = glm::half_pi<double>();
  double angle = tan(fov/2);
  double factor = aspectR * angle;
  vec3 direction;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double i = (2 * ((x + 0.5) / width) - 1) * factor;
      double j = (1 - 2 * ((y + 0.5) / height)) * angle;

      direction = norm({i, j, -1});
      castRay({0, 0, 0}, direction, 0);
      glVertex(x, y);
    }
  }
}

int main() {
  glInit(1000, 1000);

  light.position = {10, 10, 10};
  light.intensity = 1;
  light.color = {255, 255, 255};
  Material ivory({100, 100, 80}, {0.6, 0.3, 0.1, 0}, 50);
  Material rubber({80, 0, 0}, {0.9, 0.1, 0, 0}, 10);
  Material mirror({255, 255, 255}, {0, 10, 0.8, 0}, 1500);

  Sphere s1({0, -1.5, -10}, 1.5, ivory);
  Sphere s2({-2, 1, -12}, 2, mirror);
  Sphere s3({1, 1, -8}, 1.7, rubber);
  Sphere s4({0, 5, -20}, 5, mirror);

  scene.push_back(s1);
  scene.push_back(s2);
  scene.push_back(s3);
  scene.push_back(s4);
  
  
  glRender();
  glFinish();
  cout << "done" << endl;
}