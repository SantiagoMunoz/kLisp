#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <histedit.h>
#include <string.h>
#include "mpc/mpc.h"

long operate(long a, char* op, long b)
{
    if(!strcmp(op,"+")) return a+b;
    if(!strcmp(op,"-")) return a-b;
    if(!strcmp(op,"*")) return a*b;
    if(!strcmp(op,"/")) return a/b;
    if(!strcmp(op,"%")) return a%b;
    if(!strcmp(op,"^")) return a^b;
    if(!strcmp(op,"min")) return a < b ? a : b;
    if(!strcmp(op,"max")) return a > b ? a : b;

    return 0;
}

int eval(mpc_ast_t *t)
{
    if(strstr(t->tag,"number"))
        return atoi(t->contents);
    int i = t->children_num -1;
    while(!strstr(t->children[i]->tag,"operator"))
        i--;
    char *op = t->children[i]->contents;

    long num = eval(t->children[1]);

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
            "								\
            number :/-?[0-9]+/;						\
            operator :'+' | '-' | '/' | '*' | '^' | \"mini\" | \"max\" | '\%';			\
            expression :<number> | '(' <expression>+ <operator> ')';	\
            klisp :/^/<expression>+ <operator>/$/;			\
            ",
            Number, Operator, Expression, kLisp);

    printf("kLisp version 0.0.0.0.1\n");
    printf("Press Ctrl+c to Exit\n\n");

    while(1){
        char *input = readline("kLisp> ");
        add_history(input);
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, kLisp, &r)){
            long result = eval(r.output);
            printf("%li\n", result);
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
