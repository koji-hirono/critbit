#include <arpa/inet.h>
#include <netinet/in.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "critbit.h"

typedef struct Key Key;
typedef struct Tuple Tuple;

struct Key {
	int nbits;
	uint8_t oct[4];
};

struct Tuple {
	Key key;
	int val;
};

static void
tree_dump(uintptr_t n, int depth)
{
	if (n == 0) {
		printf("empty.\n");
		return;
	}

	if (critbit_isinnode(n)) {
		CritbitNode *node = critbit_toinnode(n);

		printf("%*s", depth * 2, "");
		printf("bit: %d\n", node->bit);

		tree_dump(node->child[0], depth + 1);
		tree_dump(node->child[1], depth + 1);
	} else {
		Tuple *t = critbit_toexnode(n);
		int i;

		printf("%*s", depth * 2, "");
		printf("key:");
		for (i = 0; i < 4; i++) {
			printf(" %02x", t->key.oct[i]);
		}
		printf("/%d\n", t->key.nbits);
		printf("%*s", depth * 2, "");
		printf("val: %d\n", t->val);
	}
}

static void
test(void)
{
	const Key list[] = {
		{32, {0xff, 0xff, 0xff, 0xff}},
		{32, {0xff, 0xff, 0xff, 0xfe}},
		{32, {0xff, 0xff, 0xff, 0xfc}},
		{32, {0xff, 0xff, 0xff, 0xf8}},
		{32, {0xff, 0xff, 0xff, 0xf0}},
		{32, {0xff, 0xff, 0xff, 0xe0}},
		{32, {0xff, 0xff, 0xff, 0xc0}},
		{32, {0xff, 0xff, 0xff, 0x80}},
		{32, {0xff, 0xff, 0xff, 0x00}},
		{32, {0xff, 0xff, 0xfe, 0x00}},
		{32, {0xff, 0xff, 0xfc, 0x00}},
		{32, {0xff, 0xff, 0xf8, 0x00}},
		{32, {0xff, 0xff, 0xf0, 0x00}},
		{32, {0xff, 0xff, 0xe0, 0x00}},
		{32, {0xff, 0xff, 0xc0, 0x00}},
		{32, {0xff, 0xff, 0x80, 0x00}},
		{32, {0xff, 0xff, 0x00, 0x00}},
		{32, {0xff, 0xfe, 0x00, 0x00}},
		{32, {0xff, 0xfc, 0x00, 0x00}},
		{32, {0xff, 0xf8, 0x00, 0x00}},
		{32, {0xff, 0xf0, 0x00, 0x00}},
		{32, {0xff, 0xe0, 0x00, 0x00}},
		{32, {0xff, 0xc0, 0x00, 0x00}},
		{32, {0xff, 0x80, 0x00, 0x00}},
		{32, {0xff, 0x00, 0x00, 0x00}},
		{31, {0xff, 0xff, 0xff, 0xfe}},
		{31, {0xff, 0xff, 0xff, 0xfc}},
		{31, {0xff, 0xff, 0xff, 0xf8}},
		{31, {0xff, 0xff, 0xff, 0xf0}},
		{31, {0xff, 0xff, 0xff, 0xe0}},
		{31, {0xff, 0xff, 0xff, 0xc0}},
		{31, {0xff, 0xff, 0xff, 0x80}},
		{31, {0xff, 0xff, 0xff, 0x00}},
		{31, {0xff, 0xff, 0xfe, 0x00}},
		{31, {0xff, 0xff, 0xfc, 0x00}},
		{31, {0xff, 0xff, 0xf8, 0x00}},
		{31, {0xff, 0xff, 0xf0, 0x00}},
		{31, {0xff, 0xff, 0xe0, 0x00}},
		{31, {0xff, 0xff, 0xc0, 0x00}},
		{31, {0xff, 0xff, 0x80, 0x00}},
		{31, {0xff, 0xff, 0x00, 0x00}},
		{31, {0xff, 0xfe, 0x00, 0x00}},
		{30, {0xff, 0xff, 0xff, 0xfc}},
		{29, {0xff, 0xff, 0xff, 0xf8}},
		{28, {0xff, 0xff, 0xff, 0xf0}},
		{27, {0xff, 0xff, 0xff, 0xe0}},
		{26, {0xff, 0xff, 0xff, 0xc0}},
		{25, {0xff, 0xff, 0xff, 0x80}},
		{24, {0xff, 0xff, 0xff, 0x00}},
		{23, {0xff, 0xff, 0xfe, 0x00}},
		{22, {0xff, 0xff, 0xfc, 0x00}},
		{21, {0xff, 0xff, 0xf8, 0x00}},
		{20, {0xff, 0xff, 0xf0, 0x00}}
	};
	const int n = sizeof(list) / sizeof(list[0]);
	CritbitPos pos;
	Critbit tree;
	Tuple *t;
	int ret;
	int i, j;

	critbit_init(&tree);

	for (i = 0; i < n; i++) {
		t = critbit_lookup(&tree, list[i].oct, list[i].nbits, &pos);
		assert(t == NULL);

		t = malloc(sizeof(Tuple));
		assert(t != NULL);

		t->key = list[i];
		t->val = i;
		ret = critbit_insert(&tree, t, &pos);
		assert(ret == 0);

		printf("===> Insert %d\n", i);
		printf("nums: %d\n", tree.nums);
		tree_dump(tree.root, 0);
	}

	for (i = 0; i < n; i++) {
		t = critbit_lookup(&tree, list[i].oct, list[i].nbits, NULL);
		assert(t != NULL);

		printf("get key:");
		for (j = 0; j < 4; j++) {
			printf(" %02x", t->key.oct[j]);
		}
		printf("/%d\n", t->key.nbits);
		printf("get val: %d\n", t->val);

		assert(memcmp(&t->key, &list[i], sizeof(t->key)) == 0);
	}
}

int
main(void)
{
	test();
	return 0;
}
