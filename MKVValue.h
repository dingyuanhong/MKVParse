#pragma once

#include <stdint.h>

#define CONTENT_TYPE 0x0
#define FLOAT_TYPE 0x1
#define INT_TYPE 0x2
#define STRING_TYPE 0x4
#define HEXSTRING_TYPE 0x8
#define BLOCK_TYPE 0x10

typedef uint64_t VINT;

typedef struct {
	VINT id;
	char * name;
	int type;
	int free;
}MKVValue;

static inline void freeMKVValue(MKVValue *value)
{
	if (value == NULL) return;
	if (value->free == 2) {
		delete value->name;
		delete value;
	}
	else if(value->free == 1) {
		free(value->name);
		free(value);
	}
}