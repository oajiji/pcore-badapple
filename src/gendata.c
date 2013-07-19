#define UINT64_C(X)  X##ULL

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <stdlib.h>
#include <stdio.h>
#include <lz4.h>

#define DST_WIDTH  640
#define DST_HEIGHT 480
#define FRAME_RATE 30
#define MOVIE_SOURCE "/home/pwx/projects/c++/pcore-badapple/video/badapple.mkv"
#define DATA_PATH    "/home/pwx/projects/c++/pcore/video/badapple.dat"

static FILE* outfile;
static int   grpframe, grpcount;
static uint8_t *picdata, *compressdata, *framedata;
static size_t  compress_size, origin_size;

static void OpenDatafile(const char* path) {
#define PIC_LINE_SIZE  (DST_WIDTH / 2)
#define PIC_FRAME_SIZE (DST_WIDTH * DST_HEIGHT / 2)
#define PIC_GROUP_SIZE (FRAME_RATE * PIC_FRAME_SIZE)
  outfile = fopen(path, "wb");
  if (outfile == NULL)
    exit(-1);
  picdata = (uint8_t*)malloc(PIC_GROUP_SIZE);
  framedata = (uint8_t*)malloc(PIC_FRAME_SIZE);
  compressdata = (uint8_t*)malloc(LZ4_compressBound(PIC_GROUP_SIZE));
  compress_size = origin_size = 0;
  grpcount = 0; grpframe = 0;
}

static void CloseDatafile() {
  char eofmark[4] = {0,0,0,0};
  fwrite(eofmark, 1, 4, outfile);
  fclose(outfile);
  free(picdata);
  free(compressdata);
  free(framedata);
}

extern void linear_to_vga12(const uint8_t *linear, uint8_t *vga12);

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  // convert RGB picture to 16-bit grayscale image data.
  int x, y, k;
  uint8_t *line = pFrame->data[0];
  //uint8_t *data = picdata + PIC_FRAME_SIZE * (grpframe++);
  uint8_t *data = framedata;
  for (y=0; y<height; ++y, line += pFrame->linesize[0], 
       data += PIC_LINE_SIZE) {
    // assume data is pure grayscale RGB.
    for (x=0, k=0; k<width/2; ++k, x+=6) {
      data[k] = (line[x] & 0xf0) | ((line[x+3] >> 4) & 0x0f);
    }
  }
  
  // Convert from linear data to vga12 data.
  linear_to_vga12(framedata, picdata + PIC_FRAME_SIZE * (grpframe++));
  
#if 0
  char filename[32];
  sprintf(filename, "frame%d.dat", iFrame);
  FILE *fp = fopen(filename, "wb");
  fwrite(data - PIC_FRAME_SIZE, 1, PIC_FRAME_SIZE, fp);
  fclose(fp);
#endif
  
  // compress the grayscale image data.
  if (grpframe == FRAME_RATE) {
    grpframe = 0; ++grpcount;
    int compressed = LZ4_compress((const char*)picdata, (char*)compressdata, 
                                  PIC_GROUP_SIZE);
    if (compressed == 0) 
      exit(-1);
    origin_size += PIC_GROUP_SIZE;
    compress_size += compressed;
    fwrite(&compressed, 1, sizeof(compressed), outfile);
    fwrite(compressdata, 1, compressed, outfile);
    printf("Finish data at %d sec.\n", grpcount);
    fflush(stdout);
  }
}

int main() 
{
  AVFormatContext *pFormatCtx = NULL;
  int i, videoStream;
  AVCodecContext *pCodecCtx = NULL;
  AVCodec *pCodec = NULL;
  AVFrame *pFrame = NULL;
  AVFrame *pFrameRGB = NULL;
  AVPacket packet;
  int frameFinished;
  int numBytes;
  uint8_t *buffer = NULL;

  AVDictionary *optionsDict = NULL;
  struct SwsContext *sws_ctx = NULL;
  
  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&pFormatCtx, MOVIE_SOURCE, NULL, NULL)!=0)
    return -1; // Couldn't open file
    
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information
    
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, MOVIE_SOURCE, 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
    
  // Get a pointer to the codec context for the video stream
  pCodecCtx = pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  
  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
    return -1; // Could not open codec
    
  // Allocate video frame
  pFrame = avcodec_alloc_frame();
  
  // Allocate an AVFrame structure
  pFrameRGB = avcodec_alloc_frame();
  if(pFrameRGB==NULL)
    return -1;
  
  // Determine required buffer size and allocate buffer
  numBytes = 
    avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
  buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  
  sws_ctx =
    sws_getContext
    (
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        DST_WIDTH,
        DST_HEIGHT,
        PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );
    
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, 
                 DST_WIDTH, DST_HEIGHT);
  
  // Read frames and save first five frames to disk
  i=0;
  OpenDatafile(DATA_PATH);
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                            &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
        // Convert the image from its native format to RGB
        sws_scale
        (
            sws_ctx,
            (uint8_t const * const *)pFrame->data,
            pFrame->linesize,
            0,
            pCodecCtx->height,
            pFrameRGB->data,
            pFrameRGB->linesize
        );

        // Save the frame to disk
        SaveFrame(pFrameRGB, DST_WIDTH, DST_HEIGHT, i++);
      }
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }
  if (grpframe != 0) {
    printf("Finished job unclean with %d frames left.\n", grpframe);
  }
  printf("Compress rate %f%%.\n", 
         (double)compress_size * 100.0 / (double)origin_size);
  CloseDatafile();
  
  // Free the RGB image
  av_free(buffer);
  av_free(pFrameRGB);
  
  // Free the YUV frame
  av_free(pFrame);
  
  // Close the codec
  avcodec_close(pCodecCtx);
  
  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return 0;
}

