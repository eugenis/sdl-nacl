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

extern "C" {
#include "SDL.h"
#include "SDL_nacl.h"
#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"
}

#include "SDL_naclvideo.h"
#include "SDL_naclevents_c.h"
#include "eventqueue.h"

static EventQueue event_queue;


void SDL_NACL_PushEvent(const PP_InputEvent* ppevent) {

  switch (ppevent->type) {
  case PP_INPUTEVENT_TYPE_MOUSEDOWN:
    printf("mouse down\n");
    break;
  case PP_INPUTEVENT_TYPE_MOUSEUP:
    printf("mouse up\n");
    break;
  case PP_INPUTEVENT_TYPE_MOUSEMOVE:
    printf("mouse move\n");
    break;
  case PP_INPUTEVENT_TYPE_MOUSEENTER:
    printf("mouse enter\n");
    break;
  case PP_INPUTEVENT_TYPE_MOUSELEAVE:
    printf("mouse leave\n");
    break;
  case PP_INPUTEVENT_TYPE_MOUSEWHEEL:
    printf("mouse wheel\n");
    break;
  case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
  case PP_INPUTEVENT_TYPE_KEYDOWN:
  case PP_INPUTEVENT_TYPE_KEYUP:
    printf("key\n");
    break;
  case PP_INPUTEVENT_TYPE_CHAR:
    printf("char\n");
    break;
  // case PP_INPUTEVENT_TYPE_MINIMIZE:
  //   printf("minimize\n");
  //   break;
  // case PP_INPUTEVENT_TYPE_FOCUS:
  //   printf("focus\n");
  //   break;
  // case PP_INPUTEVENT_TYPE_DEVICE:
  //   printf("device\n");
  //   break;
  default:
    printf("unknown\n");
  }

  PP_InputEvent* copy = (PP_InputEvent*)malloc(sizeof(PP_InputEvent));
  memcpy(copy, ppevent, sizeof(PP_InputEvent));
  event_queue.PushEvent(copy);
}

static Uint8 translateButton(/*NPMouseButtons*/ int32_t button) {
  switch (button) {
  case PP_INPUTEVENT_MOUSEBUTTON_LEFT:
    return SDL_BUTTON_LEFT;
  case PP_INPUTEVENT_MOUSEBUTTON_MIDDLE:
    return SDL_BUTTON_MIDDLE;
  case PP_INPUTEVENT_MOUSEBUTTON_RIGHT:
    return SDL_BUTTON_RIGHT;
  case PP_INPUTEVENT_MOUSEBUTTON_NONE:
  default:
   return 0;
  }
}

static SDLKey translateKey(uint32_t code) {
  printf("code: %u '%c'\n", code, code);
  switch (code) {
    case 'A':
      return SDLK_a;
    case 'S':
      return SDLK_s;
    case 'D':
      return SDLK_d;
    case 'W':
      return SDLK_w;
    case 37:
      return SDLK_LEFT;
    case 38:
      return SDLK_UP;
    case 39:
      return SDLK_RIGHT;
    case 40:
      return SDLK_DOWN;
    default:
      return SDLK_UNKNOWN;
  }
}

void NACL_PumpEvents(_THIS)
{
  PP_InputEvent* event;
  SDL_keysym keysym;
  while (event = event_queue.PopEvent()) {
    if (event->type == PP_INPUTEVENT_TYPE_MOUSEDOWN) {
      SDL_PrivateMouseButton(SDL_PRESSED, translateButton(event->u.mouse.button), 0, 0);
    } else if (event->type == PP_INPUTEVENT_TYPE_MOUSEUP) {
      SDL_PrivateMouseButton(SDL_RELEASED, translateButton(event->u.mouse.button), 0, 0);
    } else if (event->type == PP_INPUTEVENT_TYPE_KEYDOWN) {
      keysym.scancode = 0;
      keysym.sym = translateKey(event->u.key.key_code);
      keysym.mod = KMOD_NONE;
      keysym.unicode = 0;
      SDL_PrivateKeyboard(SDL_PRESSED, &keysym);
    } else if (event->type == PP_INPUTEVENT_TYPE_KEYUP) {
      keysym.scancode = 0;
      keysym.sym = translateKey(event->u.key.key_code);
      keysym.mod = KMOD_NONE;
      keysym.unicode = 0;
      SDL_PrivateKeyboard(SDL_RELEASED, &keysym);
    }
    free(event);
  }
}

void NACL_InitOSKeymap(_THIS)
{
	/* do nothing. */
}

/* end of SDL_naclevents.c ... */
