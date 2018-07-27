#ifndef _LVALUE_H_
#define _LVALUE_H_
#include "mpc/mpc.h"
#include "lTypes.h"


//Constructors
lValue* lValue_num(long num);
lValue* lValue_err(char* error_str);
lValue* lValue_symbol(char* symbol);
lValue* lValue_sexpression();
lValue* lValue_qexpression();
lValue* lValue_function(lFunc f);
lValue* lValue_copy(lValue* v);

//Destructor
void lValue_free(lValue* v);
//Print
void lValue_printf(lValue* in);
//attach lvalue to another lvalue
lValue* lValue_add(lValue* parent, lValue* child);
//Read
lValue* lValue_read(mpc_ast_t *input);

lValue* lValue_pop(lValue *v, int i);
lValue* lValue_take(lValue *v, int i);
lValue* lValue_join(lValue* src, lValue* extra);
//Eval s expression
lValue* lValue_eval(lEnv* e, lValue* v);

#endif
