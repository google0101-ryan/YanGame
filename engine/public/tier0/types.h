#pragma once

#include <cstdint>

#define abstract_class class

#define DEFINE_CLASS(name, parent) \
private: \
    typedef name ThisClass; \
    typedef parent ParentClass; 

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef const char* str_t;

typedef float f32;
typedef double f64;