
/*******************************************************************************
* image
* filename: unsaved
* name: mode-cod
*
* preset name: Monochrome
* data block size: 8 bit(s), uint8_t
* RLE compression enabled: no
* conversion type: Monochrome, Edge 128
* bits per pixel: 1
*
* preprocess:
*  main scan direction: top_to_bottom
*  line scan direction: forward
*  inverse: no
*******************************************************************************/

/*
 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
     } tImage;
*/
#include "resources.h"



__attribute__((weak)) const uint8_t resource_mode_cod[128] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x26, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 
    0x26, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 
    0x24, 0xc3, 0x91, 0x80, 0xc3, 0x91, 0xc3, 0xff, 
    0xa4, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff, 
    0xac, 0xfd, 0x9c, 0xe7, 0xfd, 0x9c, 0x9d, 0xff, 
    0xa8, 0xc1, 0x9d, 0xe7, 0xc1, 0x9d, 0x81, 0xff, 
    0x89, 0x81, 0x9f, 0xe7, 0x81, 0x9f, 0x81, 0xff, 
    0x89, 0x9d, 0x9f, 0xe7, 0x9d, 0x9f, 0x9f, 0xff, 
    0x99, 0x81, 0x9f, 0xe7, 0x81, 0x9f, 0x81, 0xff, 
    0x99, 0xc5, 0x9f, 0xe7, 0xc5, 0x9f, 0xc3, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
