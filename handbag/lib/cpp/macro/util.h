#pragma once

#define HANDBAG_PASS_VA_ARGS(x) x

#define HANDBAG_CAT(x, y) _HANDBAG_CAT_IMPL_I(x, y)
#define _HANDBAG_CAT_IMPL_I(x, y) _HANDBAG_CAT_IMPL_II(x, y)
#define _HANDBAG_CAT_IMPL_II(x, y) x##y

#define HANDBAG_STRINGIZE(x) _HANDBAG_STRINGIZE_IMPL(x)
#define _HANDBAG_STRINGIZE_IMPL(x) #x
