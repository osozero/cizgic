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

typedef struct {
  int width;
  int height;
  int num_of_bits_per_pixel;

  // format of pixels is bottom to up while ppm file format is top to bottom.
  char * pixels;
} Texture;

int draw_triangle(size_t * pixels, int w, int h,Point *p1, Point *p2, Point *p3, int color, char * output_filename);

int compare_f(const void *p1, const void *p2) {
  return (**(Point**)p1).y - (**(Point**)p2).y;
}

Texture* load_texture(char* image_path) {
  FILE *file = fopen(image_path, "rb");

  if (file == NULL) {
    printf("file could not read: %s\n", image_path);
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  size_t file_len = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char*) malloc(file_len * sizeof(char));

  if(!buffer) {
    printf("malloc error\n");
    fclose(file);
    exit(1);
  }

  size_t count = fread(buffer, sizeof(char), file_len, file);

  while(count < file_len) {
    if(count == 0) {
      //read error or EOF
      if (feof(file)) {
	break;
      } else {
	printf("read file error\n");
	fclose(file);
	exit(1);
      }
    } else {
      file_len -= count;
      count = fread(buffer+count, file_len, sizeof(char), file);
    }
  }

  if(buffer[0] != 'B' && buffer[1] != 'M') {
    printf("File must be bmp: %s\n", image_path);
    fclose(file);
    exit(1);
  }

  
  int file_size = *(int*)(buffer+2);
  int width_of_pixels = *(int*)(buffer + 0x12);
  int height_of_pixels = *(int*)(buffer + 0x16);

  short  num_of_bits_per_pixel = *(short*)(buffer + 0x1C);

  int image_pixel_offset = *(int*)(buffer+0xA);
  int total_size_of_pixels_data = file_size - image_pixel_offset;
  
  char *pixels = (char*)malloc(total_size_of_pixels_data * sizeof(char));
  if (!pixels) {
    printf("malloc error\n");
    fclose(file);
    exit(1);
  }

  size_t index = 0;
  int curr = image_pixel_offset;
  
  while(curr-image_pixel_offset <= total_size_of_pixels_data) {
    int padding = 0;
    for (int i = 0; i < width_of_pixels * (num_of_bits_per_pixel/8); i+=3 ) {
      // Pixel format in BMP is little endian so red is kept as 00 00 FF
      pixels[index+2] = *(buffer+curr);
      pixels[index+1] = *(buffer+curr+1);
      pixels[index] = *(buffer+curr+2);
      curr+=3;
      index += 3;
      padding+=3; // assumig number of bits per pixel is 24.
    }
    padding = padding % 4;
    while(padding > 0) {
      curr ++;
      padding--;
    }
  }


  Texture *t = (Texture*)malloc(sizeof(Texture));

  if (!t) {
    printf("malloc error for texture\n");
    fclose(file);
    exit(1);
  }

  t->pixels = pixels;
  t->height = height_of_pixels;
  t->width = width_of_pixels;
  t->num_of_bits_per_pixel = num_of_bits_per_pixel;
  
  fclose(file);
  free(buffer);

  return t;
  
}


int save_texture_as_ppm(Texture *t, char* output_filename) {
  
  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", t->width, t->height, 255);


  int len = t->width * t->height*3;
  char buf[len];

  /* Format of t->pixels is slightly different than PPM format.
     While pixels are processed top to bottom in PPM format, it is bottom to up in t->pixels.
     That's the reason we need to find correspoinding pixel in t-pixels by doing simple math
  */
  for (int i  = 0; i < len; i+=3) {
    int y_offset = t->height -  (i/3)/(t->height) - 1;
    
    int x_offset = (i/3) % t->width;

    int offset = x_offset + y_offset*t->width;
    offset*=3;
   
    buf[i] = t->pixels[offset];
    buf[i+1] = t->pixels[offset +1];
    buf[i+2] = t->pixels[offset +2];
  }
  
  for(int i=0; i<t->width * t->height*3; i+=3) {
    fwrite(buf + i, 3, 1, file);
  }

  fclose(file);

  return 0;  

}

int draw_rectangle_with_texture(Texture *t, int w, int h, char *output_filename) {
  FILE* file = fopen(output_filename, "wb");
  if (file == NULL){
    printf("error opening file\n");
    return -1;
  }
  
  fprintf(file, "P6\n%d %d\n%d\n", w, h, 255);


  int byte_per_pixel = t->num_of_bits_per_pixel/8;

  int len = t->width * t->height*3;
  char buf[len];
  for (int i  = 0; i < len; i+=3) {
    int y_offset = t->height -  (i/3)/(t->height) - 1;
    
    int x_offset = (i/3) % t->width;

    int offset = x_offset + y_offset*t->width;
    offset*=3;
   
    buf[i] = t->pixels[offset];
    buf[i+1] = t->pixels[offset +1];
    buf[i+2] = t->pixels[offset +2];
  }

  for(int i=0; i<t->width * t->height*3; i+=3) {
    fwrite(buf + i, 3, 1, file);
  }
  
  fclose(file);


  return 0;
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
  Texture * texture  = load_texture("/Users/oso/Desktop/texture_mapping/bmp_24.bmp");
  if (!texture) {

    printf("errrrrrrrrr\n");
    free(texture);

    exit(1);
  }

  draw_rectangle_with_texture(texture, 100, 100, "rectangle_with_texture_smaller.ppm");

  return 0;

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
