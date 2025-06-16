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


#define binSerialize(T) T##_binSerialize
#define binDeserialize(T) T##_binDeserialize

#define BIN_SERIAL

#define BIN_SERIAL_STABLE_SIMPLE(T) \
	static inline void T##_binSerialize(FILE* dst, const T* v) { fwrite(v, sizeof(T), 1, dst); } \
	static inline bool T##_binDeserialize(FILE* src, T* dst) { return fread(&dst, sizeof(T), 1, src) < sizeof(T); }

BIN_SERIAL_STABLE_SIMPLE(i8)
BIN_SERIAL_STABLE_SIMPLE(i16)
BIN_SERIAL_STABLE_SIMPLE(i32)
BIN_SERIAL_STABLE_SIMPLE(i64)

BIN_SERIAL_STABLE_SIMPLE(u8)
BIN_SERIAL_STABLE_SIMPLE(u16)
BIN_SERIAL_STABLE_SIMPLE(u32)
BIN_SERIAL_STABLE_SIMPLE(u64)

BIN_SERIAL_STABLE_SIMPLE(usize)

BIN_SERIAL_STABLE_SIMPLE(float)
BIN_SERIAL_STABLE_SIMPLE(double)

BIN_SERIAL_STABLE_SIMPLE(Foo)