#ifndef _SDL_nacl_eventqueue_h
#define _SDL_nacl_eventqueue_h

#include "SDL_mutex.h"

#include <queue>
#include <ppapi/c/pp_input_event.h>

class EventQueue {
public:
  EventQueue() {
    mu_ = SDL_CreateMutex();
  }

  ~EventQueue() {
    SDL_DestroyMutex(mu_);
  }

  PP_InputEvent* PopEvent() {
    SDL_LockMutex(mu_);
    PP_InputEvent* event = NULL;
    if (!queue_.empty()) {
      event = queue_.front();
      queue_.pop();
    }
    SDL_UnlockMutex(mu_);
    return event;
  }

  void PushEvent(PP_InputEvent* event) {
    SDL_LockMutex(mu_);
    queue_.push(event);
    SDL_UnlockMutex(mu_);
  }

private:
  std::queue<PP_InputEvent*> queue_;
  SDL_mutex* mu_;
};

#endif // _SDL_nacl_eventqueue_h
