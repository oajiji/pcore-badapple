// This file converts to / from vga12 4plane image data.

#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#define WIDTH   640
#define HEIGHT  480
#define PLANE_SIZE ((WIDTH * HEIGHT / 2) / 4)
#define GET_BIT(V, N) (((V) >> (N)) & 0x1)

void linear_to_vga12(const uint8_t *linear, uint8_t *vga12)
{
  
  int i, m;
  
  // loop for each plane.
  for (m = 0; m < 4; ++m) {
    // Reset pointer to the first unit.
    uint8_t *p = vga12 + m * PLANE_SIZE;
    const uint8_t *d = linear;
    // loop for each line.
    for (i = 0; i < HEIGHT; ++i) {
      uint8_t *pend = p + (WIDTH >> 3);
      // loop for each group.
      for (; p < pend; ++p, d += 4) {
        *p = (GET_BIT(*d, m+4) << 7)
            | (GET_BIT(*d, m) << 6)
            | (GET_BIT(*(d+1), m+4) << 5)
            | (GET_BIT(*(d+1), m) << 4)
            | (GET_BIT(*(d+2), m+4) << 3)
            | (GET_BIT(*(d+2), m) << 2)
            | (GET_BIT(*(d+3), m+4) << 1)
            | (GET_BIT(*(d+3), m) << 0);
      }
    }
  }
}

void vga12_to_linear(const uint8_t *vga12, uint8_t *linear)
{
  int i, m;
  
  // clear output to zero.
  memset(linear, 0, (WIDTH * HEIGHT) >> 1);
  // loop for each plane.
  for (m = 0; m < 4; ++m) {
    // Reset pointer to the first unit.
    const uint8_t *p = vga12 + m * PLANE_SIZE;
    uint8_t *d = linear;
    // loop for each line.
    for (i = 0; i < HEIGHT; ++i) {
      const uint8_t *pend = p + (WIDTH >> 3);
      // loop for each group.xx
      for (; p < pend; ++p) {
        uint8_t v = *p;
        *d++ |= (GET_BIT(v, 7) << (m + 4)) | (GET_BIT(v, 6) << (m));
        *d++ |= (GET_BIT(v, 5) << (m + 4)) | (GET_BIT(v, 4) << (m));
        *d++ |= (GET_BIT(v, 3) << (m + 4)) | (GET_BIT(v, 2) << (m));
        *d++ |= (GET_BIT(v, 1) << (m + 4)) | (GET_BIT(v, 0) << (m));
      }
    }
  }
}
