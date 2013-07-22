#include <lz4.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "wav.h"

#define WAVE_PATH  "/home/pwx/projects/c++/pcore-badapple/video/badapple.wav"
#define OUT_PATH   "/home/pwx/projects/c++/pcore-badapple/video/audio.dat"

int main() {
  FILE *fp = fopen(WAVE_PATH, "rb");
  if (fp == NULL)
    return -1;
  
  struct RIFF_CHUNK riff;
  struct FORMAT_CHUNK format;
  struct DATA_CHUNK data;
  
  // READ RIFF header
  fread(&riff, 1, sizeof(riff), fp);
  assert( riff.fccID == RIFF && riff.fccType == WAVE );
  
  // Read Format
  uint32_t *format_p = (uint32_t*)&format;
  fread(format_p, 1, 8, fp);
  fread(format_p + 2, 1, format_p[1], fp);
  
  assert( format.dwSize == 16 || format.dwSize == 18 );
  assert( format.wFormatTag == 1 );
  assert( format.wChannels == 1 || format.wChannels == 2 );
  assert( format.dwSamplesPerSec == 11025 || format.dwSamplesPerSec == 22050
          || format.dwSamplesPerSec == 44100 );
  assert( format.uiBitsPerSample == 8 || format.uiBitsPerSample == 16 );
  assert( format.fccID == FMT_ );
  assert( format.dwAvgBytePerSec == format.wBlockAlign * format.dwSamplesPerSec );
  assert( format.wBlockAlign == (format.uiBitsPerSample >> 3) * format.wChannels );
  
  // Read data
  fread(&data, 1, sizeof(data), fp);
  if ( data.fccID == FACT ) {
    fread(&data, 1, sizeof(struct FACT_CHUNK) - sizeof(data), fp);
    fread(&data, 1, sizeof(data), fp);
  }
  assert( data.fccID == DATA );
  
  // Dump information.
  printf("channel %d, sample rate %d, byte rate %d, block align %d, "
         "bit rate %d, data size %u\n",
         format.wChannels, format.dwSamplesPerSec, format.dwAvgBytePerSec, 
         format.wBlockAlign, format.uiBitsPerSample, data.dwSize
  );
  
  // Read all data.
  char* d = malloc(data.dwSize);
  assert( fread(d, 1, data.dwSize, fp) == data.dwSize );
  
  // Close file.
  fclose(fp);
  
#if 0
  // Compress data.
  char* compressed = malloc(LZ4_compressBound(data.dwSize));
  int compressed_size = LZ4_compress(d, compressed, data.dwSize);
  if (compressed_size <= 0)
    return -1;
  
  // Save compressed data.
  FILE *outfile = fopen(OUT_PATH, "wb");
  if (outfile == NULL)
    return -1;
  fwrite(&compressed_size, 1, sizeof(compressed_size), outfile);
  fwrite(compressed, 1, compressed_size, outfile);
  fclose(outfile);
  
  // Dump compress rate.
  printf("Compress rate %f%%\n", 
         (double)compressed_size * 100.0 / (double)data.dwSize);
#endif

  // Save raw data.
  FILE *outfile = fopen(OUT_PATH, "wb");
  fwrite(d, 1, data.dwSize, outfile);
  fclose(outfile);
  
  return 0;
}
