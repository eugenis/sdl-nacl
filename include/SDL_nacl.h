#ifndef _SDL_nacl_h
#define _SDL_nacl_h

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include <nacl/nacl_npapi.h>
#include <nacl/npapi_extensions.h>

void SDL_NACL_SetNPP(NPP npp);
void SDL_NACL_PushEvent(NPPepperEvent* nppevent);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_nacl_h */
