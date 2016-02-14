#include <arpa/inet.h>
#include <netinet/in.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
		{32, {0x0a, 0x00, 0x00, 0x01}},
		{32, {0x0a, 0x00, 0x00, 0x02}},
		{32, {0x0a, 0x00, 0x00, 0x03}},
		{32, {0x0a, 0x00, 0x00, 0x04}}
	};
	const int n = sizeof(list) / sizeof(list[0]);
	CritbitPos pos;
	Critbit tree;
	Tuple *t;
	int ret;
	int i;

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
}

int
main(void)
{
	test();
	return 0;
}
