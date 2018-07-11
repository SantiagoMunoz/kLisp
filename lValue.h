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

typedef enum {LVALUE_NUM, LVALUE_ERROR} lvalue_type_t;
typedef enum {FOREACH_ERROR(make_enum)} lvalue_error_t;

typedef struct{
	lvalue_type_t type;
	long value;
	char* error_str;
}lValue;

//Constructors
lValue lValue_num(long num);
lValue lValue_err(char* error_str);

//Print
void lValue_printf(lValue in);

//Operate and eval
lValue operate(lValue a, char *op, lValue b);
lValue eval(mpc_ast_t *t);

#endif
