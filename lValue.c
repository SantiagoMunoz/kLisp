#define _GNU_SOURCE
#include "lValue.h"
#include "string.h"

#define L_ASSERT(param,cond,error) if(!cond){lValue_free(param); return lValue_err(error);}
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

lValue* lValue_qexpression()
{
	lValue* ret = (lValue*)malloc(sizeof(lValue));
	ret->type = LVALUE_QEXPRESSION;
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

lValue* builtin_head(lValue *v)
{
    L_ASSERT(v,v->count==1,"Too many arguments");
    L_ASSERT(v,v->cells[0]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    L_ASSERT(v,v->cells[0]->count!=0,"Cannot apply 'head' to an empy list");
    lValue* a = lValue_take(v, 0);
    return lValue_take(a, 0);
}

lValue* builtin_tail(lValue* v)
{
    L_ASSERT(v,v->count==1,"Too many arguments");
    L_ASSERT(v,v->cells[0]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    L_ASSERT(v,v->cells[0]->count!=0,"Cannot apply 'tail' to an empy list");
    lValue* a = lValue_take(v, 0);
    lValue *tmp = lValue_pop(a, 0);
    lValue_free(tmp);
    return a;
}

lValue* builtin_eval(lValue* v)
{
    L_ASSERT(v,v->count==1,"Too many arguments");
    L_ASSERT(v,v->cells[0]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    lValue* a = lValue_take(v, 0);
    a->type = LVALUE_SEXPRESSION;
    return lValue_eval(a);
}

lValue* builtin_list(lValue* v)
{
    v->type = LVALUE_QEXPRESSION;
    return v;
}

lValue* builtin_op(lValue* v, char* op)
{
    //Check for functions
    if(strcmp(op,"head")==0)
        return builtin_head(v);
    if(strcmp(op,"tail")==0)
        return builtin_tail(v);
    if(strcmp(op,"list")==0)
        return builtin_list(v);
    if(strcmp(op,"eval")==0)
        return builtin_eval(v);
    if(strstr("+-/*",op) == NULL){
        lValue_free(v);
        return lValue_err("Unknown function");
    }
    //Make sure all arguments are numbers
    int i;
    for(i=0;i<v->count;i++){
        if(v->cells[i]->type != LVALUE_NUM){
            lValue_free(v);
            return lValue_err("Cannot operate on non-numbers");
        }
    }

    lValue* x = lValue_pop(v, 0);
    if( (v->count == 0) & (strcmp(op,"-")==0) )
        x->value = -x->value;

    while(v->count > 0){
        lValue* y = lValue_pop(v, 0);
        if(strcmp(op,"+")==0)
            x->value += y->value;
        if(strcmp(op,"-")==0)
            x->value -= y->value;
        if(strcmp(op,"/")==0){
            if(y->value == 0){
                lValue_free(x);
                lValue_free(y);
                x = lValue_err("Division by zero");
                break;
            }
            x->value /= y->value;
        }
        if(strcmp(op,"*")==0)
            x->value *= y->value;
        lValue_free(y);
    }
    lValue_free(v);
    return x;
}

lValue* eval_sexpression(lValue *v)
{
    int i;
    //Manage the empty expression
    if(v->count == 0)
        return v;
    //First, evaluate all children
    for(i=0;i< v->count;i++){
        v->cells[i] = lValue_eval(v->cells[i]);
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
    if(f->type != LVALUE_SYMBOL){
        lValue_free(f);
        lValue_free(v);
        return lValue_err("S-Expression does not end with a symbol!");
    }
    
    lValue *result = builtin_op(v,f->symbol);
    lValue_free(f);
	return result;
}

lValue* lValue_eval(lValue *v)
{
	//If not s expression -> remain the same
	if(v->type == LVALUE_SEXPRESSION)
			return eval_sexpression(v);
	return v;	
}
