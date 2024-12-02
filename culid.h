#pragma once

#ifdef __SIZEOF_INT128__
#define CULID_UINT128
#endif

#ifdef CULID_UINT128
#include "culid_uint128.h"
#else
#include "culid_struct.hh"
#endif // ULID_UINT128
