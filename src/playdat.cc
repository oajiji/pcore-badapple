#define DST_WIDTH  640
#define DST_HEIGHT 480
#define FRAME_RATE 30
#define PIC_LINE_SIZE  (DST_WIDTH / 2)
#define PIC_FRAME_SIZE (DST_WIDTH * DST_HEIGHT / 2)
#define PIC_GROUP_SIZE (FRAME_RATE * PIC_FRAME_SIZE)

#define DATA_OFFSET  52428800
#define DATA_PATH    "/home/pwx/projects/c++/pcore/build/pcore.img" 

#include <lz4.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <opencv2/opencv.hpp>

cv::Mat image(DST_HEIGHT, DST_WIDTH, CV_8UC3);

extern "C" void vga12_to_linear(const uint8_t *vga12, uint8_t *linear);

static void showGroup(uint8_t* group, uint8_t *framedata)
{
  int i, x, y;
  for (i=0; i<30; ++i, group += PIC_FRAME_SIZE) {
    uint8_t *line = framedata;
    vga12_to_linear(group, framedata);
    
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
    cv::waitKey(33);
  }
}

int main() 
{
  // Initialize opencv.
  cv::namedWindow("main", CV_WINDOW_AUTOSIZE);
  
  // Load all data into memory.
  FILE *fp = fopen(DATA_PATH, "rb");
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, DATA_OFFSET, SEEK_SET);
  fsize -= DATA_OFFSET;
  
  char *data = (char*)malloc(fsize);
  char *pdata = (char*)malloc(PIC_GROUP_SIZE);
  uint8_t *framedata = (uint8_t*)malloc(PIC_FRAME_SIZE);
  
  if (fread(data, 1, fsize, fp) == 0) {
    printf("Cannot read data.\n");
    return -1;
  }
  fclose(fp);
  
  // Play the frames.
  char* p = data;
  int group = 0;
  
  for (;;) {
    // get next group compressed size.
    int compress_size = *(int*)p;
    p += sizeof(int);
    if (compress_size == 0) {
      break;
    }
    // decompress this group.
    int size = LZ4_decompress_safe(p, pdata, 
                                   compress_size, PIC_GROUP_SIZE);
    assert (size == PIC_GROUP_SIZE);
    p += compress_size; ++group;
    // Set the ready flag.
    showGroup((uint8_t*)pdata, framedata);
    printf("[badapple] Decoded %d sec.\n", group);
  }
  
  return 0;
}
