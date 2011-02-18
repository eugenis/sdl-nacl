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
  if (code >= 'A' && code <= 'Z')
    return (SDLKey)(code - 'A' + SDLK_a);
  if (code >= SDLK_0 && code <= SDLK_9)
    return (SDLKey)code;
  const uint32_t f1_code = 112;
  if (code >= f1_code && code < f1_code + 12)
    return (SDLKey)(code - f1_code + SDLK_F1);
  const uint32_t kp0_code = 96;
  if (code >= kp0_code && code < kp0_code + 10)
    return (SDLKey)(code - kp0_code + SDLK_KP0);
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
    case 106:
      return SDLK_KP_MULTIPLY;
    case 107:
      return SDLK_KP_PLUS;
    case 109:
      return SDLK_KP_MINUS;
    case 110:
      return SDLK_KP_PERIOD;
    case 111:
      return SDLK_KP_DIVIDE;
    case 45:
      return SDLK_INSERT;
    case 46:
      return SDLK_DELETE;
    case 36:
      return SDLK_HOME;
    case 35:
      return SDLK_END;
    case 33:
      return SDLK_PAGEUP;
    case 34:
      return SDLK_PAGEDOWN;
    case 189:
      return SDLK_MINUS;
    case 187:
      return SDLK_EQUALS;
    case 219:
      return SDLK_LEFTBRACKET;
    case 221:
      return SDLK_RIGHTBRACKET;
    case 186:
      return SDLK_SEMICOLON;
    case 222:
      return SDLK_QUOTE;
    case 220:
      return SDLK_BACKSLASH;
    case 188:
      return SDLK_COMMA;
    case 190:
      return SDLK_PERIOD;
    case 191:
      return SDLK_SLASH;
    case 192:
      return SDLK_BACKQUOTE;
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
    } else if (event->type == PP_INPUTEVENT_TYPE_MOUSEMOVE) {
      SDL_PrivateMouseMotion(0, 0, event->u.mouse.x, event->u.mouse.y);
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
