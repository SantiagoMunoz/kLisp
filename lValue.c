#define _GNU_SOURCE
#include "lValue.h"
#include "lEnv.h"
#include <string.h>

char* error_strings[] = {FOREACH_ERROR(make_str)};

lValue* lValue_num(long num)
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_NUM;
	ret->value = num;
	return ret;
}

lValue* lValue_err(char* error_str)
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

lValue* lValue_qexpression()
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_QEXPRESSION;
	ret->count = 0;
	ret->cells = NULL;
	return ret;
}

lValue* lValue_function(lFunc f)
{
    lValue* v = (lValue*)malloc(sizeof(lValue));
    v->type = LVALUE_FUNCTION;
    v->fun = f;
    return v;
}

lValue* lValue_copy(lValue* v)
{
    if(!v)
        return NULL;
    lValue* new = (lValue*)malloc(sizeof(lValue)); 
    new->type = v->type;
    switch(v->type){
        case LVALUE_NUM:
            new->value = v->value;
            break;
        case LVALUE_ERROR:
            asprintf(&new->error_str,"%s",v->error_str);
            break;
        case LVALUE_SYMBOL:
            asprintf(&new->symbol,"%s",v->symbol);            
            break;
        case LVALUE_SEXPRESSION:
            //Fallthrough
        case LVALUE_QEXPRESSION:
            new->count = v->count;
            new->cells = (lValue**)malloc(sizeof(lValue*)*new->count);
            int i;
            for(i=0;i<new->count;i++)
                new->cells[i] = lValue_copy(v->cells[i]);
            break;
        case LVALUE_FUNCTION:
            new->fun= v->fun;
    }
    return new;
}

void lValue_free(lValue* v)
{
	switch(v->type){
		case LVALUE_NUM:
            //Fallthrough
        case LVALUE_FUNCTION:
			break;
		case LVALUE_ERROR:
			free(v->error_str);
			break;
		case LVALUE_SYMBOL:
			free(v->symbol);
			break;
        case LVALUE_SEXPRESSION:
            //Fallthrough
        case LVALUE_QEXPRESSION:
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
        case LVALUE_FUNCTION:
            printf("<function>");
            break;
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
        case LVALUE_QEXPRESSION:
            lValue_expression_printf(in, '{', '}');
            break;
	}
}

lValue* lValue_add(lValue* parent, lValue* child)
{
	if(!(parent->type == LVALUE_SEXPRESSION) & !(parent->type == LVALUE_QEXPRESSION))
		return parent;
    if(child == NULL)
        return parent;
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
    else if(strstr(input->tag,"qexpr")!= NULL)
        v = lValue_qexpression();
	else
		return NULL;
	int i;
	for(i=0;i< input->children_num;i++){
		if( strcmp(input->children[i]->contents, "(") == 0)	continue;
		if( strcmp(input->children[i]->contents, ")") == 0)	continue;
		if( strcmp(input->children[i]->contents, "{") == 0)	continue;
		if( strcmp(input->children[i]->contents, "}") == 0)	continue;
		if( strcmp(input->children[i]->contents, "regex") == 0)	continue;
		v = lValue_add(v, lValue_read(input->children[i]));
	}
	return v;
}

lValue* lValue_pop(lValue *v, int i)
{
    if(i > (v->count + 1))
        return NULL;
    lValue* x = v->cells[i];
    memmove(&v->cells[i], &v->cells[i+1], sizeof(lValue*)*(v->count - 1 - i));
    v->count --;
    v->cells = realloc(v->cells, sizeof(lValue*)*v->count);
    return x;
}

lValue* lValue_take(lValue *v, int i)
{
    lValue* x = lValue_pop(v, i);
    lValue_free(v);
    return x;
}

lValue* lValue_join(lValue* src, lValue* extra)
{
    int i;
    while(extra->count> 0)
        src = lValue_add(src,lValue_pop(extra, 0));
    return src;
}

lValue* eval_sexpression(lEnv* e, lValue* v)
{
    int i;
    //Manage the empty expression
    if(v->count == 0)
        return v;
    //First, evaluate all children
    for(i=0;i< v->count;i++){
        v->cells[i] = lValue_eval(e,v->cells[i]);
    }
    //Have there been any errors?
    for(i=0;i< v->count;i++){
        if(v->cells[i]->type == LVALUE_ERROR)
            return lValue_take(v,i);
    }
    //Manage single expression
    if(v->count == 1)
       return lValue_take(v,0); 
    //Rest of cases -> Process as normal 
    lValue *f = lValue_pop(v,v->count - 1);
    if(f->type != LVALUE_FUNCTION){
        lValue_free(f);
        lValue_free(v);
        return lValue_err("S-Expression does not end with a function!");
    }
    
    lValue *result = f->fun(e, v);
    lValue_free(f);
	return result;
}

lValue* lValue_eval(lEnv* e, lValue *v)
{
    if(v->type == LVALUE_SYMBOL)
        return lEnv_get(e, v);
	if(v->type == LVALUE_SEXPRESSION)
	    return eval_sexpression(e, v);
	return v;	
}
