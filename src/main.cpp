#include <stdio.h>
#include <assert.h>

struct Term {
	enum { Num, Operation } type;
	union {
		double num;
		struct Operation* op;
	};
};

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

	Term term1, term2;
};

const auto operation_pool_cap = 1024;
Operation operation_pool[operation_pool_cap];
int operation_pool_count = 0;

void print_op(const Operation* const op);

void print_term(const Term& term, const int parent_index, const int term_num)
{
	if (term.type == Term::Operation)
	{
		const auto term_index = (int)(term.op - operation_pool);
		printf("\top_%d [label=%s]\n", term_index, Operation::enum_get_string(term.type));
		printf("\top_%d -> op_%d\n", parent_index, term_index);
		print_op(term.op);
	}
	else
	{
		printf("\tnum_%d_%d [label=%f]\n", term_num, parent_index, term.num);
		printf("\top_%d -> num_%d_%d\n", parent_index, term_num, parent_index);
	}
}

void print_op(const Operation* const op)
{
	const auto index = (int)(op - operation_pool);
	printf("\top_%d [label=%s]\n", index, Operation::enum_get_string(op->type));
	print_term(op->term1, index, 1);
	print_term(op->term2, index, 2);
}

Operation* alloc_operation()
{
	assert(operation_pool_count < operation_pool_cap);
	return &operation_pool[operation_pool_count++];
}

Operation* parse_string(const char* const string)
{
	auto op = alloc_operation();
	op->type = Operation::Mul;
	op->term1.type = Term::Operation;
	op->term1.op = alloc_operation();
	op->term1.op->type = Operation::Add;
	op->term1.op->term1.type = Term::Num;
	op->term1.op->term1.num = 5;
	op->term1.op->term2.type = Term::Num;
	op->term1.op->term2.num = 6;

	op->term2.type = Term::Num;
	op->term2.num = 2;

	return op;
}

int main()
{
	const auto string = "3*2";
	const auto op = parse_string(string);

	printf("digraph {\n");
	print_op(op);
	printf("}\n");
}
