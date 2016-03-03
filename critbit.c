#include <inttypes.h>
#include <stdlib.h>

#include "critbit.h"

static uintptr_t *data_find(Critbit *, const void *, int, CritbitPos *);
static uintptr_t *node_find(Critbit *, const void *, int, int);
static int node_create(Critbit *, uintptr_t, uintptr_t *, int);
static void node_destroy(Critbit *, uintptr_t *, int);
static int node_direction(const void *, int, int);
static int data_critbit(uintptr_t, const uint8_t *, int);
static int msb_bit(uint8_t);
static uint8_t *key_get(int *, uintptr_t);
static int push(CritbitIt *, uintptr_t);
static uintptr_t pop(CritbitIt *);
static int expand(CritbitIt *);


void
critbit_init(Critbit *tree)
{
	tree->nums = 0;
	tree->root = 0;
}

void *
critbit_lookup(Critbit *tree, const void *key, int keybits, CritbitPos *pos)
{
	uintptr_t *n;
	int bit;

	if (critbit_isempty(tree))
		return NULL;

	n = data_find(tree, key, keybits, pos);

	bit = data_critbit(*n, key, keybits);

	if (pos != NULL)
		pos->bit = bit;

	if (bit != -1)
		return NULL;

	return (void *)*n;
}

int
critbit_insert(Critbit *tree, const void *tuple, const CritbitPos *pos)
{
	uintptr_t *n;
	uint8_t *key;
	int keybits;

	if (critbit_isempty(tree)) {
		tree->root = (uintptr_t)tuple;
		tree->nums++;
		return 0;
	}

	key = key_get(&keybits, (uintptr_t)tuple);

	n = node_find(tree, key, keybits, pos->bit);

	return node_create(tree, (uintptr_t)tuple, n, pos->bit);
}

void
critbit_remove(Critbit *tree, const CritbitPos *pos)
{
	if (pos->p == NULL) {
		tree->root = 0;
		tree->nums--;
		return;
	}

	node_destroy(tree, pos->p, pos->v);
}

int
critbit_it_init(CritbitIt *it, Critbit *tree, int max, int reverse)
{
	if (max == 0)
		max = 2;

	it->stack = malloc(sizeof(uintptr_t) * max);
	if (it->stack == NULL)
		return -1;

	it->v = reverse ? 1 : 0;
	it->max = max;
	it->cur = 0;
	push(it, tree->root);

	return 0;
}

void
critbit_it_destroy(CritbitIt *it)
{
	free(it->stack);
}

void *
critbit_next(CritbitIt *it)
{
	CritbitNode *node;
	uintptr_t n;

	n = pop(it);
	if (n == 0)
		return NULL;

	while (critbit_isinnode(n)) {
		node = critbit_toinnode(n);
		push(it, node->child[it->v ^ 1]);
		n = node->child[it->v];
	}

	return (void *)n;
}

static uintptr_t *
data_find(Critbit *tree, const void *key, int keybits, CritbitPos *pos)
{
	CritbitNode *node;
	uintptr_t *p;
	uintptr_t *n;
	int v;

	v = 0;
	p = NULL;
	n = &tree->root;
	while (critbit_isinnode(*n)) {
		p = n;
		node = critbit_toinnode(*n);
		v = node_direction(key, keybits, node->bit);
		n = &node->child[v];
	}

	if (pos != NULL) {
		pos->p = p;
		pos->v = v;
	}

	return n;
}

static uintptr_t *
node_find(Critbit *tree, const void *key, int keybits, int bit)
{
	CritbitNode *node;
	uintptr_t *n;
	int v;

	n = &tree->root;
	while (critbit_isinnode(*n)) {
		node = critbit_toinnode(*n);
		if (node->bit > bit)
			break;
		v = node_direction(key, keybits, node->bit);
		n = &node->child[v];
	}

	return n;
}

static int
node_create(Critbit *tree, uintptr_t n, uintptr_t *p, int bit)
{
	CritbitNode *node;
	uint8_t *key;
	int keybits;
	int v;

	node = malloc(sizeof(CritbitNode));
	if (node == NULL)
		return -1;

	key = key_get(&keybits, n);
	v = node_direction(key, keybits, bit);
	node->child[v] = n;
	node->child[v ^ 1] = *p;
	node->bit = bit;
	*p = (uintptr_t)node + 1;
	tree->nums++;

	return 0;
}

static void
node_destroy(Critbit *tree, uintptr_t *p, int v)
{
	CritbitNode *node;

	node = critbit_toinnode(*p);
	*p = node->child[v ^ 1];
	tree->nums--;

	free(node);
}

static int
node_direction(const void *key, int keybits, int bit)
{
	const uint8_t *k = key;
	int keylen = (keybits + 7) >> 3;
	int cbit = bit >> 1;
	int coff = cbit >> 3;

	if (coff < keylen)
		if (k[coff] & (0x80 >> (cbit & 0x7)))
			return 1;

	if (bit & 1)
		return 0;

	if (cbit < keybits)
		return 1;

	return 0;
}

static int
data_critbit(uintptr_t n, const uint8_t *key, int keybits)
{
	const uint8_t *nkey;
	int nkeybits;
	int nlen;
	int mlen;
	int len;
	int mod;
	int i;
	uint8_t d;

	nkey = key_get(&nkeybits, n);

	nlen = nkeybits >> 3;
	len = keybits >> 3;

	if (nlen < len) {
		mlen = nlen;
		mod = nkeybits & 7;
	} else {
		mlen = len;
		mod = keybits & 7;
	}

	for (i = 0; i < mlen; i++) {
		d = nkey[i] ^ key[i];
		if (d != 0) {
			return ((i << 3 | msb_bit(d)) << 1) | 1;
		}
	}

	if (mod > 0) {
		d = nkey[i] ^ key[i];
		d &= ~0 << (8 - mod);
		if (d != 0) {
			return ((i << 3 | msb_bit(d)) << 1) | 1;
		}
	}

	if (nkeybits == keybits)
		return -1;
	else if (nkeybits < keybits)
		return nkeybits << 1;
	else
		return keybits << 1;
}

static int
msb_bit(uint8_t x)
{
	int n;

	if (x == 0)
		return 0;

	n = 0;

	if ((x & 0xf0) == 0) {
		n += 4;
		x <<= 4;
	}
	if ((x & 0xc0) == 0) {
		n += 2;
		x <<= 2;
	}
	if ((x & 0x80) == 0)
		n++;

	return n;
}

static uint8_t *
key_get(int *keybits, uintptr_t n)
{
	int *v = (int *)n;

	*keybits = *v;

	return (uint8_t *)(v + 1);
}

static int
push(CritbitIt *it, uintptr_t n)
{
	if (n == 0)
		return 0;

	if (it->cur >= it->max)
		if (expand(it) != 0)
			return -1;

	it->stack[it->cur++] = n;
	return 0;
}

static uintptr_t
pop(CritbitIt *it)
{
	if (it->cur <= 0)
		return 0;

	return it->stack[--it->cur];
}

static int
expand(CritbitIt *it)
{
	int max;
	void *p;

	max = it->max * 2;

	p = realloc(it->stack, sizeof(uintptr_t) * max);
	if (p == NULL)
		return -1;
	it->stack = p;
	it->max = max;

	return 0;
}
