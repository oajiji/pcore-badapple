#define DST_WIDTH  640
#define DST_HEIGHT 480
#define FRAME_RATE 30
#define PIC_LINE_SIZE  (DST_WIDTH / 2)
#define PIC_FRAME_SIZE (DST_WIDTH * DST_HEIGHT / 2)
#define PIC_GROUP_SIZE (FRAME_RATE * PIC_FRAME_SIZE)

#include <lz4.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <opencv2/opencv.hpp>

cv::Mat image;

int main(int argc, char **argv) 
{
  // Check parameters 
  if (argc < 3) {
    printf("mkimg input-image output-file\n");
    return 0;
  }
  
  // Initialize opencv.
  cv::namedWindow("main", CV_WINDOW_AUTOSIZE);
  
  // Load the image.
  image = cv::imread(argv[1]);
  if (!image.data) {
    printf("Cannot read input image!\n");
    return -1;
  }
  
  // Save the output data.
  FILE *outfile = fopen(argv[2], "wb");
  if (!outfile) {
    printf("Cannot open output file!\n");
  }
  
    // convert RGB picture to 16-bit grayscale image data.
  int x, y, k;
  uint8_t *picdata = (uint8_t*)malloc(PIC_FRAME_SIZE);
  uint8_t *data = picdata;
  for (y=0; y<DST_HEIGHT; ++y) {
    // assume data is pure grayscale RGB.
    for (x=0, k=0; k<DST_WIDTH/2; ++k, x+=2) {
      uint8_t a1, a2;
      a1 = image.at<cv::Vec3b>(y, x)[0];
      a2 = image.at<cv::Vec3b>(y, x+1)[0];
      data[k] = (a1 & 0xf0) | ((a2 >> 4) & 0x0f);
    }
    data += PIC_LINE_SIZE;
  }
  printf("Image data converted.\n");
  
  // Compress the image.
  int maxsize = LZ4_compressBound(PIC_FRAME_SIZE);
  uint8_t *compressdata = (uint8_t*)malloc(maxsize);
  int compressed = 
    LZ4_compress((const char*)picdata, (char*)compressdata, PIC_FRAME_SIZE);
  if (compressed == 0) {
    printf("Compress error!\n");
  }
  printf("Data compressed!\n");
  
  fwrite(&compressed, 1, 4, outfile);
  fwrite(compressdata, 1, compressed, outfile);
  fclose(outfile);
  
  printf("Job finished!\n");
  
  // Revert display the image.
  uint8_t *line = picdata;
    
  for (y=0; y<DST_HEIGHT; ++y, line += PIC_LINE_SIZE) {
    for (x=0; x<DST_WIDTH/2; ++x) {
      uint8_t a1, a2;
      a1 = (line[x] & 0xf0);
      a2 = (line[x] & 0x0f) << 4;
      image.at<cv::Vec3b>(y, 2*x) = cv::Vec3b(a1, a1, a1);
      image.at<cv::Vec3b>(y, 2*x+1) = cv::Vec3b(a2, a2, a2);
    }
  }
  
  cv::imshow("main", image);
  cv::waitKey(0);
  
  free(picdata);
  free(compressdata);
  return 0;
}
