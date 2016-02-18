#ifndef CRITBIT_H__
#define CRITBIT_H__

#include <inttypes.h>

typedef struct CritbitPos CritbitPos;
typedef struct CritbitNode CritbitNode;
typedef struct CritbitIt CritbitIt;
typedef struct Critbit Critbit;

struct CritbitPos {
	int bit;
	int v;
	uintptr_t *p;
};

struct CritbitNode {
	int bit;
	uintptr_t child[2];
};

struct CritbitIt {
	int v;
	int cur;
	int max;
	uintptr_t *stack;
};

struct Critbit {
	int nums;
	uintptr_t root;
};

#define critbit_isempty(t) ((t)->root == 0)
#define critbit_isinnode(n) ((n) & 1)
#define critbit_isexnode(n) (((n) & 1) == 0)
#define critbit_toinnode(n) (CritbitNode *)((n) - 1)
#define critbit_toexnode(n) (void *)(n)

extern void critbit_init(Critbit *);
extern void *critbit_lookup(Critbit *, const void *, int, CritbitPos *);
extern int critbit_insert(Critbit *, const void *, const CritbitPos *);
extern void critbit_remove(Critbit *, const CritbitPos *);

extern int critbit_it_init(CritbitIt *, Critbit *, int, int);
extern void critbit_it_destroy(CritbitIt *);
extern void *critbit_next(CritbitIt *);

#endif /* !CRITBIT_H__ */
