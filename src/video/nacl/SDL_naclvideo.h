#include "SDL_config.h"

#ifndef _SDL_naclvideo_h
#define _SDL_naclvideo_h

extern "C" {
#include "../SDL_sysvideo.h"
#include "SDL_mutex.h"
}

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_2d.h>


/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *_this


/* Private display data */

struct SDL_PrivateVideoData {
  int bpp;
  int w, h;
  int pitch;
  void *buffer;

  SDL_mutex* image_data_mutex;
  int ow, oh; // plugin output dimensions
  pp::ImageData* image_data;
  pp::Graphics2D* context2d;  // The PINPAPI 2D drawing context.
};

#endif /* _SDL_naclvideo_h */
