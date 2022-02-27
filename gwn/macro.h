#ifndef __gwn_MACRO_H__
#define __gwn_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

#define gwn_ASSERT(x) \
    if(!(x)){ \
        gwn_LOG_ERROR(gwn_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << gwn::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define gwn_ASSERT2(x, w) \
    if(!(x)) { \
        gwn_LOG_ERROR(gwn_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << gwn::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif