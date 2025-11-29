#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void* (*rope_alloc_t)(size_t);
typedef void (*rope_free_t)(void*);

//Ropes are tagged unions
typedef struct Rope {
	enum Type { LEAF, BRANCH } type;
	union {
		const char* const c_str;
		struct {
			struct Rope* left;
			struct Rope* right;
		} branch;
	} as;
} Rope;

Rope alloc_rope(rope_alloc_t alloc, const char* str) {
	assert(str);

	size_t len = strlen(str);
	void* ptr = alloc(len + 1);
	memcpy(ptr, str, len + 1);

	return (Rope){LEAF,{ .c_str = ptr }};
}

void free_rope(rope_free_t free, Rope* rope) {
	if (rope->type == LEAF) {
		free((void*)(rope->as.c_str));
	}
	else if (rope->type == BRANCH) {
		free_rope(free, rope->as.branch.left);
		free_rope(free, rope->as.branch.right);
	}
	else {
		assert(0);
	}
}

Rope concat_rope(Rope* left, Rope* right) {
	return (Rope){BRANCH,{ .branch = {left,right} }};
}

void dbg_print_rope(Rope* rope, int prefix_width) {
	assert(rope);

	if (rope->type == LEAF) {
		printf("%*s%s\n",
			prefix_width, //left-pad
			prefix_width > 0 ? "+-" : "", //left-pad contents
			rope->as.c_str
		);
	}
	else if (rope->type == BRANCH) {
		dbg_print_rope(rope->as.branch.left, prefix_width + 2);
		dbg_print_rope(rope->as.branch.right, prefix_width + 2);
	}
	else {
		assert(0);
	}
}

int main() {
	// Rope rope = alloc_rope(malloc, "Hello world!");
	// dbg_print_rope(&rope, 0);
	// free_rope(free, &rope);

	Rope a = alloc_rope(malloc, "Alfa");
	Rope b = alloc_rope(malloc, "Bravo");
	Rope c = alloc_rope(malloc, "Charlie");
	Rope d = alloc_rope(malloc, "Delta");
	Rope e = alloc_rope(malloc, "Echo");
	Rope f = alloc_rope(malloc, "Foxtrot");

	Rope ab = concat_rope(&a, &b);
	Rope abc = concat_rope(&ab, &c);
	Rope abcd = concat_rope(&abc, &d);
	Rope abcde = concat_rope(&abcd, &e);
	Rope abcdef = concat_rope(&abcde, &f);

	dbg_print_rope(&abcdef, 0);

	return 0;
}


//split
//concat

//append - concat(a, b)
//insert - split, concat, concat
//delete - split, split, concat


//auto-balancing - children should be balanced first
//rotate left
//rotate right