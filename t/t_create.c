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
	int mask;
	struct in_addr addr;
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
		char str[INET_ADDRSTRLEN];

		inet_ntop(AF_INET, &t->key.addr, str, sizeof(str));

		printf("%*s", depth * 2, "");
		printf("key: %s/%d\n", str, t->key.mask);
		printf("%*s", depth * 2, "");
		printf("val: %d\n", t->val);
	}
}

static void
test(void)
{
	const char *list[] = {
		"10.0.0.1",
		"10.0.0.2",
		"11.0.0.1"
	};
	const int n = sizeof(list) / sizeof(list[0]);
	struct in_addr addr;
	CritbitPos pos;
	Critbit tree;
	Tuple *t;
	int ret;
	int i;

	critbit_init(&tree);

	for (i = 0; i < n; i++) {
		inet_pton(AF_INET, list[i], &addr);
		t = critbit_lookup(&tree, &addr, 32, &pos);
		assert(t == NULL);

		t = malloc(sizeof(Tuple));
		assert(t != NULL);

		t->key.mask = 32;
		t->key.addr = addr;
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
