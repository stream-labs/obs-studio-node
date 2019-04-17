#pragma once

#include <stdlib.h>  /*a more portable std::size_t definition than stddef.h itself*/
#ifdef __cplusplus
extern "C" {
#endif
void*  ltmalloc(size_t);
void   ltfree(void*);
void   ltfreeclear(void*);
void*  ltrealloc(void*, size_t);
void*  ltcalloc(size_t, size_t);
void*  ltmemalign(size_t, size_t);
void   ltsqueeze(size_t); /*return memory to system (see README.md)*/
std::size_t ltmsize(void*);
void   ltonthreadexit();
#ifdef __cplusplus
}
#endif
