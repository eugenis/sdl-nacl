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

#ifndef _SDL_naclvideo_h
#define _SDL_naclvideo_h

extern "C" {
#include "../SDL_sysvideo.h"
}

#include <nacl/nacl_imc.h>
#include <nacl/nacl_npapi.h>
#include <nacl/npapi_extensions.h>
#include <nacl/npruntime.h>


/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *_this


/* Private display data */

struct SDL_PrivateVideoData {
  NPDevice* device2d_;  // The PINPAPI 2D device.                                                                                                                           
  NPDeviceContext2D context2d_;  // The PINPAPI 2D drawing context.                 

  int bpp;
  int w, h;
  int pitch;
  void *buffer;
  SDL_Color* palette;
};

#endif /* _SDL_naclvideo_h */
