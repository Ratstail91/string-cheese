#include <assert.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define RESERVED_ROPES		8000
#define RESERVED_STRINGS	4000

//Ropes are tagged unions that point to c-strings or other ropes
typedef struct Rope {
	enum Type { LEAF, BRANCH } type;
	size_t weight; //tree-balancing
	union {
		struct {
			size_t power; //derived from the string length
			const char* c_str;
		} leaf;

		struct {
			struct Rope* left;
			struct Rope* right;
		} branch;
	} as;
} Rope;

//Buckets hold space in memory for c-strings
typedef struct StringBucket {
	size_t reserved;
	char* bucket;
	size_t probe;
} StringBucket;

char* find_empty_string(StringBucket* stringBucket, size_t power) {
	//find the next free space for the string, wrapping at the end
	size_t probe;
	do {
		if (stringBucket->probe >= RESERVED_STRINGS) stringBucket->probe = 0;
		probe = stringBucket->probe++;
	} while(stringBucket->bucket[probe * power] != '\0'); //BUG: might be an arena overflow here

	return &(stringBucket->bucket[probe * power]);
}

//Knots are collections of ropes and strings (using StringBucket)
typedef struct Knot {
	size_t ropes_reserved;
	Rope* ropes_bucket;
	size_t ropes_probe;

	//each bucket is for a different string length
	StringBucket string16, string32, string64, string128, string256, string512, string1024;
} Knot;

Knot make_knot() {
	Knot knot = {0};

	//allocates the rope memory
	knot.ropes_reserved = RESERVED_ROPES * sizeof(Rope);
	knot.ropes_bucket = mmap(NULL, knot.ropes_reserved, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	knot.ropes_probe = 0;

	//allocates the string memory (macros avoid typos in this case)
	#define KNOT_STRING_MMAP(LENGTH) \
		knot.string##LENGTH.reserved = RESERVED_STRINGS * (sizeof(char) * LENGTH); \
		knot.string##LENGTH.bucket = mmap(NULL, knot.string##LENGTH.reserved, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); \
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

//utils
size_t next_power_of_2(size_t sz) {
	sz--;
	sz |= sz >> 1;
	sz |= sz >> 2;
	sz |= sz >> 4;
	sz |= sz >> 8;
	sz |= sz >> 16;
	sz |= sz >> 32;
	sz++;
	return sz;
}

//usable functions
Rope* allocate_rope(Knot* knot, const char* c_str) {
	size_t length = strlen(c_str);
	size_t power = next_power_of_2(length);

	//find the next free space for the rope, wrapping at the end
	size_t rope_probe;
	do {
		if (knot->ropes_probe >= RESERVED_ROPES) knot->ropes_probe = 0;
		rope_probe = knot->ropes_probe++;
	} while(knot->ropes_bucket[rope_probe].weight != 0); //BUG: might be an arena overflow here

	//find where to store the c_str
	char* dest_str = NULL;
	if (power <= 16) {
		dest_str = find_empty_string(&knot->string16, power);
	}
	else if (power <= 32) {
		dest_str = find_empty_string(&knot->string32, power);
	}
	else if (power <= 64) {
		dest_str = find_empty_string(&knot->string64, power);
	}
	else if (power <= 128) {
		dest_str = find_empty_string(&knot->string128, power);
	}
	else if (power <= 256) {
		dest_str = find_empty_string(&knot->string256, power);
	}
	else if (power <= 512) {
		dest_str = find_empty_string(&knot->string512, power);
	}
	else if (power <= 1024) {
		dest_str = find_empty_string(&knot->string1024, power);
	}

	//store the c-string and return the new rope
	strncpy(dest_str, c_str, length);

	Rope* rope = &(knot->ropes_bucket[rope_probe]);
	*rope = (Rope){LEAF,length,{ .leaf = {power, dest_str} }};

	return rope;
}

Rope* concat_rope(Knot* knot, Rope* left, Rope* right) {
	//find the next free space for the rope, wrapping at the end
	size_t rope_probe;
	do {
		if (knot->ropes_probe >= RESERVED_ROPES) knot->ropes_probe = 0;
		rope_probe = knot->ropes_probe++;
	} while(knot->ropes_bucket[rope_probe].weight != 0); //BUG: might be an arena overflow here

	Rope* rope = &(knot->ropes_bucket[rope_probe]);
	*rope = (Rope){BRANCH,left->weight + right->weight,{ .branch = {left,right} }};

	return rope;
}

//for testing
void print_rope(Rope* rope, int prefix_width) {
	assert(rope);

	if (rope->type == LEAF) {
		printf("%*s%s\n",
			prefix_width, //left-pad
			prefix_width > 0 ? "+-" : "", //left-pad contents
			rope->as.leaf.c_str
		);
	}
	else if (rope->type == BRANCH) {
		print_rope(rope->as.branch.left, prefix_width + 2);
		print_rope(rope->as.branch.right, prefix_width + 2);
	}
	else {
		assert(0);
	}
}

int test_sizes() {
	#define printf_sz(X) printf("sizeof " #X ": %ld\n", sizeof(X))
	#define printf_ld(X) printf(#X ": %ld\n", X)
	#define printf_kb(X) printf(#X ": %ldkb\n", X/1024)

	#define printf_int(X) printf(#X ": %d\n", X)
	printf_int(RESERVED_ROPES);
	printf_int(RESERVED_STRINGS);

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

	#undef printf_sz
	#undef printf_ld
	#undef printf_kb
	#undef printf_int

	return 0;
}

int test_ropes() {
	Knot knot = make_knot();

	Rope* a = allocate_rope(&knot, "Alfa");
	Rope* b = allocate_rope(&knot, "Bravo");
	Rope* c = allocate_rope(&knot, "Charlie");
	Rope* d = allocate_rope(&knot, "Delta");
	Rope* e = allocate_rope(&knot, "Echo");
	Rope* f = allocate_rope(&knot, "Foxtrot");

	Rope* ab = concat_rope(&knot, a, b);
	Rope* abc = concat_rope(&knot, ab, c);
	Rope* abcd = concat_rope(&knot, abc, d);
	Rope* abcde = concat_rope(&knot, abcd, e);
	Rope* abcdef = concat_rope(&knot, abcde, f);

	print_rope(abcdef, 0);

	unmake_knot(&knot);
	return 0;
}

int main() {
	return test_ropes();
}