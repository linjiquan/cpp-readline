#pragma once

#include "xma_status.h"

namespace xma {

#define _XMA_DEBUG_

#ifdef _XMA_DEBUG_
#define XMA_DEBUG(fmt, ...) \
  do { \
    printf ("[DEBUG %s@%s:%d]", __FUNCTION__, __FILE__, __LINE__); \
    printf (fmt, __VA_ARGS__); \
    printf ("\n"); \
  } while (0)
#else
#define XMA_DEBUG(fmt, ...)
#endif

}
