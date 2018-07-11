#define _GNU_SOURCE
#include "lValue.h"
#include "string.h"
char* error_strings[] = {FOREACH_ERROR(make_str)};

lValue lValue_num(long num)
{
		lValue ret = {
				.type = LVALUE_NUM,
				.value = num,
		};
		return ret;
}

lValue lValue_err(char *error_str)
{
		lValue ret = {
				.type = LVALUE_ERROR,
				.value = 0,
		};
		asprintf(&ret.error_str, "%s", error_str);
		return ret;
}

void lValue_printf(lValue in)
{
		if(in.type == LVALUE_NUM)
				printf("%li\n", in.value);
		if((in.type == LVALUE_ERROR) & (in.error_str != NULL))
				printf("Error: %s\n",in.error_str);
}

lValue operate(lValue a, char* op, lValue b)
{
		if( (a.type != LVALUE_NUM) | (b.type != LVALUE_NUM) ){
				return lValue_err(error_strings[LVALUE_ERR_BAD_NUMBER]);
		}

		if(!strcmp(op,"+")) return lValue_num(a.value+b.value);
		if(!strcmp(op,"-")) return lValue_num(a.value-b.value);
		if(!strcmp(op,"*")) return lValue_num(a.value*b.value);
		if(!strcmp(op,"/")) return b.value != 0 ? lValue_num(a.value/b.value) : lValue_err(error_strings[LVALUE_ERR_DIV_ZERO]);
		if(!strcmp(op,"%")) return lValue_num(a.value%b.value);
		if(!strcmp(op,"^")) return lValue_num(a.value^b.value);
		if(!strcmp(op,"min")) return lValue_num(a.value < b.value ? a.value : b.value);
		if(!strcmp(op,"max")) return lValue_num(a.value > b.value ? a.value : b.value);

		return lValue_err(error_strings[LVALUE_ERR_BAD_OPERATION]);
}

lValue eval(mpc_ast_t *t)
{
		if(strstr(t->tag,"number")){
				long tmp = strtol(t->contents, NULL, 10);
				if(errno != ERANGE)
						return lValue_num(tmp);
				else
						return lValue_err(error_strings[LVALUE_ERR_BAD_NUMBER]);
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

