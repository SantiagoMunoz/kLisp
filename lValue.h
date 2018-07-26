#ifndef _LVALUE_H_
#define _LVALUE_H_
#include "mpc/mpc.h"

#define make_str(s)	#s,
#define make_enum(s)	s,

#define FOREACH_ERROR(ACTION)					\
		ACTION(LVALUE_ERR_NONE)					\
		ACTION(LVALUE_ERR_DIV_ZERO)				\
		ACTION(LVALUE_ERR_BAD_OPERATION)		\
		ACTION(LVALUE_ERR_BAD_NUMBER)			\

typedef enum {LVALUE_NUM, LVALUE_ERROR, LVALUE_SYMBOL, LVALUE_SEXPRESSION, LVALUE_QEXPRESSION} lvalue_type_t;
typedef enum {FOREACH_ERROR(make_enum)} lvalue_error_t;

typedef struct lValue{
	lvalue_type_t type;
	long value;
	char *symbol;
	char* error_str;
	//for symbolic expressions
	int count;
	struct lValue **cells;
}lValue;

//Constructors
lValue* lValue_num(long num);
lValue* lValue_err(char* error_str);
lValue* lValue_symbol(char* symbol);
lValue* lValue_sexpression();
lValue* lValue_qexpression();

//Destructor
void lValue_free(lValue* v);
//Print
void lValue_printf(lValue* in);
//attach lvalue to another lvalue
lValue* lValue_add(lValue* parent, lValue* child);
//Read
lValue* lValue_read(mpc_ast_t *input);

//Eval s expression
lValue* lValue_eval(lValue *v);

#endif
