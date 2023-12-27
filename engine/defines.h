#pragma once

#define TRUE 1
#define FALSE 0

// Unsigned types
typedef unsigned char u8;
typedef	unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed types
typedef signed char i8;
typedef	signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef char b8;
typedef int b32;

#if defined(__clang__) || defined(__GNUC__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size.

/** @brief Assert u8 to be 1 byte.*/
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");

/** @brief Assert u16 to be 2 bytes.*/
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");

/** @brief Assert u32 to be 4 bytes.*/
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");

/** @brief Assert u64 to be 8 bytes.*/
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

/** @brief Assert i8 to be 1 byte.*/
STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");

/** @brief Assert i16 to be 2 bytes.*/
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");

/** @brief Assert i32 to be 4 bytes.*/
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");

/** @brief Assert i64 to be 8 bytes.*/
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

/** @brief Assert f32 to be 4 bytes.*/
STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");

/** @brief Assert f64 to be 8 bytes.*/
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

/** @brief Assert b8 to be 1 bytes.*/
STATIC_ASSERT(sizeof(b8) == 1, "Expected b8 to be 1 byte.");

/** @brief Assert b32 to be 4 bytes.*/
STATIC_ASSERT(sizeof(b32) == 4, "Expected f64 to be 4 bytes.");

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define PLATFORM_LINUX 1
#elif defined(__unix__)
// Catch anything not caught by the above.
#define PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define PLATFORM_POSIX 1
#else
#error "Unknown platform!"
#endif

// Inline functions
#if defined(__linux__) || defined(__gnu_linux__)
#define FINLINE static inline
#elif defined(_MSC_VER)
#define FINLINE __forceinline
#endif