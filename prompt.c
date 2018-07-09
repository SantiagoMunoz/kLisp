#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <histedit.h>
#include "mpc/mpc.h"

int main(int argc, char **argv)
{
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");	
	mpc_parser_t* Expression = mpc_new("expression");
	mpc_parser_t* kLisp = mpc_new("klisp");
	
	//Reverse polac notation
	mpca_lang(MPCA_LANG_DEFAULT,
			"								\
			number : /-?[0-9]+/ ;						\
			operator : '+' | '-' | '/' | '*' ;				\
			expression : <number> | '(' <expression>+ <operator> ')' ;	\
			klisp : /^/ <expression>+ <operator> /$/ ;			\
			",
			Number, Operator, Expression, kLisp);

	printf("kLisp version 0.0.0.0.1\n");
	printf("Press Ctrl+c to Exit\n\n");

	while(1){
		char *input = readline("kLisp> ");
		add_history(input);
		mpc_result_t r;
		if(mpc_parse("<stdin>", input, kLisp, &r)){
			mpc_ast_print(r.output);
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
