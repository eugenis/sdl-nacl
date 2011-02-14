#include "SDL_config.h"

#include <assert.h>

#include "SDL_naclvideo.h"
#include "SDL_naclevents_c.h"
#include "SDL_naclmouse_c.h"

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
  if (gNaclPPInstance &&
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

	assert(gNaclPPInstance);

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

        device->hidden->ow = gNaclVideoWidth;
        device->hidden->oh = gNaclVideoHeight;

        printf("initialize 2d graphics... (instance %p)\n", (void*)gNaclPPInstance);
        if (device->hidden->context2d)
          delete device->hidden->context2d;
        device->hidden->context2d = new pp::Graphics2D(gNaclPPInstance,
            pp::Size(device->hidden->ow, device->hidden->oh), false);
        assert(device->hidden->context2d != NULL);

        printf("binding graphics\n");
        if (!gNaclPPInstance->BindGraphics(*device->hidden->context2d)) {
          printf("***** Couldn't bind the device context *****\n");
        }

        printf("allocating imagedata\n");
        device->hidden->image_data = new pp::ImageData(gNaclPPInstance,
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

	bpp = 32; // Let SDL handle pixel format conversion.
	width = _this->hidden->ow;
	height = _this->hidden->oh;

	_this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
	if ( ! _this->hidden->buffer ) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return(NULL);
	}

	// if (bpp == 8) {
	//   _this->hidden->palette = (SDL_Color*)SDL_malloc(256 * sizeof(SDL_Color));
	//   if ( ! _this->hidden->palette ) {
	//     SDL_SetError("Couldn't allocate palette for requested 8-bit mode");
	//     return(NULL);
	//   }
	// }

/* 	printf("Setting mode %dx%d\n", width, height); */

	SDL_memset(_this->hidden->buffer, 0, width * height * (bpp / 8));
	// if (_this->hidden->palette) {
	//   SDL_memset(_this->hidden->palette, 0, 256 * sizeof(SDL_Color));
	// }

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


static void flush(void* data, int32_t unused) {
  SDL_VideoDevice* _this = reinterpret_cast<SDL_VideoDevice*>(data);

  SDL_LockMutex(_this->hidden->image_data_mu);

  // fprintf(stderr, "paint\n");
  _this->hidden->context2d->PaintImageData(*_this->hidden->image_data, pp::Point());

  // fprintf(stderr, "flush\n");
  _this->hidden->context2d->Flush(pp::CompletionCallback(&flush, _this));

  SDL_UnlockMutex(_this->hidden->image_data_mu);
}

static void NACL_SetCaption(_THIS, const char* title, const char* icon) {
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
