#pragma once

#include "lib/cpp/fluent/internal/fluent.h"
#include "lib/cpp/macro/va_args.h"

#define HANDBAG_FLUENT_MEMBER(...) \
    HANDBAG_MACRO_OVERLOAD_DISPATCHER_3(__VA_ARGS__, _HANDBAG_FLUENT_MEMBER_IMPL_3, _HANDBAG_FLUENT_MEMBER_IMPL_2, _HANDBAG_FLUENT_MEMBER_IMPL_1)(__VA_ARGS__)
