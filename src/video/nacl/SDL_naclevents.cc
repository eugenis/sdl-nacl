#include "SDL_config.h"

extern "C" {
#include "SDL.h"
#include "SDL_nacl.h"
#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"
}

#include "SDL_naclevents_c.h"
#include "eventqueue.h"

static EventQueue event_queue;

void SDL_NACL_PushEvent(const PP_InputEvent* ppevent) {
  PP_InputEvent* copy = (PP_InputEvent*)malloc(sizeof(PP_InputEvent));
  memcpy(copy, ppevent, sizeof(PP_InputEvent));
  event_queue.PushEvent(copy);
}

static Uint8 translateButton(int32_t button) {
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
  // TODO: Some keys are missing from this list.
  if (code >= 'A' && code <= 'Z')
    return (SDLKey)(code - 'A' + SDLK_a);
  if (code >= SDLK_0 && code <= SDLK_9)
    return (SDLKey)code;
  const uint32_t f1_code = 112;
  if (code >= f1_code && code < f1_code + 12)
    return (SDLKey)(code - f1_code + SDLK_F1);
  switch (code) {
    case SDLK_BACKSPACE:
      return SDLK_BACKSPACE;
    case SDLK_TAB:
      return SDLK_TAB;
    case SDLK_RETURN:
      return SDLK_RETURN;
    case SDLK_PAUSE:
      return SDLK_PAUSE;
    case SDLK_ESCAPE:
      return SDLK_ESCAPE;
    case 16:
      return SDLK_LSHIFT;
    case 17:
      return SDLK_LCTRL;
    case 18:
      return SDLK_LALT;
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

void NACL_PumpEvents(_THIS) {
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

void NACL_InitOSKeymap(_THIS) {
  /* do nothing. */
}

/* end of SDL_naclevents.c ... */
