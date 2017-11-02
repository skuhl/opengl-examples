// Author: Tyler Eichten

#include <stdio.h>
#include <stdlib.h>

//if standing in center of screen, x should be 0;
//if standing on the right, x should be 2.06;
//if standing on the left, x should be -2.06;

#define HORIZ_LEFT -3.09
#define HORIZ_RIGHT 3.09
#define HORIZ_COUNT 2
#define HORIZ_SIZE (HORIZ_RIGHT - HORIZ_LEFT)
#define HORIZ_SCREEN_SIZE (HORIZ_SIZE / HORIZ_COUNT)
#define VERT_BOT 0.28
#define VERT_TOP 2.6
#define VERT_COUNT 4
#define VERT_SIZE (VERT_TOP - VERT_BOT)
#define VERT_SCREEN_SIZE (VERT_SIZE / VERT_COUNT)
#define NEAR 3.5
#define FAR 100.0

typedef struct {
  float frustum_l;
  float frustum_r;
  float frustum_b;
  float frustum_t;
} Frustum;

int x = 0;
float y = 0;

void setFrustum(Frustum* input, int lMult, int rMult, int bMult, int tMult) {
  input->frustum_l = HORIZ_LEFT + HORIZ_SCREEN_SIZE * lMult - x;
  input->frustum_r = HORIZ_LEFT + HORIZ_SCREEN_SIZE * rMult - x;
  input->frustum_b = VERT_BOT + VERT_SCREEN_SIZE * bMult - y;
  input->frustum_t = VERT_BOT + VERT_SCREEN_SIZE * tMult - y;
  return;
}

char* getFrustum(int nodeNum, Frustum* input) {
  char* frustumInfo = malloc(100 * sizeof(char));
  switch (nodeNum) {
    case 1: setFrustum(input, 1, 2, 3, 4); break;
    case 2: setFrustum(input, 1, 2, 2, 3); break;
    case 3: setFrustum(input, 1, 2, 1, 2); break;
    case 4: setFrustum(input, 1, 2, 0, 1); break;
    case 5: setFrustum(input, 0, 1, 3, 4); break;
    case 6: setFrustum(input, 0, 1, 2, 3); break;
    case 7: setFrustum(input, 0, 1, 1, 2); break;
    case 8: setFrustum(input, 0, 1, 0, 1); break;
  }
  sprintf(frustumInfo, "frustum = %f %f %f %f %f %f\n", input->frustum_l, input->frustum_r, input->frustum_b, input->frustum_t, NEAR, FAR);
  return frustumInfo;
}

int main(int argc, char** argv) {

  if (argc == 3) {
    x = atoi(argv[1]) * 2.06;
    y = atof(argv[2]);
  }
  else {
    if (argc != 1) {
      printf("Invalid format!\n");
    }
    printf("Using default X and Y values of (0,0).\nCustom format: ./frustum-generate [x: -1, 0, or 1] [y: subject's height (meters)]\n");
  }

  for (int i = 1; i <= 8; i++) {
    char filename[100];
    sprintf(filename, "node%d.ini", i);
    printf("Writing: %s\n", filename);
    Frustum frustum;

    //open ini
    FILE* ini = fopen(filename, "w");

    if (!ini) {
      printf("Error opening file/n");
      return -1;
    }

    //include common in ini
    fputs("include = config/ivs/common.ini\n", ini);

    //place log file into ini
    char logFile[100] = "";
    sprintf(logFile, "log.filename = log-node%d.txt\n", i);
    fputs(logFile, ini);

    //put frustum info into ini
    char* frustumInfo = getFrustum(i, &frustum);
    fputs(frustumInfo, ini);
    free(frustumInfo);
    //close ini
    fclose(ini);
  }
  return 1;
}
