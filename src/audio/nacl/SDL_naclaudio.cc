
#include "SDL_config.h"
#include "SDL_naclaudio.h"

#include <assert.h>
#include <ppapi/cpp/instance.h>

extern pp::Instance* gNaclPPInstance;

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
static void NACLAUD_CloseAudio(_THIS);

static void AudioCallback(void* samples, size_t buffer_size, void* data);


/* Audio driver bootstrap functions */
static int NACLAUD_Available(void)
{
  const char *envr = SDL_getenv("SDL_AUDIODRIVER");
  // Available if gNaclPPInstance is set and SDL_AUDIODRIVER is either unset, empty, or "nacl".
  if (gNaclPPInstance &&
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

	printf("NACLAUD_CreateDevice v1\n");
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

        // device->hidden->mu = SDL_CreateMutex();

        // SDL_LockMutex(_this->hidden->mu);

        _this->hidden->sample_frame_count =
            pp::AudioConfig::RecommendSampleFrameCount(PP_AUDIOSAMPLERATE_44100,
                kSampleFrameCount);
        _this->hidden->audio = pp::Audio(
            gNaclPPInstance,
            pp::AudioConfig(gNaclPPInstance,
                PP_AUDIOSAMPLERATE_44100,
                _this->hidden->sample_frame_count),
            AudioCallback, _this);

        printf("starting audio playback\n");
        _this->hidden->audio.StartPlayback();

	/* Set the function pointers */
	_this->OpenAudio = NACLAUD_OpenAudio;
	_this->CloseAudio = NACLAUD_CloseAudio;

	_this->free = NACLAUD_DeleteDevice;

	return _this;
}

AudioBootStrap NACLAUD_bootstrap = {
	NACLAUD_DRIVER_NAME, "SDL nacl audio driver",
	NACLAUD_Available, NACLAUD_CreateDevice
};


static void NACLAUD_CloseAudio(_THIS)
{
}


static void AudioCallback(void* samples, size_t buffer_size, void* data) {
  SDL_AudioDevice* _this = reinterpret_cast<SDL_AudioDevice*>(data);
  printf("============== audio callback\n");

  // TODO: lock!
  if (_this->spec.callback) {
    _this->spec.callback(_this->spec.userdata, (Uint8*)samples, buffer_size);
  } else {
    printf("silence...\n");
    SDL_memset(samples, 0, buffer_size);
  }
  printf("audio callback done\n");

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
