#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct Operation {
#define STRINGABLE_ENUM(C) C(Add) C(Sub) C(Mul) C(Div) 
	enum {
#define INSTANCE(inst) inst,
		STRINGABLE_ENUM(INSTANCE)
#undef INSTANCE
	};
	int type;

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
		char* num_big;
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

const char* const operators[2][2] = {
	{"+", "-"},
	{"*", "/"}
};

const char* const istrnstr(const char* const haystack, const int haystack_count, const char* const needle)
{
	const auto needle_count = strlen(needle);
	const auto haystack_end = haystack + haystack_count - 1;
	for (const char* c = haystack_end; c >= haystack; c--)
	{
		const auto comp_count = haystack + needle_count > haystack_end ? haystack_end - haystack : needle_count;
		if (strncmp(c, needle, comp_count) == 0) return c;
	}

	return 0;
}

Term* parse_string(const char* const string, const int string_count, const bool big)
{
	const auto result = alloc_term();
	int operator_index;
	int precedence_index;
	const auto string_end = string + string_count - 1;
	const char* operator_pos = string;

	for (int i = 0; i < ARR_COUNT(operators); i++)
	{
		auto last_operator_pos = operator_pos;
		for (int j = 0; j < ARR_COUNT(operators[0]); j++)
		{
			const auto found_operator_pos = istrnstr(string, string_count, operators[i][j]);
			if (found_operator_pos && found_operator_pos > last_operator_pos)
			{
				last_operator_pos = found_operator_pos;
				precedence_index = i;
				operator_index = j;
			}
		}
		if (last_operator_pos != operator_pos) {
			operator_pos = last_operator_pos;
			break;
		}
	}

	if (operator_pos == string)
	{
		// number
		result->type = Term::Number;
		if (big)
		{
			result->num_big = (char*)malloc((string_count + 1) * sizeof(char));
			strncpy(result->num_big, string, string_count);
			result->num_big[string_count] = 0;
		}
		else
			result->num = atoi(string);
	}

	if (operator_pos > string && operator_pos < string_end)
	{
		// operation
		result->type = Term::Operation;
		result->op.type = precedence_index * ARR_COUNT(operators) + operator_index;
		result->op.term1 = parse_string(string, (int)(operator_pos - string), big);
		const auto term2_pos = operator_pos + strlen(operators[precedence_index][operator_index]);
		const auto term2_count = (int)(string_end + 1 - term2_pos);
		result->op.term2 = parse_string(term2_pos, term2_count, big);
	}

	return result;
}

const char* add_big(const char* x1, const char* x2)
{
	auto x1_len = strlen(x1);
	auto x2_len = strlen(x2);
	int delta = (int)(x1_len - x2_len);
	auto new_x1 = x1;
	auto new_x2 = x2;
	if (delta < 0)
	{
		new_x1 = x2;
		new_x2 = x1;

		// switch x1_len & x2_len
		auto temp = x1_len;
		x1_len = x2_len;
		x2_len = temp;

		delta *= -1;
	}

	auto result_size = x1_len + 2;
	auto result = (char*)malloc(result_size * sizeof(char));
	int result_index = (int)(result_size - 1);
	result[result_index--] = 0;
	auto carry = 0;
	while (result_index > 0)
	{
		auto x1_num = *(new_x1 + result_index - 1) - 48;
		auto x2_offset = result_index - 1 - delta;
		auto x2_num = x2_offset >= 0 ? *(new_x2 + x2_offset) - 48 : 0;
		auto sum = x1_num + x2_num;

		result[result_index--] = ((sum + carry) % 10) + 48;
		carry = (sum + carry) / 10;
	}
	if (carry)
	{
		result[result_index] = (char)(carry + 48);
	}
	else
		result++;

	return result;
}

double calculate(const Term& term)
{
	if (term.type == Term::Number) return term.num;
	switch (term.op.type)
	{
		case Operation::Add:
			return calculate(*term.op.term1) + calculate(*term.op.term2);
		case Operation::Sub:
			return calculate(*term.op.term1) - calculate(*term.op.term2);
		case Operation::Mul:
			return calculate(*term.op.term1) * calculate(*term.op.term2);
		case Operation::Div:
			return calculate(*term.op.term1) / calculate(*term.op.term2);
	}
	return 0;
}

const char* const calculate_big(const Term& term)
{
	if (term.type == Term::Number) return term.num_big;
	switch (term.op.type)
	{
		case Operation::Add:
			return add_big(calculate_big(*term.op.term1), calculate_big(*term.op.term2));
	}
	return 0;
}

int main()
{
	const char string[] = "10312312312+31231231+22";
#if 0
	const auto term = parse_string(string, ARR_COUNT(string) - 1, false);
	printf("%f\n", calculate(*term));
#else
	const auto term = parse_string(string, ARR_COUNT(string) - 1, true);
	printf("%s\n", calculate_big(*term));
#endif

#if 0
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
#endif

	printf("digraph {\n");
	print_term(*term, 0, 1);
	printf("}\n");
}
