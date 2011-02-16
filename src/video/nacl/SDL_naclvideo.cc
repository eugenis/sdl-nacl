#include "SDL_config.h"

#include <assert.h>

#include "SDL_naclvideo.h"
#include "SDL_naclevents_c.h"

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/image_data.h>

pp::Instance* gNaclPPInstance;
static int gNaclVideoWidth;
static int gNaclVideoHeight;

extern "C" {

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_nacl.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#define NACLVID_DRIVER_NAME "nacl"

void SDL_NACL_SetInstance(PP_Instance instance, int width, int height) {
  gNaclPPInstance = pp::Module::Get()->InstanceForPPInstance(instance);
  gNaclVideoWidth = width;
  gNaclVideoHeight = height;
}

static void flush(void* data, int32_t unused);

/* Initialization/Query functions */
static int NACL_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **NACL_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *NACL_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static void NACL_VideoQuit(_THIS);

/* etc. */
static void NACL_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* NACL driver bootstrap functions */

static int NACL_Available(void)
{
  return !!gNaclPPInstance;
}

static void NACL_DeleteDevice(SDL_VideoDevice *device)
{
  SDL_free(device->hidden);
  SDL_free(device);
}

static SDL_VideoDevice *NACL_CreateDevice(int devindex)
{
  SDL_VideoDevice *device;

  assert(gNaclPPInstance);

  /* Initialize all variables that we clean on shutdown */
  device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
  if ( device ) {
    SDL_memset(device, 0, (sizeof *device));
    device->hidden = (struct SDL_PrivateVideoData *)
        SDL_malloc((sizeof *device->hidden));
  }
  if ( (device == NULL) || (device->hidden == NULL) ) {
    SDL_OutOfMemory();
    if ( device ) {
      SDL_free(device);
    }
    return(0);
  }
  SDL_memset(device->hidden, 0, (sizeof *device->hidden));

  device->hidden->image_data_mu = SDL_CreateMutex();

  device->hidden->ow = gNaclVideoWidth;
  device->hidden->oh = gNaclVideoHeight;

  if (device->hidden->context2d)
    delete device->hidden->context2d;
  device->hidden->context2d = new pp::Graphics2D(gNaclPPInstance,
      pp::Size(device->hidden->ow, device->hidden->oh), false);
  assert(device->hidden->context2d != NULL);

  if (!gNaclPPInstance->BindGraphics(*device->hidden->context2d)) {
    printf("***** Couldn't bind the device context *****\n");
  }

  device->hidden->image_data = new pp::ImageData(gNaclPPInstance,
      PP_IMAGEDATAFORMAT_BGRA_PREMUL,
      device->hidden->context2d->size(),
      false);
  assert(device->hidden->image_data != NULL);

  /* Set the function pointers */
  device->VideoInit = NACL_VideoInit;
  device->ListModes = NACL_ListModes;
  device->SetVideoMode = NACL_SetVideoMode;
  device->UpdateRects = NACL_UpdateRects;
  device->VideoQuit = NACL_VideoQuit;
  device->InitOSKeymap = NACL_InitOSKeymap;
  device->PumpEvents = NACL_PumpEvents;

  device->free = NACL_DeleteDevice;

  flush(device, 0);

  return device;
}

VideoBootStrap NACL_bootstrap = {
  NACLVID_DRIVER_NAME, "SDL Native Client video driver",
  NACL_Available, NACL_CreateDevice
};


int NACL_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
  fprintf(stderr, "CONGRATULATIONS: You are using the SDL nacl video driver!\n");

  /* Determine the screen depth (use default 8-bit depth) */
  /* we change this during the SDL_SetVideoMode implementation... */
  vformat->BitsPerPixel = 8;
  vformat->BytesPerPixel = 1;

  /* We're done! */
  return(0);
}

SDL_Rect **NACL_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
  return (SDL_Rect **) -1;
}


SDL_Surface *NACL_SetVideoMode(_THIS, SDL_Surface *current,
    int width, int height, int bpp, Uint32 flags)
{
  if ( _this->hidden->buffer ) {
    SDL_free( _this->hidden->buffer );
  }

  bpp = 32; // Let SDL handle pixel format conversion.
  width = _this->hidden->ow;
  height = _this->hidden->oh;

  _this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
  if ( ! _this->hidden->buffer ) {
    SDL_SetError("Couldn't allocate buffer for requested mode");
    return(NULL);
  }

  SDL_memset(_this->hidden->buffer, 0, width * height * (bpp / 8));

  /* Allocate the new pixel format for the screen */
  if ( ! SDL_ReallocFormat(current, bpp, 0xFF0000, 0xFF00, 0xFF, 0xFF000000) ) {
    SDL_free(_this->hidden->buffer);
    _this->hidden->buffer = NULL;
    SDL_SetError("Couldn't allocate new pixel format for requested mode");
    return(NULL);
  }

  /* Set up the new mode framebuffer */
  current->flags = flags & SDL_FULLSCREEN;
  _this->hidden->bpp = bpp;
  _this->hidden->w = current->w = width;
  _this->hidden->h = current->h = height;
  _this->hidden->pitch = current->pitch = current->w * (bpp / 8);
  current->pixels = _this->hidden->buffer;

  /* We're done */
  return(current);
}


static void flush(void* data, int32_t unused) {
  SDL_VideoDevice* _this = reinterpret_cast<SDL_VideoDevice*>(data);

  SDL_LockMutex(_this->hidden->image_data_mu);
  _this->hidden->context2d->PaintImageData(*_this->hidden->image_data, pp::Point());
  _this->hidden->context2d->Flush(pp::CompletionCallback(&flush, _this));
  SDL_UnlockMutex(_this->hidden->image_data_mu);
}

static void flip(_THIS) {
  // printf("flip: h %d w %d bpp %d\n", _this->hidden->h, _this->hidden->w, _this->hidden->bpp);
  // assert(_this->hidden->bpp == 8);
  assert(_this->hidden->image_data);
  assert(_this->hidden->w == _this->hidden->ow);
  assert(_this->hidden->h == _this->hidden->oh);

  SDL_LockMutex(_this->hidden->image_data_mu);


  SDL_memcpy(_this->hidden->image_data->data(), _this->hidden->buffer,
      _this->hidden->w * _this->hidden->h * _this->hidden->bpp / 8);

  SDL_UnlockMutex(_this->hidden->image_data_mu);

}

static void NACL_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
  if (_this->hidden->bpp == 0) // not initialized yet
    return;
  flip(_this);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void NACL_VideoQuit(_THIS)
{
  if (_this->screen->pixels != NULL)
  {
    SDL_free(_this->screen->pixels);
    _this->screen->pixels = NULL;
  }
  delete _this->hidden->context2d;
  delete _this->hidden->image_data;
}
} // extern "C"
