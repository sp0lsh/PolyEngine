#pragma once

#if defined(__STDC_LIB_EXT1__)
// Request presence of bound checked STL functions like strcat_s, strcpy_s, etc.
	#define __STDC_WANT_LIB_EXT1__ 1
#else
	// TODO implement all necessary *_s methods here.
#endif

// stupid warning in MSVC about template specialization exporting, according to https://msdn.microsoft.com/en-US/library/esew7y1w.aspx it can be ignored
#if defined(_WIN32)
	#pragma warning(disable: 4251)
	#pragma warning(disable: 4275)
	#define SILENCE_MSVC_WARNING(warning_id, reason) \
		__pragma(warning(push))                      \
		__pragma(warning(disable:warning_id))
	#define UNSILENCE_MSVC_WARNING() __pragma(warning(pop))
#else
	#define SILENCE_MSVC_WARNING(unused_warning_id, unused_reason)
	#define UNSILENCE_MSVC_WARNING()
#endif

#ifdef __GNUC__ //todo(vuko): add a reason parameter to the warning-silencing macro
	#define IMPL_SAVE_WARNING_SETTINGS _Pragma("GCC diagnostic push")
	#define IMPL_SILENCE_WARNING(w) _Pragma(#w)
	#define SILENCE_GCC_WARNING(w, reason)             \
		IMPL_SAVE_WARNING_SETTINGS                     \
		IMPL_SILENCE_WARNING(GCC diagnostic ignored #w)
	#define UNSILENCE_GCC_WARNING() _Pragma("GCC diagnostic pop")
#else
	#define SILENCE_GCC_WARNING(unused_w, unused_reason)
	#define UNSILENCE_GCC_WARNING()
#endif

SILENCE_MSVC_WARNING(4251, "Invalid warning for stl and other template classes.");
SILENCE_MSVC_WARNING(4530, "Invalid warning for xlocale.");
SILENCE_MSVC_WARNING(4577, "Invalid warning for xstring (noexcept).");

// STL
// This is only legal place for STL includes
#include <ostream>
#include <cmath>
#include <cassert>
#include <cstring>
#include <type_traits>
#include <limits>
#include <array>
#include <cstddef>
#include <memory>
#include <algorithm>
#include <iostream>
#include <string>
#include <functional>

// Custom type names
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

using uint = unsigned int;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ALIGN_16
#if defined(_WIN32)
	#define ALIGN_16 __declspec(align(16))
#elif defined(__GNUC__) || defined(__clang__)
	#define ALIGN_16 __attribute__ ((aligned(16)))
#else
	#define ALIGN_16 alignas(16)
#endif

#if defined(__GNUC__) && !defined(offsetof)
	#define offsetof(type, member) __builtin_offsetof(type, member)
#endif

// CORE_DLLEXPORT
#if defined(_WIN32)
	#if defined(_WINDLL) && defined(_CORE)
		#define CORE_DLLEXPORT __declspec(dllexport)
	#else
		#define CORE_DLLEXPORT __declspec(dllimport)
	#endif

	#if defined(_WINDLL) && defined(_ENGINE)
		#define ENGINE_DLLEXPORT __declspec(dllexport)
	#else
		#define ENGINE_DLLEXPORT __declspec(dllimport)
	#endif

	#if defined(_WINDLL) && defined(_GAME)
		#define GAME_DLLEXPORT __declspec(dllexport)
	#else
		#define GAME_DLLEXPORT __declspec(dllimport)
	#endif

	#if defined(_WINDLL) && defined(_DEVICE)
		#define DEVICE_DLLEXPORT __declspec(dllexport)
	#else
		#define DEVICE_DLLEXPORT __declspec(dllimport)
	#endif
#elif defined(__GNUC__) || defined(__clang__)
	//NOTE(vuko): dllexport and dllimport are the same as far as GCC and Clang are concerned
	#define CORE_DLLEXPORT __attribute__ ((visibility ("default")))
	#define ENGINE_DLLEXPORT __attribute__ ((visibility ("default")))
	#define GAME_DLLEXPORT __attribute__ ((visibility ("default")))
	#define DEVICE_DLLEXPORT __attribute__ ((visibility ("default")))
#else
	#define CORE_DLLEXPORT
	#define ENGINE_DLLEXPORT
	#define GAME_DLLEXPORT
	#define DEVICE_DLLEXPORT
#endif

#if defined(_WIN32)
#define POLY_STDCALL __stdcall
#else
#define POLY_STDCALL
#endif

#ifndef DISABLE_SIMD
#define DISABLE_SIMD false
#endif

// limits
constexpr auto MAX_FLOAT = (std::numeric_limits<float>::max)(); //the parentheses are there to prevent WinAPI macros from breaking this
constexpr auto MIN_FLOAT = (std::numeric_limits<float>::min)();

// Assertions
#define ASSERTE(expr, msg) assert((expr) && static_cast<const char*>(#msg)) //note(muniu): enabled in all builds except Release
#define HEAVY_ASSERTE(expr, msg) assert((expr) && static_cast<const char*>(#msg)) //todo(muniu): enabled only in Debug
#define STATIC_ASSERTE(expr, msg) static_assert(expr, msg)

// `Unreachable code` compiler intrinsic
#if defined(FINAL) && defined(_WIN32)
#define UNREACHABLE() __assume(0);
#elif defined(FINAL) && (defined(__GNUC__) || defined(__clang__))
#define UNREACHABLE() __builtin_unreachable();
#else
#define UNREACHABLE() do { assert("Unreachable code reached!" && false); throw; } while (false) //note(vuko): `throw` is needed to terminate the control path (assert is sadly not enough)
#endif

// Utilities
#define BIT(x) (1u<<x)

#define UNUSED(expr) do { (void)(expr); } while (false)

// Hack for clang compilation, should be used in every lambda in constexpr_match everywhere where T is required.
// required lambda argument to be "auto lazy"
#define LAZY_TYPE(T) decltype(lazy(std::declval<T>()))

template<bool B>
struct identity : std::integral_constant<bool, B> {
    template<typename T> typename std::conditional<B, T, void>::type operator()(T&& x) const { return std::forward<T>(x); }
};

//-----------------------------------------------------------------------------------------------------
template<typename T, typename F>
auto select(std::true_type, T&& t, F&&) { return t(identity<true>{}); }

template<typename T, typename F>
auto select(std::false_type, T&&, F&& f) { return f(identity<true>{}); }

//-----------------------------------------------------------------------------------------------------
template<typename C, typename T, typename F>
auto constexpr_if(T&& t, F&& f) { return select(C{}, std::forward<T>(t), std::forward<F>(f)); }

//-----------------------------------------------------------------------------------------------------
template<typename TL>
auto constexpr_match(TL&& tl) { return std::forward<TL>(tl); }

template<typename C1, typename T1, typename TL>
auto constexpr_match(C1, T1&& t1, TL&& tl) { return select(C1{}, std::forward<T1>(t1), std::forward<TL>(tl)); }

template<typename C1, typename T1, typename C2, typename T2, typename... Args>
auto constexpr_match(C1, T1&& t1, C2, T2&& t2, Args&&... tail)
{
	return select(C1{}, std::forward<T1>(t1), [&, tail ...](auto lazy) { return constexpr_match(C2{}, std::forward<LAZY_TYPE(T2)>(t2), tail...); });
}

//-----------------------------------------------------------------------------------------------------
#include "BaseObject.hpp"
