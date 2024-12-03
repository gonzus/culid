#pragma once

#if defined(CULID_FORCE_UINT128) && defined(CULID_FORCE_STRUCT)
#error cannot define both CULID_FORCE_UINT128 and CULID_FORCE_STRUCT
#endif

#if defined(CULID_FORCE_UINT128)
#define CULID_USE_UINT128
#elif defined(CULID_FORCE_STRUCT)
#undef CULID_USE_UINT128
#elif defined(__SIZEOF_INT128__)
#define CULID_USE_UINT128
#endif

#if defined(CULID_USE_UINT128)
#include "culid_uint128.h"
#else
#include "culid_struct.h"
#endif
