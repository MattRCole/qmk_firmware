
/*******************************************************************************
* image
* filename: unsaved
* name: mode-fn
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



__attribute__((weak)) const uint8_t resource_mode_fn[128] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x3f, 0x27, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x3f, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x03, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x03, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x3f, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x3f, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x3f, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x3f, 0x3b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
