#include <stdbool.h>
#include <stdio.h>
// for qsort
#include <stdlib.h>
#include <search.h>
//
#include <math.h>
#include <stddef.h>
#include <string.h>

#define WIDTH 100
#define HEIGHT 100

typedef struct {
  float x;
  float  y;
} Point;

int draw_triangle(size_t * pixels, int w, int h,Point *p1, Point *p2, Point *p3, int color, char * output_filename);

int compare_f(const void *p1, const void *p2) {
  return (**(Point**)p1).y - (**(Point**)p2).y;
}

int draw_rectangle(int width, int height, int color, char* output_filename)
{

  unsigned int pixels[width * height];

  for(int i =0; i< width*height; i++)
    {
      
      pixels[i] = color;
    }

  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", width, height, 255);

  for(int i=0; i<width*height; ++i) {
    fwrite(&pixels[i], 3, 1, file);
  }
  
  fclose(file);

  return 0;
  
}

int draw_circle(int centerx, int centery, int r, int color, char* output_filename) {

  int window_width = 100;
  int window_height = 100;

  size_t pixels[window_height*window_width];
  memset(pixels, 0, window_height*window_width*sizeof(size_t));
  
  for(int i=0; i< window_height*window_width; ++i) {
    int x = i % window_width;
    int y = i/window_height;

    int distance_sq = (y - centery)*(y-centery) + (x-centerx) * (x - centerx);

    if (distance_sq <= r*r) {
      pixels[i] = color;
    } 
  }

  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", window_width, window_height, 255);

  for(int i=0; i<window_height*window_width; ++i) {
    fwrite(&pixels[i], 3, 1, file);
  }

  fclose(file);

  return 0;
  
}

int is_between(int num1, int num2, int candidate) {

  if (num1 <= num2) {
    return ((num1 <= candidate) && (candidate <= num2));
  }

  else {
    return ((num2 <= candidate) && (candidate <= num1));
  }
  
}

int draw_line_in_a_better_way(size_t *pixels, int window_width, int window_height, Point *p1, Point *p2, int color, char* output_filename, bool create_file) {

  int ystart = p1->y;
  int yend = p2->y;

  int xstart = p1->x;
  int xend = p2->x;
  
  if (ystart == yend) {
    int bigx = xend > xstart ? xend : xstart;
    int smallx = xend == bigx ? xstart : xend ;

    for(int i = bigx; i>=smallx; i-- ) {
      pixels[i+ystart*window_width] = color;
    }

  }
  else if (xstart == xend ) {
    int bigy = yend > ystart ? yend : ystart;
    int smally = yend == bigy ? ystart : yend;

    for(int i = bigy; i >= smally; i--) {
      pixels[xstart + i * window_height] = color;

    }

  }
  else {
    float m = ((float)yend - (float)ystart) / ((float)xend - (float)xstart);
    int op = yend >= ystart ? -1 : 1;
    int i = yend;
    while (i != ystart) {

      int  xcandidate = xend - (yend - i) / (m);
      pixels[xcandidate+i*window_height] = color;
      i += op;
    }
  }
  if (create_file) {
    FILE* file = fopen(output_filename, "wb");
    if (file == NULL){
      printf("error opening file\n");
      return -1;
    }
  
    fprintf(file, "P6\n%d %d\n%d\n", window_width, window_height, 255);

    for(int i=0; i<window_height*window_width; ++i) {
      fwrite(&pixels[i], 3, 1, file);
    }

    fclose(file);
  }
  return 0;
}

int draw_triangle_in_a_better_way(size_t* pixels, int w, int h, Point *p1, Point *p2, Point *p3, int color, bool fill, char * output_filename) {

  if (!fill) {
    return draw_triangle(pixels, w,h, p1, p2, p3, color, output_filename);
  }
  
  Point*  points[] = {p1, p2, p3};
  qsort(points, 3, sizeof(Point), compare_f);


  for (int i=0; i<3; i++) {
    printf("Point: %f, %f\n", points[i]->x, points[i]->y);
  }

  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", w, h, 255);

  float m1 = (points[1]->y - points[0]->y)/(points[1]->x - points[0]->x);
  float m2 = (points[2]->y - points[0]->y)/(points[2]->x - points[0]->x);

  for (int i = points[0]->y; i<=points[1]->y; i++) {
    
    int x1 = (i - points[0]->y) / m1 + points[0]->x;
    int x2 = (i - points[2]->y) / m2 + points[2]->x;
    
    int op = x1>=x2 ? -1 : 1 ;
    if (x1>=x2) {
      while (x1>=x2) {
	pixels[i*w + x1] = color;
	x1 += op;
      }
    }

    else {
      while (x1<=x2) {
	pixels[i*w + x1] = color;
	x1 += op;
      }
    }
  }

  float m3 = (points[0]->y - points[2]->y)/(points[0]->x - points[2]->x);
  float m4 = (points[1]->y - points[2]->y)/(points[1]->x - points[2]->x);

  if (points[2]->y != points[1]->y) {
    for (int i=points[2]->y; i>=points[1]->y; i--) {

      int x1 = (i - points[0]->y) / m3 + points[0]->x;
      int x2 = (i - points[1]->y) / m4 + points[1]->x;

      int op = x1>=x2 ? -1 : 1 ;
      if (x1>=x2) {
	while (x1>=x2) {
	  pixels[i*w + x1] = color;
	  x1 += op;
	}
      }

      else {
	while (x1<=x2) {
	  pixels[i*w + x1] = color;
	  x1 += op;
	}
      }
    }
  }

  for(int i=0; i<h*w; ++i) {
    fwrite(&pixels[i], 3, 1, file);
  }

  fclose(file);
  
  return 0;
}

int draw_triangle(size_t * pixels, int w, int h,Point *p1, Point *p2, Point *p3, int color, char * output_filename) {
  
  memset(pixels, 0, h*w*sizeof(size_t));

  draw_line_in_a_better_way(pixels, w, h, p1, p2, color, output_filename, false);
  draw_line_in_a_better_way(pixels, w, h, p2, p3, color, output_filename, false);
  draw_line_in_a_better_way(pixels, w, h, p1, p3, color, output_filename,false);
 
  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", w, h, 255);

  for(int i=0; i<h*w; ++i) {
    fwrite(&pixels[i], 3, 1, file);
  }

  fclose(file);
  
  return 0;

}

int draw_dots(size_t * pixels, int w, int h, int x1, int y1, int x2, int y2, int x3, int y3, int color, char * output_filename) {
  
  memset(pixels, 0, h*w*sizeof(size_t));

  pixels[y1*w + x1] = color;
  pixels[y2*w + x2] = color;
  pixels[y3*w + x3] = color;

  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", w, h, 255);

  for(int i=0; i<h*w; ++i) {
    fwrite(&pixels[i], 3, 1, file);
  }

  fclose(file);
  return 0;
}

int main() {
  // Note that os x is  little indian so 0x000000ff is kept in memory as 0xff000000
  // which I would like to interpret as RGB (ff, 00, 00) for ppm format
  draw_rectangle(10, 10, 0x000000ff, "rect.ppm");

  draw_circle(0,0,50, 0x000000ff, "circle.ppm");

  int w = 100;
  int h = 100;

  size_t pixels[w*h];

  //@Clean: Have memset in the functions

  memset(pixels, 0, h*w*sizeof(size_t));
  Point p1 = {10, 50};
  Point p2 = {90, 50};
  draw_line_in_a_better_way(pixels, w, h, &p1,&p2, 0x000000ff, "line_horizontal.ppm", true);

  memset(pixels, 0, h*w*sizeof(size_t));
  Point p3 = {50, 10};
  Point p4  = {50, 90};
  draw_line_in_a_better_way(pixels, w, h, &p3, &p4, 0x000000ff, "line_vertical.ppm", true);
 
  memset(pixels, 0, h*w*sizeof(size_t));
  Point p5 = {30, 10};
  Point p6  = {70, 90};

  draw_line_in_a_better_way(pixels, w, h, &p5, &p6, 0x000000ff, "line_slope_in_a_better_way.ppm", true);
  
  memset(pixels, 0, h*w*sizeof(size_t));

  Point p7 = {0, 70};
  Point p8  = {70, 10};

  draw_line_in_a_better_way(pixels, w, h, &p7,&p8, 0x000000ff, "line_slope_in_a_better_way_2.ppm", true);

  Point p9 = {50, 0};
  Point p10 = {0, 99};
  Point p11 = {99, 99};
  
  memset(pixels, 0, h*w*sizeof(size_t));
  draw_triangle_in_a_better_way(pixels,w, h,  &p9, &p10, &p11, 0x000000ff, true,  "triangle_test.ppm");

  Point p12 = {0, 30};
  Point p13= {0, 60};
  Point p14 = {99, 99};
  memset(pixels, 0, h*w*sizeof(size_t));

  draw_triangle_in_a_better_way(pixels,w, h,  &p12, &p13, &p14, 0x000000ff, true,  "triangle_irregular_test.ppm");

  memset(pixels, 0, h*w*sizeof(size_t));
  draw_triangle_in_a_better_way(pixels,w, h, &p12, &p13, &p14, 0x000000ff, false,  "triangle_irregular_no_fill_test.ppm");
  return 0;
}




