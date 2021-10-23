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
Material backgroundMaterial({0, 0, 0}, {0, 0, 0, 0}, 0);
Intersect defaultIntersect(-10000, {0, 0, 0}, {0, 0, 0});
vector<Sphere> scene;

double diffuseIntensity;
double specularIntensity;
double shadowIntensity;

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

Clash* sceneIntersect(vec3 origin, vec3 direction) {
  zbuffer = numeric_limits<double>::infinity();
  Clash* clash = new Clash(backgroundMaterial, defaultIntersect);
  Intersect hit;
  for (int i=0; i<scene.size(); i++) {
    hit = scene[i].rayIntersect(origin, direction);
    if (hit.distance > -10000) {
      if (hit.distance < zbuffer) {
        zbuffer = hit.distance;
        clash = new Clash(scene[i].material, hit); 
        //clash.material = scene[i].material;
        //clash.intersect = hit;
        //cout << hit.distance << endl;
        //cout << "1 " << to_string(scene[i].material.diffuse) << endl;
        //cout << to_string(clash->material.diffuse) << endl;
      }
    }
  }
  //cout << clash->intersect.distance << endl << endl;
  return clash;
}

void castRay(vec3 origin, vec3 direction) {
  Clash* hit = sceneIntersect(origin, direction);
  
  if (hit->intersect.distance <= -10000) { //Revisar para fondo!!
    glColor(backgroundMaterial.diffuse.x, backgroundMaterial.diffuse.y, backgroundMaterial.diffuse.z);
    return;
  }
  //cout << hit->intersect.distance << endl;
  vec3 light_dir = norm(light.position - hit->intersect.point);

  vec3 offset_normal = hit->intersect.normal * 0.1f;
  vec3 shadow_origin;
  if (dot(light_dir, hit->intersect.normal) > 0) {
    shadow_origin = hit->intersect.point + offset_normal;
  } else {
    shadow_origin = hit->intersect.point - offset_normal;
  }
  Clash* shadowhit = sceneIntersect(shadow_origin, light_dir);

  if (shadowhit->intersect.distance <= -10000) {
    shadowIntensity = 0;
  } else {
    shadowIntensity = 0.9;
  }

  diffuseIntensity = light.intensity * std::max(0.f, dot(light_dir, hit->intersect.normal)) * (1 - shadowIntensity);
  
  if (shadowIntensity > 0) {
    specularIntensity = 0;
  } else {
    vec3 R = reflect(light_dir, hit->intersect.normal);
    specularIntensity = light.intensity * pow(std::max(0.f, dot(R, direction)), hit->material.specular);
  }

  vec3 diffuse = hit->material.diffuse * (float)diffuseIntensity * hit->material.albedo.x;
  vec3 specular = light.color * (float)specularIntensity * hit->material.albedo.y;

  vec3 finalColor = diffuse + specular;
  //cout << to_string(hit->material.diffuse) << endl;
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
      castRay({0, 0, 0}, direction);
      glVertex(x, y);
    }
  }
}

int main() {
  glInit(1000, 1000);

  light.position = {10, 10, 10};
  light.intensity = 1;
  light.color = {255, 255, 255};
  Material ivory({100, 100, 80}, {0.6, 0.3, 0, 0}, 50);
  Material rubber({80, 0, 0}, {0.9, 0.1, 0, 0}, 10);
  Sphere s1({0, -1.5, -10}, 2, ivory);
  Sphere s2({-2, 1, -12}, 2, ivory);
  Sphere s3({1, 1, -7}, 2, rubber);
  Sphere s4({0, 5, -20}, 2, ivory);

  scene.push_back(s1);
  scene.push_back(s2);
  scene.push_back(s3);
  scene.push_back(s4);
  
  
  glRender();
  glFinish();
  cout << "done" << endl;
}