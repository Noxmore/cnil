#define CNIL_TEST

#include "nil/bserial.h"

typedef struct Foo {
	i32 bap;
	float bar;
	Vec(u16) baz;
} Foo;

BIN_SERIAL(Foo,
	i32, bap,
	float, bar,
	Vec$u16, baz
)

int main() {
	Foo foo = {
		.bap = 37,
		.bar = 6.36f,
		.baz = {0},
	};
	vecPush(&foo.baz, 77);
	vecPush(&foo.baz, 79);

	FILE* file = fopen("out.bin", "w");
	if (file == nullptr) {
		printf("Failed to open file\n");
		return 1;
	}

	// printf("Test");
	// Foo_binSerialize(file, &foo);
	binSerialize(Foo)(file, &foo);

	fclose(file);

	printf("Serialized\n");

	file = fopen("out.bin", "r");
	if (file == nullptr) {
		printf("Failed to open file for reading\n");
		return 1;
	}

	Foo foo2;
	// fread(&foo2, sizeof(Foo), 1, file);
	if (!Foo_binDeserialize(file, &foo2)) {
		printf("Failed to deserialize\n");
		return 1;
	}

	fclose(file);

	if (memcmp(&foo, &foo2, sizeof(Foo)) == 0) {
		printf("Sucess\n");
	} else {
		printf("Failed! Deserialized foo does not match\n");
	}
	vecDrop(&foo.baz);
	vecDrop(&foo2.baz);

	return 0;
}
