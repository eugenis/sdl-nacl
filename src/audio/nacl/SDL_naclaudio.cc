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

// extern NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

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

/* Audio driver functions */
static int NACLAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void NACLAUD_WaitAudio(_THIS);
static void NACLAUD_PlayAudio(_THIS);
static Uint8 *NACLAUD_GetAudioBuf(_THIS);
static void NACLAUD_CloseAudio(_THIS);

/* Audio driver bootstrap functions */
static int NACLAUD_Available(void)
{




  // This code needs blocking push mode to work, which is currently (3 Oct 2010) not implemented in NaCl.
  // https://wiki.mozilla.org/Plugins:PepperAudioAPI#Model_Two:_Blocking_Push_Model
  const char *envr = SDL_getenv("SDL_AUDIODRIVER");
  // Available if NPP is set and SDL_AUDIODRIVER is either unset, empty, or "nacl".
  if (global_pp_instance &&
      (!envr || !*envr || SDL_strcmp(envr, NACLAUD_DRIVER_NAME) == 0)) {
    printf("nacl audio is available\n");
    return 1;
  }
  return 0;
}

static void NACLAUD_DeleteDevice(SDL_AudioDevice *device)
{
  // TODO: delete the device
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *NACLAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *_this;

	// printf("NACLAUD_CreateDevice\n");
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

	// _this->hidden->device = NPN_AcquireDevice(global_npp, NPPepperAudioDevice);
	// assert(_this->hidden->device);

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
  // SDL_LockMutex(call.mu);
  // printf("starting async call\n");
  // NPN_PluginThreadAsyncCall(global_npp, initialize_context, &call);
  // while (!call.done) {
  //   printf("== wait\n");
  //   SDL_CondWait(call.cv, call.mu);
  // }
  // printf("==done!\n");
  // SDL_UnlockMutex(call.mu);

  // SDL_Delay(1);
  // Nothing to do! PlayAudio is blocking until the device is ready for another buffer.
}

static void NACLAUD_PlayAudio(_THIS)
{
  assert(0);
  // NPDevice* device = _this->hidden->device;
  // NPDeviceContextAudio& context = _this->hidden->context;
  // if (!context.outBuffer) {
  //   printf("too early? let's wait a bit\n");
  //   while (!context.outBuffer) {
  //     printf("waiting...\n");
  //     SDL_Delay(2);
  //   }
  // }
  // printf("PlayAudio size %d\n", _this->hidden->mixlen);
  // assert(context.outBuffer);
  // memcpy(context.outBuffer, _this->hidden->mixbuf, _this->hidden->mixlen);
  // device->flushContext(global_npp, &context, NULL, NULL);
}

static Uint8 *NACLAUD_GetAudioBuf(_THIS)
{
	return(_this->hidden->mixbuf);
}

static void NACLAUD_CloseAudio(_THIS)
{
	if ( _this->hidden->mixbuf != NULL ) {
		SDL_FreeAudioMem(_this->hidden->mixbuf);
		_this->hidden->mixbuf = NULL;
	}
}

struct InitializeContextCall {
  SDL_AudioDevice* _this;
  // NPDeviceContextAudioConfig cfg;
  // NPError init_err;
  bool done;

  SDL_mutex* mu;
  SDL_cond* cv;
};

void initialize_context(void* data) {
  // InitializeContextCall& call = *(InitializeContextCall*)data;
  // NPDeviceContextAudioConfig& cfg = call.cfg;

  // SDL_LockMutex(call.mu);
  // // printf("== initializeContext\n");
  // call.init_err = call._this->hidden->device->initializeContext(
  //     global_npp, &cfg, &call._this->hidden->context);
  // // printf("== initializeContext done\n");
  // assert(call.init_err == NPERR_NO_ERROR);
  // call.done = true;
  // // printf("== signal\n");
  // SDL_CondSignal(call.cv);
  // // printf("== unlock\n");
  // SDL_UnlockMutex(call.mu);
}
#include <math.h>
static void AudioCallback(NPDeviceContextAudio *context) {
  printf("============== audio callback\n");
  fprintf(stderr, "============== audio callback z\n");
  if (!context)
    return;

  // static unsigned time_value_ = 0;
  // const size_t sample_count  = context->config.sampleFrameCount;
  // const size_t channel_count = context->config.outputChannelMap;
  // const double theta = 2 * 3.1415 * 10000 / context->config.sampleRate;
  // int16_t* buf = reinterpret_cast<int16_t*>(context->outBuffer);
  // for (size_t sample = 0; sample < sample_count; ++sample) {
  //   int16_t value = static_cast<int16_t>(sin(theta * time_value_) *
  // 					 0xFFFF);
  //   ++time_value_;  // Just let this wrap.
  //   for (size_t channel = 0; channel < channel_count; ++channel) {
  //     *buf++ = value;
  //   }
  // }
  return;

  // SDL_AudioDevice* audio = (SDL_AudioDevice*)context->config.userData;
  // Uint8 *stream;
  // int    stream_len;
  // void  *udata;
  // void (SDLCALL *fill)(void *userdata,Uint8 *stream, int len);
  // int    silence;

  // fill  = audio->spec.callback;
  // udata = audio->spec.userdata;
  

  // /* Fill the current buffer with sound */
  // if ( audio->convert.needed ) {
  //   if ( audio->convert.buf ) {
  //     stream = audio->convert.buf;
  //   } else {
  //     return;
  //   }
  // } else {
  //   stream = audio->GetAudioBuf(audio);
  //   if ( stream == NULL ) {
  //     stream = audio->fake_stream;
  //   }
  // }

  // SDL_memset(stream, silence, stream_len);

  // // printf("ready to fill\n");
  // if ( ! audio->paused ) {
  //   SDL_mutexP(audio->mixer_lock);
  //   (*fill)(udata, stream, stream_len);
  //   // printf("filled\n");
  //   SDL_mutexV(audio->mixer_lock);
  // }

  // /* Convert the audio if necessary */
  // if ( audio->convert.needed ) {
  //   SDL_ConvertAudio(&audio->convert);
  //   stream = audio->GetAudioBuf(audio);
  //   if ( stream == NULL ) {
  //     stream = audio->fake_stream;
  //   }
  //   SDL_memcpy(stream, audio->convert.buf,
  // 	       audio->convert.len_cvt);
  // }

  // // printf("sending data to the browser\n");
  // // Play the audio.
  // const size_t sample_count  = context->config.sampleFrameCount;
  // const size_t channel_count = context->config.outputChannelMap;
  // assert(sample_count * channel_count * 2 == stream_len);
  // int16_t* buf = reinterpret_cast<int16_t*>(context->outBuffer);
  // memcpy(buf, stream, stream_len);
}

static int NACLAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
  // printf("NACLAUD_OpenAudio\n");

  // InitializeContextCall call;
  // NPDeviceContextAudioConfig& cfg = call.cfg;
  // SDL_memset(&cfg, 0, sizeof(NPDeviceContextAudioConfig));
  // cfg.sampleRate = spec->freq;
 
  // Uint16 test_format = SDL_FirstAudioFormat(spec->format);
  // assert(test_format == AUDIO_S16LSB);

  // cfg.sampleType = NPAudioSampleTypeInt16;
  // cfg.outputChannelMap = spec->channels == 2 ? NPAudioChannelStereo : NPAudioChannelMono;
  // cfg.inputChannelMap = NPAudioChannelNone;
  // cfg.sampleFrameCount = spec->samples;
  // cfg.startThread = 1;  // Start a thread for the audio producer.
  // cfg.flags = 0;
  // cfg.callback = AudioCallback;
  // cfg.userData = reinterpret_cast<void*>(_this);

  // // printf("freq %d, samples %d\n", spec->freq, spec->samples);
 
  // call._this = _this;
  // call.done = false;
  // call.mu = SDL_CreateMutex();
  // call.cv = SDL_CreateCond();

  // SDL_LockMutex(call.mu);
  // // printf("starting async call\n");
  // NPN_PluginThreadAsyncCall(global_npp, initialize_context, &call);
  // while (!call.done) {
  //   // printf("== wait\n");
  //   SDL_CondWait(call.cv, call.mu);
  // }
  // // printf("==done!\n");
  // SDL_UnlockMutex(call.mu);

  // SDL_DestroyMutex(call.mu);
  // SDL_DestroyCond(call.cv);

  // assert(call.init_err == NPERR_NO_ERROR);

  // /* Allocate mixing buffer */
  // _this->hidden->mixlen = spec->size;
  // _this->hidden->mixbuf = (Uint8 *) SDL_AllocAudioMem(_this->hidden->mixlen);
  // if (_this->hidden->mixbuf == NULL) {
  //   return -1;
  // }
  // SDL_memset(_this->hidden->mixbuf, spec->silence, spec->size);

  // Do not create an audio thread.
  return 1;
}
} // extern "C"
