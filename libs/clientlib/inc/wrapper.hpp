#pragma once

#include "libpan_cxx_macros.hpp"

#define WRAPP_DECODE()\
    __attribute__((cdecl, regparm(0))) lang_decode_msg()
