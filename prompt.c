#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <histedit.h>
#include <string.h>
#include "mpc/mpc.h"
#include "lValue.h"

int main(int argc, char **argv)
{
		mpc_parser_t* Number = mpc_new("number");
		mpc_parser_t* Symbol = mpc_new("symbol");
		mpc_parser_t* Expression = mpc_new("expression");
		mpc_parser_t* sExpression = mpc_new("sexpression");
		mpc_parser_t* kLisp = mpc_new("klisp");

		//Reverse polac notation
		mpca_lang(MPCA_LANG_DEFAULT,
						"																			\
						number: /-?[0-9]+/;															\
						symbol: '+' | '-' | '/' | '*';												\
						sexpression: '(' <expression>* ')';											\
						expression : <number> | <symbol> | <sexpression>;							\
						klisp :/^/ <expression>* /$/;												\
						",
						Number, Symbol, sExpression, Expression, kLisp);

		printf("kLisp version 0.0.0.0.1\n");
		printf("Press Ctrl+c to Exit\n\n");

		while(1){
				char *input = readline("kLisp> ");
				add_history(input);
				mpc_result_t r;
				if(mpc_parse("<stdin>", input, kLisp, &r)){
						lValue* v= lValue_read(r.output);
                        v = lValue_eval(v);
						lValue_printf(v);
                        putchar('\n');
						lValue_free(v);
						mpc_ast_delete(r.output);
				}else{
						mpc_err_print(r.error);
						mpc_err_delete(r.error);
				}
				free(input);
		}

		mpc_cleanup(5, Number, Symbol, sExpression, Expression, kLisp);
		return 0;
}
