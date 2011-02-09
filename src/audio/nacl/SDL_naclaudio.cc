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

    This file written by Ryan C. Gordon (icculus@icculus.org)
*/
#include "SDL_config.h"

#include <assert.h>

#include "SDL_naclaudio.h"

#include <ppapi/cpp/instance.h>

extern pp::Instance* global_pp_instance;

extern "C" {

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "SDL_mutex.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"

/* The tag name used by NACL audio */
#define NACLAUD_DRIVER_NAME         "nacl"

const uint32_t kSampleFrameCount = 4096u;

/* Audio driver functions */
static int NACLAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void NACLAUD_WaitAudio(_THIS);
static void NACLAUD_PlayAudio(_THIS);
static Uint8 *NACLAUD_GetAudioBuf(_THIS);
static void NACLAUD_CloseAudio(_THIS);

static void AudioCallback(void* samples, size_t buffer_size, void* data);


/* Audio driver bootstrap functions */
static int NACLAUD_Available(void)
{
  const char *envr = SDL_getenv("SDL_AUDIODRIVER");
  // Available if global_pp_instance is set and SDL_AUDIODRIVER is either unset, empty, or "nacl".
  if (global_pp_instance &&
      (!envr || !*envr || SDL_strcmp(envr, NACLAUD_DRIVER_NAME) == 0)) {
    printf("nacl audio is available\n");
    return 1;
  }
  return 0;
}

static void NACLAUD_DeleteDevice(SDL_AudioDevice *device)
{
  // stop playback? likely main thread only.
}

static SDL_AudioDevice *NACLAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *_this;

	printf("NACLAUD_CreateDevice\n");
	/* Initialize all variables that we clean on shutdown */
	_this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( _this ) {
		SDL_memset(_this, 0, (sizeof *_this));
		_this->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *_this->hidden));
	}
	if ( (_this == NULL) || (_this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( _this ) {
			SDL_free(_this);
		}
		return(0);
	}
	SDL_memset(_this->hidden, 0, (sizeof *_this->hidden));


        _this->hidden->sample_frame_count =
            pp::AudioConfig::RecommendSampleFrameCount(PP_AUDIOSAMPLERATE_44100,
                kSampleFrameCount);
        _this->hidden->audio = pp::Audio(
            global_pp_instance,
            pp::AudioConfig(global_pp_instance,
                PP_AUDIOSAMPLERATE_44100,
                _this->hidden->sample_frame_count),
            AudioCallback, _this);

        printf("starting audio playback\n");
        _this->hidden->audio.StartPlayback();

	/* Set the function pointers */
	_this->OpenAudio = NACLAUD_OpenAudio;
	_this->WaitAudio = NACLAUD_WaitAudio;
	_this->PlayAudio = NACLAUD_PlayAudio;
	_this->GetAudioBuf = NACLAUD_GetAudioBuf;
	_this->CloseAudio = NACLAUD_CloseAudio;

	_this->free = NACLAUD_DeleteDevice;

	return _this;
}

AudioBootStrap NACLAUD_bootstrap = {
	NACLAUD_DRIVER_NAME, "SDL nacl audio driver",
	NACLAUD_Available, NACLAUD_CreateDevice
};

/* This function waits until it is possible to write a full sound buffer */
static void NACLAUD_WaitAudio(_THIS)
{
  assert(0);
}

static void NACLAUD_PlayAudio(_THIS)
{
  assert(0);
}

static Uint8 *NACLAUD_GetAudioBuf(_THIS)
{
}

static void NACLAUD_CloseAudio(_THIS)
{
}


static void AudioCallback(void* samples, size_t buffer_size, void* data) {
  SDL_AudioDevice* _this = reinterpret_cast<SDL_AudioDevice*>(data);
  printf("============== audio callback\n");


  // const double frequency = 440;
  // const double delta = kTwoPi * frequency / PP_AUDIOSAMPLERATE_44100;
  // const int16_t max_int16 = std::numeric_limits<int16_t>::max();

  //   int16_t* buff = reinterpret_cast<int16_t*>(samples);

  //   // Make sure we can't write outside the buffer.
  //   assert(buffer_size >= (sizeof(*buff) * kChannels *
  //           _this->hidden->sample_frame_count));

  //   double theta = 0.;

  //   for (size_t sample_i = 0;
  //        sample_i < _this->hidden->sample_frame_count;
  //        ++sample_i, theta += delta) {
  //     // Keep theta_ from going beyond 2*Pi.
  //     if (theta > kTwoPi) {
  //       theta -= kTwoPi;
  //     }
  //     double sin_value(std::sin(theta));
  //     int16_t scaled_value = static_cast<int16_t>(sin_value * max_int16);
  //     for (size_t channel = 0; channel < kChannels; ++channel) {
  //       *buff++ = scaled_value;
  //     }
  //   }

  _this->spec.callback(_this->spec.userdata, (Uint8*)samples, buffer_size);

  return;
}


static int NACLAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
  printf("NACLAUD_OpenAudio\n");

  // We don't give a damn what the user wants.
  spec->freq = 44100;
  spec->format = AUDIO_S16LSB;
  spec->channels = 2;
  spec->samples = _this->hidden->sample_frame_count;

  // Do not create an audio thread.
  return 1;
}
} // extern "C"
