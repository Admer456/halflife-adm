//
// Word size dependent definitions
// DAL 1/03
//

#pragma once

#include "steam/steamtypes.h"

#ifndef _WIN32
#define MAX_PATH PATH_MAX
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stddef.h>
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
typedef long unsigned int ulong;
#endif
