#pragma once

#include <cstdint>
#include <cstdbool>
#include <array>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;

#define KiB                 1024U
#define MiB                 1048576U

#define MSB(n)              (((uint16_t)(n) >> 8) & 0x00ff)
#define LSB(n)              ((uint16_t)(n) & 0x00ff)
#define U16(lsb, msb)       (((uint16_t)(msb) << 8) | (uint16_t)(lsb))
#define NTHBIT(f, n)        (((f) >> (n)) & 0x0001)
#define SET(f, n)           ((f) |= (1U << (n)))
#define RES(f, n)           ((f) &= ~(1U << (n)))
#define IN_RANGE(x, a, b)   ((x) >= (a) && (x) <= (b))

#define STT_FAILED			0U
#define STT_SUCCESS			1U
