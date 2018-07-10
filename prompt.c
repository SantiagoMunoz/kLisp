#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <histedit.h>
#include <string.h>
#include "mpc/mpc.h"

#define make_str(s)	#s,
#define make_enum(s)	s,

#define FOREACH_ERROR(ACTION)						\
			ACTION(LVALUE_ERR_NONE)					\
			ACTION(LVALUE_ERR_DIV_ZERO)				\
			ACTION(LVALUE_ERR_BAD_OPERATION)		\
			ACTION(LVALUE_ERR_BAD_NUMBER)			\



typedef enum {LVALUE_NUM, LVALUE_ERROR} lvalue_type_t;
typedef enum {FOREACH_ERROR(make_enum)} lvalue_error_t;
char* error_strings[] = {FOREACH_ERROR(make_str)};

typedef struct{
	lvalue_type_t type;
	long value;
	lvalue_error_t error;
} lValue;

lValue lValue_num(long num)
{
	lValue ret = {
			.type = LVALUE_NUM,
			.value = num,
			.error = LVALUE_ERR_NONE };
	return ret;
}

lValue lValue_err(lvalue_error_t err)
{
	lValue ret = {
			.type = LVALUE_ERROR,
			.value = 0,
			.error = err};
	return ret;
}

void print_lValue(lValue in)
{
	if(in.type == LVALUE_NUM)
		printf("%li\n", in.value);
	if(in.type == LVALUE_ERROR)
		printf("Error: %s\n",error_strings[in.error]);
}

lValue operate(lValue a, char* op, lValue b)
{
	if( (a.type != LVALUE_NUM) | (b.type != LVALUE_NUM) ){
		return lValue_err(LVALUE_ERR_BAD_NUMBER);
	}

	if(!strcmp(op,"+")) return lValue_num(a.value+b.value);
	if(!strcmp(op,"-")) return lValue_num(a.value-b.value);
	if(!strcmp(op,"*")) return lValue_num(a.value*b.value);
	if(!strcmp(op,"/")) return b.value != 0 ? lValue_num(a.value/b.value) : lValue_err(LVALUE_ERR_DIV_ZERO);
	if(!strcmp(op,"%")) return lValue_num(a.value%b.value);
	if(!strcmp(op,"^")) return lValue_num(a.value^b.value);
	if(!strcmp(op,"min")) return lValue_num(a.value < b.value ? a.value : b.value);
	if(!strcmp(op,"max")) return lValue_num(a.value > b.value ? a.value : b.value);

	return lValue_err(LVALUE_ERR_BAD_OPERATION);
}

lValue eval(mpc_ast_t *t)
{
	if(strstr(t->tag,"number")){
		long tmp = strtol(t->contents, NULL, 10);
		if(errno != ERANGE)
			return lValue_num(tmp);
		else
			return lValue_err(LVALUE_ERR_BAD_NUMBER);
	}
	int i = t->children_num -1;
	while(!strstr(t->children[i]->tag,"operator"))
		i--;
	char *op = t->children[i]->contents;

	lValue num = eval(t->children[1]);

	i=2;
	while(strstr(t->children[i]->tag,"expr")){
		num = operate(num, op, eval(t->children[i]));
		i++;
	}
	return num;
}


int main(int argc, char **argv)
{
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expression = mpc_new("expression");
	mpc_parser_t* kLisp = mpc_new("klisp");

	//Reverse polac notation
	mpca_lang(MPCA_LANG_DEFAULT,
			"																			\
			number :/-?[0-9]+/;															\
			operator :'+' | '-' | '/' | '*' | '^' | \"mini\" | \"max\" | '\%';			\
			expression :<number> | '(' <expression>+ <operator> ')';					\
			klisp :/^/<expression>+ <operator>/$/;										\
			",
			Number, Operator, Expression, kLisp);

	printf("kLisp version 0.0.0.0.1\n");
	printf("Press Ctrl+c to Exit\n\n");

	while(1){
		char *input = readline("kLisp> ");
		add_history(input);
		mpc_result_t r;
		if(mpc_parse("<stdin>", input, kLisp, &r)){
			lValue result = eval(r.output);
			print_lValue(result);
			mpc_ast_delete(r.output);
		}else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}

	mpc_cleanup(4, Number, Operator, Expression, kLisp);
	return 0;
}
