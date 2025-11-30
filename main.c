#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>

#define RESERVED_ROPES		4000
#define RESERVED_STRINGS	4000

//Ropes are tagged unions
typedef struct Rope {
	enum Type { LEAF, BRANCH } type;
	size_t weight; //tree-balancing
	union {
		struct {
			size_t length; //length of the c-string - rounding up determines which bucket to use
			size_t entry; //the nth entry of the bucket
		} leaf;

		struct {
			struct Rope* left;
			struct Rope* right;
		} branch;
	} as;
} Rope;

//Knots are collections of ropes and strings
typedef struct Knot {
	size_t ropes_reserved;
	Rope* ropes_bucket;
	size_t ropes_probe;

	//each pair is for a different string length
	struct {
		size_t reserved;
		void* bucket;
		size_t probe;
	} string16, string32, string64, string128, string256, string512, string1024;
} Knot;

Knot make_knot() {
	Knot knot = {0};

	knot.ropes_reserved = RESERVED_ROPES * sizeof(Rope);
	knot.ropes_bucket = mmap(NULL, knot.ropes_reserved, PROT_READ, MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0);
	knot.ropes_probe = 0;

#define KNOT_STRING_MMAP(LENGTH) \
	knot.string##LENGTH.reserved = RESERVED_STRINGS * (sizeof(char) * LENGTH); \
	knot.string##LENGTH.bucket = mmap(NULL, knot.string##LENGTH.reserved, PROT_READ, MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0); \
	knot.ropes_probe = 0;

	KNOT_STRING_MMAP(16);
	KNOT_STRING_MMAP(32);
	KNOT_STRING_MMAP(64);
	KNOT_STRING_MMAP(128);
	KNOT_STRING_MMAP(256);
	KNOT_STRING_MMAP(512);
	KNOT_STRING_MMAP(1024);

#undef KNOT_STRING_MMAP

	return knot;
}

void unmake_knot(Knot* knot) {
	munmap(knot->ropes_bucket, knot->ropes_reserved);

#define KNOT_STRING_MUNMAP(LENGTH) munmap(knot->string##LENGTH.bucket, knot->string##LENGTH.reserved)

	KNOT_STRING_MUNMAP(16);
	KNOT_STRING_MUNMAP(32);
	KNOT_STRING_MUNMAP(64);
	KNOT_STRING_MUNMAP(128);
	KNOT_STRING_MUNMAP(256);
	KNOT_STRING_MUNMAP(512);
	KNOT_STRING_MUNMAP(1024);

#undef KNOT_STRING_MUNMAP
}

int main() {

#define printf_sz(X) printf("sizeof " #X ": %ld\n", sizeof(X))
#define printf_ld(X) printf(#X ": %ld\n", X)
#define printf_kb(X) printf(#X ": %ldkb\n", X/1024)

	printf_ld(RESERVED_ROPES);
	printf_ld(RESERVED_STRINGS);

	printf_sz(Rope);
	printf_sz(Knot);

	Knot knot = make_knot();

	printf_ld(knot.ropes_reserved);
	printf_ld(knot.string16.reserved);
	printf_ld(knot.string32.reserved);
	printf_ld(knot.string64.reserved);
	printf_ld(knot.string128.reserved);
	printf_ld(knot.string256.reserved);
	printf_ld(knot.string512.reserved);
	printf_ld(knot.string1024.reserved);

	printf_kb(knot.ropes_reserved);
	printf_kb(knot.string16.reserved);
	printf_kb(knot.string32.reserved);
	printf_kb(knot.string64.reserved);
	printf_kb(knot.string128.reserved);
	printf_kb(knot.string256.reserved);
	printf_kb(knot.string512.reserved);
	printf_kb(knot.string1024.reserved);

	//total space reserved
	size_t total_reserved = knot.ropes_reserved
		+ knot.string16.reserved
		+ knot.string32.reserved
		+ knot.string64.reserved
		+ knot.string128.reserved
		+ knot.string256.reserved
		+ knot.string512.reserved
		+ knot.string1024.reserved
	;

	printf_ld(total_reserved);
	printf_kb(total_reserved);

	unmake_knot(&knot);

	return 0;
}
