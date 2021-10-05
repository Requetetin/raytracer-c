#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "operations.hpp"
#define byte uint8_t
using namespace std;
using namespace glm;

byte bmpfileheader[14] = {'B', 'M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
byte bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
byte framebuffer[2160][4096][3];

byte BLACK[3] = {0, 0, 0};
byte WHITE[3] = {255, 255, 255};
byte RED[3] = {255, 0, 0};
byte GREEN[3] = {0, 255, 0};
byte BLUE[3] = {0, 0, 255};
byte clearColor[3] = {0, 0, 0};
byte color[3] = {255, 255, 255};

int width, height;
float aspectR;

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

void castRay(vec3 origin, vec3 direction) {
  glColor(0, 0, 255);
}

void glRender() {
  float fov = glm::half_pi<float>();
  float angle = tan(fov/2);
  float factor = aspectR * angle;
  vec3 direction;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (rand() % 100 > 50) {
        float i = (2 * ((x + 0.5) / width) - 1) * factor;
        float j = 1 - 2 * ((y + 0.5) / height) * angle;

        direction = norm({i, j, -1});
        castRay({0, 0, 0}, direction);
        glVertex(x, y);
      }
    }
  }
}

int main() {
  glInit(4096, 2160);
  glRender();
  glFinish();
  cout << "done" << endl;
}