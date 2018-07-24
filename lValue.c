#define _GNU_SOURCE
#include "lValue.h"
#include "string.h"
char* error_strings[] = {FOREACH_ERROR(make_str)};

lValue* lValue_num(long num)
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_NUM;
	ret->value = num;
	return ret;
}

lValue* lValue_err(char *error_str)
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_ERROR;
	asprintf(&ret->error_str, "%s", error_str);
	return ret;
}

lValue* lValue_symbol(char* symbol)
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_SYMBOL;
	asprintf(&ret->symbol, "%s", symbol);
	return ret;
}

lValue* lValue_sexpression()
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_SEXPRESSION;
	ret->count = 0;
	ret->cells = NULL;
	return ret;
}

void lValue_free(lValue* v)
{
	switch(v->type){
		case LVALUE_NUM:
			break;
		case LVALUE_ERROR:
			free(v->error_str);
			break;
		case LVALUE_SYMBOL:
			free(v->symbol);
			break;
		case LVALUE_SEXPRESSION:
			while(v->count > 0){
				lValue_free(v->cells[v->count-1]);
				v->count--;
			}
			free(v->cells);
			break;
	}
	free(v);
}

void lValue_expression_printf(lValue *exp, char start, char close)
{
	int i;
	putchar(start);
	for(i=0;i<exp->count;i++){
		lValue_printf(exp->cells[i]);
		if(i != exp->count-1)
				putchar(' ');
	}
	putchar(close);
	return;
}

void lValue_printf(lValue* in)
{
	switch(in->type){
		case LVALUE_NUM:
			printf("%li", in->value);
			break;
		case LVALUE_ERROR:
			if(in->error_str)
				printf("Error: %s",in->error_str);
			break;
		case LVALUE_SYMBOL:
			if(in->symbol)
				printf("%s", in->symbol);
			break;
		case LVALUE_SEXPRESSION:
			lValue_expression_printf(in, '(', ')');
			break;
	}
}

lValue* lValue_add(lValue* parent, lValue* child)
{
	if(parent->type != LVALUE_SEXPRESSION)
			return NULL;
	parent->count++;
	parent->cells = realloc(parent->cells, parent->count * sizeof(lValue*));
	parent->cells[parent->count - 1] = child;
	return parent;
}

lValue* read_num(mpc_ast_t* input)
{
	errno = 0;
	long n = strtol(input->contents, NULL, 10);
	if(errno != ERANGE)
		return lValue_num(n);
		return lValue_err("Not a valid number");
}

lValue* lValue_read(mpc_ast_t* input)
{
	if(strstr(input->tag,"number"))
		return read_num(input);
	if(strstr(input->tag,"symbol"))
		return  lValue_symbol(input->contents);
	//If root node or sexpression -> create empty list and add to it everything below
	lValue* v = NULL;
	if((strcmp(input->tag,">")==0) | (strstr(input->tag,"sexpr")!= NULL))
		v = lValue_sexpression();
	else
		return NULL;
	int i;
	for(i=0;i< input->children_num;i++){
		if( strcmp(input->children[i]->contents, "(") == 0)	continue;
		if( strcmp(input->children[i]->contents, ")") == 0)	continue;
		if( strcmp(input->children[i]->contents, "regex") == 0)	continue;
		v = lValue_add(v, lValue_read(input->children[i]));
	}
	return v;
}

lValue* eval_sexpression(lValue *v)
{
	//TODO Implement this
	return v;
}

lValue* lValue_eval(lValue *v)
{
	//If not s expression -> remain the same
	if(v->type == LVALUE_SEXPRESSION)
			return eval_sexpression(v);
	return v;	
}
