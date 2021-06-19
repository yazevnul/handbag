#pragma once

// TODO(kostya): codegen this header

/// Macros to help overload other macroses by number of arguments.
/// @{
#define HANDBAG_MACRO_OVERLOAD_DISPATCHER_2(_0, _1, impl, ...) impl
#define HANDBAG_MACRO_OVERLOAD_DISPATCHER_3(_0, _1, _2, impl, ...) impl
#define HANDBAG_MACRO_OVERLOAD_DISPATCHER_4(_0, _1, _2, _3, impl, ...) impl
/// }@
