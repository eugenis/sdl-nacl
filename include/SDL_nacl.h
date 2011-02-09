#ifndef _SDL_nacl_h
#define _SDL_nacl_h

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// #include <ppapi/cpp/instance.h>
// #include <nacl/nacl_npapi.h>
// #include <nacl/npapi_extensions.h>
#include <ppapi/c/pp_input_event.h>

void SDL_NACL_SetInstance(void* instance);
void SDL_NACL_PushEvent(const PP_InputEvent* ppevent);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_nacl_h */
