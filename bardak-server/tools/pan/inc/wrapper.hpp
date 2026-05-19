#pragma once

#include "libpan_cxx_macros.hpp"

#define GLOBAL_VAR_NAME(loc, ipref, itype) global_##PAN_GH_NAME(loc, ipref, itype)

#define GLOBAL_VAR_MSG GLOBAL_VAR_NAME(msg, , )

#define WRAPPER_NAME(loc, ipref, itype, field) get_##PAN_GH_NAME(loc, ipref, itype)##_##field

#define WRAPPER_I(loc, ipref, itype, field)\
    __attribute__((cdecl, regparm(0))) inline int\
    WRAPPER_NAME(loc, ipref, itype, field)() {\
        return GLOBAL_VAR_NAME(loc, ipref, itype)##.##field;\
    }

#define WRAPPER_DECODE_NAME(loc, ipref, itype) decode_##PAN_GH_NAME(loc, ipref, itype)

#define WRAPPER_DECODE_I(loc, ipref, itype)\
    __attribute__((cdecl, regparm(0))) inline int\
    WRAPPER_DECODE_NAME(loc, ipref, itype)() {\
        GLOBAL_VAR_NAME(loc, ipref, itype)##::decode(GLOBAL_VAR_MSG);\
        return 0;\
    }
