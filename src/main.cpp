#include <stdio.h>
#include <assert.h>

struct Operation {
#define STRINGABLE_ENUM(C) C(Add) C(Sub) C(Mul) C(Div) 
	enum {
#define INSTANCE(inst) inst,
		STRINGABLE_ENUM(INSTANCE)
#undef INSTANCE
	} type;

	static const char* enum_get_string(int inst) {
		switch (inst) {
#define INSTANCE(inst) case inst: return #inst;
			STRINGABLE_ENUM(INSTANCE)
#undef INSTANCE
		}
		return 0;
	}
#undef STRINGABLE_ENUM

	struct Term* term1;
	struct Term* term2;
};

struct Term {
	enum { Number, Operation } type;
	union {
		double num;
		struct Operation op;
	};
};

const auto term_pool_cap = 1024;
Term term_pool[term_pool_cap];
int term_pool_count = 0;

#define ARR_COUNT(x) (sizeof(x)/sizeof((x)[0]))

void print_term(const Term& term, const char* const parent_node, const int term_num)
{
	const auto index = (int)(&term - term_pool);
	char node[256];

	if (term.type == Term::Operation)
	{
		snprintf(node, ARR_COUNT(node), "op_%d_%d", term_num, index);
		printf("\t%s [label=%s]\n", node, Operation::enum_get_string(term.op.type));
	}
	else
	{
		snprintf(node, ARR_COUNT(node), "num_%d_%d", term_num, index);
		printf("\t%s [label=%f]\n", node, term.num);
	}

	if (parent_node)
		printf("\t%s -> %s\n", parent_node, node);

	if (term.type == Term::Operation)
	{
		print_term(*term.op.term1, node, 1);
		print_term(*term.op.term2, node, 2);
	}
}

Term* alloc_term()
{
	assert(term_pool_count < term_pool_cap);
	return &term_pool[term_pool_count++];
}

int main()
{
	auto term = alloc_term();
	term->type = Term::Operation;
	term->op.type = Operation::Mul;
	term->op.term1 = alloc_term();
	term->op.term1->type = Term::Operation;
	term->op.term1->op.type = Operation::Add;
	term->op.term1->op.term1 = alloc_term();
	term->op.term1->op.term1->type = Term::Number;
	term->op.term1->op.term1->num = 5;
	term->op.term1->op.term2 = alloc_term();
	term->op.term1->op.term2->type = Term::Number;
	term->op.term1->op.term2->num = 6;

	term->op.term2 = alloc_term();
	term->op.term2->type = Term::Number;
	term->op.term2->num = 2;

	printf("digraph {\n");
	print_term(*term, 0, 1);
	printf("}\n");
}
