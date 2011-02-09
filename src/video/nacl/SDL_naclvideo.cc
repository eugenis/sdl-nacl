/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#include <assert.h>

#include "SDL_naclvideo.h"
#include "SDL_naclevents_c.h"
#include "SDL_naclmouse_c.h"

// extern NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/image_data.h>

pp::Instance* global_pp_instance;

extern "C" {

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_nacl.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#define NACLVID_DRIVER_NAME "nacl"

void SDL_NACL_SetInstance(void* instance) {
  global_pp_instance = reinterpret_cast<pp::Instance*>(instance);
}

static void flush(void* data, int32_t unused);

/* Initialization/Query functions */
static int NACL_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **NACL_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *NACL_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int NACL_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void NACL_VideoQuit(_THIS);

/* Hardware surface functions */
static int NACL_AllocHWSurface(_THIS, SDL_Surface *surface);
static int NACL_LockHWSurface(_THIS, SDL_Surface *surface);
static void NACL_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void NACL_FreeHWSurface(_THIS, SDL_Surface *surface);

/* etc. */
static void NACL_SetCaption(_THIS, const char* title, const char* icon);
static void NACL_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* NACL driver bootstrap functions */

static int NACL_Available(void)
{
  const char *envr = SDL_getenv("SDL_VIDEODRIVER");
  // Available if NPP is set and SDL_VIDEODRIVER is either unset, empty, or "nacl".
  if (global_pp_instance &&
      (!envr || !*envr || SDL_strcmp(envr, NACLVID_DRIVER_NAME) == 0)) {
    printf("nacl video is available\n");
    return 1;
  }
  return 0;
}

static void NACL_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_VideoDevice *NACL_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	assert(global_pp_instance);

	printf("Creating a NaCl device\n");
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

        device->hidden->ow = 400;
        device->hidden->oh = 256;

        printf("initialize 2d graphics... (instance %p)\n", (void*)global_pp_instance);
        if (device->hidden->context2d)
          delete device->hidden->context2d;
        device->hidden->context2d = new pp::Graphics2D(global_pp_instance,
            pp::Size(device->hidden->ow, device->hidden->oh), false);
        assert(device->hidden->context2d != NULL);

        printf("binding graphics\n");
        if (!global_pp_instance->BindGraphics(*device->hidden->context2d)) {
          printf("***** Couldn't bind the device context *****\n");
        }

        printf("allocating imagedata\n");
        device->hidden->image_data = new pp::ImageData(global_pp_instance,
            PP_IMAGEDATAFORMAT_BGRA_PREMUL,
            device->hidden->context2d->size(),
            false);
        assert(device->hidden->image_data != NULL);

        printf("PP magic successful\n");


	/* Set the function pointers */
	device->VideoInit = NACL_VideoInit;
	device->ListModes = NACL_ListModes;
	device->SetVideoMode = NACL_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = NACL_SetColors;
	device->UpdateRects = NACL_UpdateRects;
	device->VideoQuit = NACL_VideoQuit;
	device->AllocHWSurface = NACL_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = NACL_LockHWSurface;
	device->UnlockHWSurface = NACL_UnlockHWSurface;
	device->FlipHWSurface = NULL;
	device->FreeHWSurface = NACL_FreeHWSurface;
	device->SetCaption = NACL_SetCaption;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
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
  printf("SetVideoMode: %dx%dx%d, flags %u\n", width, height, bpp, (unsigned)flags);
	if ( _this->hidden->buffer ) {
		SDL_free( _this->hidden->buffer );
	}

	_this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
	if ( ! _this->hidden->buffer ) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return(NULL);
	}

	if (bpp == 8) {
	  _this->hidden->palette = (SDL_Color*)SDL_malloc(256 * sizeof(SDL_Color));
	  if ( ! _this->hidden->palette ) {
	    SDL_SetError("Couldn't allocate palette for requested 8-bit mode");
	    return(NULL);
	  }
	}

/* 	printf("Setting mode %dx%d\n", width, height); */

	SDL_memset(_this->hidden->buffer, 0, width * height * (bpp / 8));
	if (_this->hidden->palette) {
	  SDL_memset(_this->hidden->palette, 0, 256 * sizeof(SDL_Color));
	}

	/* Allocate the new pixel format for the screen */
	if ( ! SDL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
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

/* We don't actually allow hardware surfaces other than the main one */
static int NACL_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}
static void NACL_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int NACL_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return(0);
}

static void NACL_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

// static void write_ppm(_THIS, char* fname) {
//   FILE* fp = fopen(fname, "w");
//   int x, y;
//   assert(_this->hidden->bpp == 8);
//   fprintf(fp, "P3\n%d %d\n255\n", _this->hidden->w, _this->hidden->h);
//   for (y = 0; y < _this->hidden->h; ++y) {
//     unsigned char* pixels = ((unsigned char*)_this->hidden->buffer) + _this->hidden->pitch * y;
//     for (x = 0; x < _this->hidden->w; ++x) {
//       SDL_Color pixel = _this->hidden->palette[pixels[x]];
//       fprintf(fp, "%d %d %d  ", pixel.r, pixel.g, pixel.b);
//     }
//     fprintf(fp, "\n");
//   }
//   fclose(fp);
// }

// This is called by the brower when the 2D context has been flushed to the
// browser window.
// static void FlushCallback(NPP instance, NPDeviceContext* context,
//   NPError err, void* user_data) {
// }



static void flush(void* data, int32_t unused) {
  SDL_VideoDevice* _this = reinterpret_cast<SDL_VideoDevice*>(data);

  SDL_LockMutex(_this->hidden->image_data_mu);

  fprintf(stderr, "paint\n");
  _this->hidden->context2d->PaintImageData(*_this->hidden->image_data, pp::Point());

  fprintf(stderr, "flush\n");
  _this->hidden->context2d->Flush(pp::CompletionCallback(&flush, _this));

  SDL_UnlockMutex(_this->hidden->image_data_mu);
}

static void NACL_SetCaption(_THIS, const char* title, const char* icon) {
}

static void flip(_THIS) {
  // printf("flip: this %p\n", _this);
  printf("flip: h %d w %d bpp %d\n", _this->hidden->h, _this->hidden->w, _this->hidden->bpp);
  assert(_this->hidden->bpp == 8);
  assert(_this->hidden->image_data);
  assert(_this->hidden->w <= _this->hidden->ow);
  assert(_this->hidden->h <= _this->hidden->oh);

  SDL_LockMutex(_this->hidden->image_data_mu);

  uint32_t* pixel_bits = static_cast<uint32_t*>(_this->hidden->image_data->data());
  assert(pixel_bits);
  // printf("blitting to %p\n", pixel_bits);
  for (int y = 0; y < _this->hidden->h; ++y) {
    // printf("y = %d\n", y);
    unsigned char* pixels = ((unsigned char*)_this->hidden->buffer) + _this->hidden->pitch * y;
    for (int x = 0; x < _this->hidden->w; ++x) {
      SDL_Color color = _this->hidden->palette[pixels[x]];
      uint32_t val = 0xFF000000 + ((uint32_t)color.r << 16) + ((uint32_t)color.g << 8) +
	((uint32_t)color.b);
      pixel_bits[_this->hidden->ow * y + x] = val; // use stride() ?
    }
  }
  // flush(_this);

  SDL_UnlockMutex(_this->hidden->image_data_mu);

  // pp::Module::Get()->core()->CallOnMainThread(0, pp::CompletionCallback(flush, _this), 0);


  // NPN_PluginThreadAsyncCall(global_npp, (void (*)(void*))&flush, _this);
}

static void NACL_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
  /* static int frameno; */
  /* char fname[100]; */
  /* if (_this->hidden->bpp == 0) // not initialized yet */
  /*   return; */
  /* snprintf(fname, 100, "frame%05d.ppm", frameno++); */
  /* write_ppm(_this, fname); */

  if (_this->hidden->bpp == 0) // not initialized yet
    return;
  flip(_this);
}

int NACL_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
  int i;
  assert(_this->hidden->bpp == 8);
  assert(_this->hidden->palette);
  for (i = 0; i < ncolors; ++i)
    _this->hidden->palette[firstcolor + i] = colors[i];
  return(1);
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
        // TODO(eugenis): free all the PP stuff
}
} // extern "C"
