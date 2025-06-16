#include <stdio.h>

#define CNIL_TEST

#include "nil/vec.h"
#include "nil/bserial.h"

int main() {
	Foo foo = {
		.bap = 37,
		.bar = 6.36f,
		.baz = {0},
	};

	FILE* file = fopen("out.bin", "w");
	if (file == nullptr) {
		printf("Failed to open file");
		return 1;
	}

	// printf("Test");
	// Foo_binSerialize(file, &foo);
	binSerialize(Foo)(file, &foo);

	fclose(file);

	printf("Serialized");

	file = fopen("out.bin", "r");
	if (file == nullptr) {
		printf("Failed to open file for reading");
		return 1;
	}

	Foo foo2;
	if (!Foo_binDeserialize(file, &foo2)) {
		printf("Failed to deserialize");
		return 1;
	}

	fclose(file);

	if (memcmp(&foo, &foo2, sizeof(Foo)) == 0) {
		printf("Sucess");
	} else {
		printf("Failed! Deserialized foo does not match");
	}
}