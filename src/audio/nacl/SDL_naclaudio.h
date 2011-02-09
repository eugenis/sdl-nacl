#include "SDL_config.h"

#ifndef _SDL_naclaudio_h
#define _SDL_naclaudio_h

extern "C" {
#include "SDL_audio.h"
#include "../SDL_sysaudio.h"
}

#include <ppapi/cpp/audio.h>

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_AudioDevice *_this

struct SDL_PrivateAudioData {

  int sample_frame_count;
  pp::Audio audio;
};

#endif /* _SDL_naclaudio_h */
