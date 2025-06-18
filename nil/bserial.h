#pragma once

#include <endian.h>

#include "vec.h"

/*typedef enum BinSerializeTarget {
	BIN_SERIALIZER_BUFFER,
	BIN_SERIALIZER_MEMORY,
} BinSerializeTarget;

typedef struct BinSerializer {
	BinSerializeTarget target;
	union {
		Vec(u8) buf;
		FILE file;
	};
} BinSerializer;*/

/*static void binSerializeBytes(BinSerializer* s, const void* src, const usize num) {
	void* dst = internal_vecReserveItem((ErasedVec*)(&s->buf), num);
	memcpy(dst, src, num);
}*/

/*inline usize i8_binSerializeSize() {
	return 1;
}
inline void i8_binSerialize(BinSerializer* s, const i8 v) {
	vecPush(&s->buf, v);
}*/

/*inline BinSerializer binSerializerNew() {
	BinSerializer s = {0};

	i8_binSerialize(&s, BYTE_ORDER == BIG_ENDIAN);

	return s;
}*/

/*inline void binSerializerDrop(BinSerializer* s) {
	vecDrop(&s->buf);
}*/

typedef struct Foo {
	i32 bap;
	float bar;
	Vec(u16) baz;
} Foo;

/*typedef void (*SerializeFunction)(FILE*, const void*);
typedef bool (*DeserializeFunction)(FILE*, void*);

typedef struct SerialFunctions {

} SerialFunctions;*/

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
		T v = {0}; \
		DESERIALIZE \
		return true; \
	} \
	// BIN_SERIAL_DEFINE_CONTAINERS(T)
// TODO: defaults, error propagation, and size limiting

#define BIN_SERIAL_INTERNAL_GET_OVERRIDE(_1, _1b, _2, _2b, _3, _3b, pad, NAME, ...) NAME

#define BIN_SERIAL(T, ...) BIN_SERIAL_INTERNAL_GET_OVERRIDE(__VA_ARGS__, BIN_SERIAL_3, invalid3, BIN_SERIAL2, invalid2, BIN_SERIAL1, invalid1)(T, __VA_ARGS__)

#define BIN_SERIAL1(T, T0, F0) BIN_SERIAL_INTERNAL(T, \
	binSerialize(T0)(dst, &v->F0);,\
	if (!binDeserialize(T0)(src, &v.F0)) return false;\
)

#define BIN_SERIAL2(T, T0, F0, T1, F1) BIN_SERIAL_INTERNAL(T, \
	{ \
		binSerialize(T0)(dst, &v->F0); \
		binSerialize(T1)(dst, &v->F1); \
	}, \
	{ \
		if (!binDeserialize(T0)(src, &v.F0)) return false; \
		if (!binDeserialize(T1)(src, &v.F1)) return false; \
	} \
)


// #define BIN_SERIAL_FIELD(T, FIELD) (T)(&v->FIELD)

#define BIN_SERIAL_DEFINE_CONTAINERS(T) \
	static void binSerialize(Vec$##T)(FILE* dst, const Vec(T)* v) { for (usize i = 0; i < v->len; i++) binSerialize(T)(dst, &v->data[i]); } \
	static inline void binSerialize(Box$##T)(FILE* dst, const T** v) { binSerialize(T)(dst, *v); } \
	static bool binDeserialize(Vec$##T)(FILE* src, Vec(T)* dst) { \
		if (!binDeserialize(usize)(src, &dst->len)) return false; \
		dst->cap = dst->len; \
		dst->data = nullptr; \
		vecReserve(dst, dst->len); \
		for (usize i = 0; i < dst->len; i++) \
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

// BIN_SERIAL_STABLE_SIMPLE(Foo)

BIN_SERIAL(Foo,
	i32, bap,
	float, bar,
	// Vec$u16, baz,
)

void fooTest(FILE* dst, const Foo* v) {
	// _GET_OVERRIDE(a, b, o3, o2, o0);
	u16 size = sizeof(Foo);
	fwrite(&size, sizeof(u16), 1, dst);

	binSerialize(i32)(dst, &v->bap);
	binSerialize(float)(dst, &v->bar);
	binSerialize(Vec$u16)(dst, &v->baz);
}