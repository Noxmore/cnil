#pragma once

#include <endian.h>
#include <stdio.h>

#include "vec.h"

// We need controls to serialize
// - Sequence
// - Object
// - Map

typedef struct Serializer {
	
} Serializer;

#define binSerialize(T) T##_binSerialize
#define binDeserialize(T) T##_binDeserialize

#define BIN_SERIAL_INTERNAL(T, SERIALIZE, DESERIALIZE) \
	static void binSerialize(T)(FILE* dst, const T* v) { \
		const u16 size = sizeof(T); \
		binSerialize(u16)(dst, &size); \
		SERIALIZE \
	} \
	static bool binDeserialize(T)(FILE* src, T* dst) { \
		u16 size; \
		if (!binDeserialize(u16)(src, &size)) return false; \
		DESERIALIZE \
		return true; \
	} \
	BIN_SERIAL_DEFINE_CONTAINERS(T)
// TODO: defaults, error propagation, and size limiting

#define BIN_SERIAL_INTERNAL_GET_OVERRIDE(_1, _1b, _2, _2b, _3, _3b, _4, _4b, _5, _5b, _6, _6b, pad, NAME, ...) NAME
#define BIN_SERIAL_INT_SE(T, F) binSerialize(T)(dst, &v->F);
#define BIN_SERIAL_INT_DE(T, F) if (!binDeserialize(T)(src, &dst->F)) return false;

#define BIN_SERIAL(T, ...) BIN_SERIAL_INTERNAL_GET_OVERRIDE(__VA_ARGS__, invalid6, BIN_SERIAL6, invalid5, BIN_SERIAL5, invalid4, BIN_SERIAL4, invalid3, BIN_SERIAL3, invalid2, BIN_SERIAL2, invalid1, BIN_SERIAL1)(T, __VA_ARGS__)

#define BIN_SERIAL1(T, T0, F0) BIN_SERIAL_INTERNAL(T, \
	binSerialize(T0)(dst, &v->F0);, \
	if (!binDeserialize(T0)(src, &v.F0)) return false; \
)

#define BIN_SERIAL2(T, T1, F1, T2, F2) BIN_SERIAL_INTERNAL(T, \
	{ BIN_SERIAL_INT_SE(T1,F1) BIN_SERIAL_INT_SE(T2,F2) }, \
	{ BIN_SERIAL_INT_DE(T1,F1) BIN_SERIAL_INT_DE(T2,F2) } \
)
#define BIN_SERIAL3(T, T1, F1, T2, F2, T3, F3) BIN_SERIAL_INTERNAL(T, \
	{ BIN_SERIAL_INT_SE(T1,F1) BIN_SERIAL_INT_SE(T2,F2) BIN_SERIAL_INT_SE(T3,F3) }, \
	{ BIN_SERIAL_INT_DE(T1,F1) BIN_SERIAL_INT_DE(T2,F2) BIN_SERIAL_INT_DE(T3,F3) } \
)
#define BIN_SERIAL4(T, T1, F1, T2, F2, T3, F3, T4, F4) BIN_SERIAL_INTERNAL(T, \
	{ BIN_SERIAL_INT_SE(T1,F1) BIN_SERIAL_INT_SE(T2,F2) BIN_SERIAL_INT_SE(T3,F3) BIN_SERIAL_INT_SE(T4,F4) }, \
	{ BIN_SERIAL_INT_DE(T1,F1) BIN_SERIAL_INT_DE(T2,F2) BIN_SERIAL_INT_DE(T3,F3) BIN_SERIAL_INT_DE(T4,F4) } \
)
#define BIN_SERIAL5(T, T1, F1, T2, F2, T3, F3, T4, F4, T5, F5) BIN_SERIAL_INTERNAL(T, \
	{ BIN_SERIAL_INT_SE(T1,F1) BIN_SERIAL_INT_SE(T2,F2) BIN_SERIAL_INT_SE(T3,F3) BIN_SERIAL_INT_SE(T4,F4) BIN_SERIAL_INT_SE(T5,F5) }, \
	{ BIN_SERIAL_INT_DE(T1,F1) BIN_SERIAL_INT_DE(T2,F2) BIN_SERIAL_INT_DE(T3,F3) BIN_SERIAL_INT_DE(T4,F4) BIN_SERIAL_INT_DE(T5,F5) } \
)
#define BIN_SERIAL6(T, T1, F1, T2, F2, T3, F3, T4, F4, T5, F5, T6, F6) BIN_SERIAL_INTERNAL(T, \
	{ BIN_SERIAL_INT_SE(T1,F1) BIN_SERIAL_INT_SE(T2,F2) BIN_SERIAL_INT_SE(T3,F3) BIN_SERIAL_INT_SE(T4,F4) BIN_SERIAL_INT_SE(T5,F5) BIN_SERIAL_INT_SE(T6,F6) }, \
	{ BIN_SERIAL_INT_DE(T1,F1) BIN_SERIAL_INT_DE(T2,F2) BIN_SERIAL_INT_DE(T3,F3) BIN_SERIAL_INT_DE(T4,F4) BIN_SERIAL_INT_DE(T5,F5) BIN_SERIAL_INT_DE(T6,F6) } \
)


#define BIN_SERIAL_DEFINE_CONTAINERS(T) \
	static void binSerialize(Vec$##T)(FILE* dst, const Vec(T)* v) { binSerialize(usize)(dst, &v->len); for (usize i = 0; i < v->len; i++) binSerialize(T)(dst, &v->data[i]); } \
	static inline void binSerialize(Box$##T)(FILE* dst, const T** v) { binSerialize(T)(dst, *v); } \
	static bool binDeserialize(Vec$##T)(FILE* src, Vec(T)* dst) { \
		usize count; \
		if (!binDeserialize(usize)(src, &count)) return false; \
		*dst = (struct Vec$##T){0}; \
		vecReserve(dst, count); \
		for (usize i = 0; i < count; i++) \
			if (!binDeserialize(T)(src, vecReserveItem(dst))) return false; \
		return true; \
	} \
	static inline bool binDeserialize(Box$##T)(FILE* src, T** dst) { return binDeserialize(T)(src, *dst); }

#define BIN_SERIAL_STABLE_SIMPLE(T) \
	static inline void T##_binSerialize(FILE* dst, const T* v) { fwrite(v, sizeof(T), 1, dst); } \
	static inline bool T##_binDeserialize(FILE* src, T* dst) { return fread(dst, sizeof(T), 1, src) < sizeof(T); } \
	BIN_SERIAL_DEFINE_CONTAINERS(T)


BIN_SERIAL_STABLE_SIMPLE(usize)

BIN_SERIAL_STABLE_SIMPLE(i8)
BIN_SERIAL_STABLE_SIMPLE(i16)
BIN_SERIAL_STABLE_SIMPLE(i32)
BIN_SERIAL_STABLE_SIMPLE(i64)

BIN_SERIAL_STABLE_SIMPLE(u8)
BIN_SERIAL_STABLE_SIMPLE(u16)
BIN_SERIAL_STABLE_SIMPLE(u32)
BIN_SERIAL_STABLE_SIMPLE(u64)

BIN_SERIAL_STABLE_SIMPLE(float)
BIN_SERIAL_STABLE_SIMPLE(double)

BIN_SERIAL_STABLE_SIMPLE(char)
