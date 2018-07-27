#include "lBuiltins.h"


lValue* builtin_add(lEnv* e, lValue* v);
lValue* builtin_sub(lEnv* e, lValue* v);
lValue* builtin_mul(lEnv* e, lValue* v);
lValue* builtin_div(lEnv* e, lValue* v);
lValue* builtin_head(lEnv *e, lValue* v);
lValue* builtin_tail(lEnv *e, lValue* v);
lValue* builtin_eval(lEnv* e, lValue* v);
lValue* builtin_list(lEnv* e, lValue* v);
lValue* builtin_join(lEnv* e, lValue* v);
lValue* builtin_def(lEnv* e, lValue* v);

void lEnv_add_builtin(lEnv* e, lFunc f, char* name)
{
    if(!name)
        return;
    lValue* fun = lValue_function(f);
    lValue* sym = lValue_symbol(name);
    lEnv_add(e,fun, sym);
    lValue_free(fun);
    lValue_free(sym);

}

void lEnv_add_builtInFunctions(lEnv* e)
{
    if(!e)
        return;
    lEnv_add_builtin(e,builtin_add,"+");
    lEnv_add_builtin(e,builtin_sub,"-");
    lEnv_add_builtin(e,builtin_mul,"*");
    lEnv_add_builtin(e,builtin_div,"/");
    lEnv_add_builtin(e,builtin_head,"head");
    lEnv_add_builtin(e,builtin_tail,"tail");
    lEnv_add_builtin(e,builtin_eval,"eval");
    lEnv_add_builtin(e,builtin_list,"list");
    lEnv_add_builtin(e,builtin_join,"join");
    lEnv_add_builtin(e,builtin_def,"def");
}

lValue* builtin_head(lEnv* e, lValue *v)
{
    L_ASSERT(v,v->count==1,"Too many arguments");
    L_ASSERT(v,v->cells[0]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    L_ASSERT(v,v->cells[0]->count!=0,"Cannot apply 'head' to an empy list");
    lValue* a = lValue_take(v, 0);
    lValue* ret = lValue_qexpression();
    ret = lValue_add(ret,lValue_take(a, 0));
    return ret;
}

lValue* builtin_tail(lEnv* e, lValue* v)
{
    L_ASSERT(v,v->count==1,"Too many arguments");
    L_ASSERT(v,v->cells[0]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    L_ASSERT(v,v->cells[0]->count!=0,"Cannot apply 'tail' to an empy list");
    lValue* a = lValue_take(v, 0);
    lValue *tmp = lValue_pop(a, 0);
    lValue_free(tmp);
    return a;
}

lValue* builtin_eval(lEnv* e, lValue* v)
{
    L_ASSERT(v,v->count==1,"Too many arguments");
    L_ASSERT(v,v->cells[0]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    lValue* a = lValue_take(v, 0);
    a->type = LVALUE_SEXPRESSION;
    return lValue_eval(e,a);
}

lValue* builtin_list(lEnv* e, lValue* v)
{
    v->type = LVALUE_QEXPRESSION;
    return v;
}

lValue* builtin_operators(lEnv *e, lValue* v, char op)
{
    //Make sure all arguments are numbers
    int i;
    for(i=0;i<v->count;i++){
        if(v->cells[i]->type != LVALUE_NUM){
            lValue_free(v);
            return lValue_err("Cannot operate on non-numbers");
        }
    }

    lValue* x = lValue_pop(v, 0);
    if((v->count == 0) & (op=='-'))
        x->value = -x->value;

    while(v->count > 0){
        lValue* y = lValue_pop(v, 0);
        if(op=='+')
            x->value += y->value;
        if(op=='-')
            x->value -= y->value;
        if(op=='/'){
            if(y->value == 0){
                lValue_free(x);
                lValue_free(y);
                x = lValue_err("Division by zero");
                break;
            }
            x->value /= y->value;
        }
        if(op=='*')
            x->value *= y->value;
        lValue_free(y);
    }
    lValue_free(v);
    return x;
}

lValue* builtin_add(lEnv* e, lValue* v)
{
    return builtin_operators(e, v, '+');
}

lValue* builtin_sub(lEnv* e, lValue* v)
{
    return builtin_operators(e, v, '-');
}

lValue* builtin_mul(lEnv* e, lValue* v)
{
    return builtin_operators(e, v, '*');
}

lValue* builtin_div(lEnv* e, lValue* v)
{
    return builtin_operators(e, v, '/');
}

lValue* builtin_join(lEnv*e, lValue* v)
{
    L_ASSERT(v,v->count>1,"Too few arguments");
    int i;
    for(i=0;i<v->count;i++){
        L_ASSERT(v,v->cells[i]->type==LVALUE_QEXPRESSION,"Wrong operand type");
    }
    lValue* a = lValue_qexpression();
    for(i=0;i<v->count;i++){
        a= lValue_join(a, v->cells[i]);
    }
    lValue_free(v);
    return a;
}

lValue* builtin_def(lEnv* e, lValue* v)
{
    //First argument must be a list of symbols
    L_ASSERT(v,v->type == LVALUE_QEXPRESSION, "First argument of 'def' must be a list");
    lValue* symbols = lValue_pop(v, 0);
    //Make sure all of the elements are symbols
    int i;
    for(i=0;i<symbols->count;i++)
        L_ASSERT(v, symbols->cells[i]->type == LVALUE_SYMBOL, "Cant define a non-symbol");
    L_ASSERT(v,v->count==symbols->count,"Uneven number of variables and symbols");
    for(i=0;i<symbols->count;i++)
        lEnv_add(e,v->cells[i],symbols->cells[i]);
    return lValue_sexpression();
}
