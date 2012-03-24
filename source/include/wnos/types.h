#ifndef _KERNEL_INCLUDE_TYPES_H
#define _KERNEL_INCLUDE_TYPES_H

#include <stdbool.h>

#define PRIVATE static
#define PUBLIC  extern

typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long uint64_t;
typedef long int64_t;
typedef unsigned long size_t;
typedef unsigned long uintptr_t;
typedef unsigned int spinlock_t;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

#endif