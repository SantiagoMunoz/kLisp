#ifndef _LTYPES_H_
#define _LTYPES_H_

#define make_str(s)	#s,
#define make_enum(s)	s,
#define L_ASSERT(param,cond,error) if(!cond){lValue_free(param); return lValue_err(error);}

#define FOREACH_ERROR(ACTION)					\
		ACTION(LVALUE_ERR_NONE)					\
		ACTION(LVALUE_ERR_DIV_ZERO)				\
		ACTION(LVALUE_ERR_BAD_OPERATION)		\
		ACTION(LVALUE_ERR_BAD_NUMBER)			\

typedef enum {LVALUE_NUM, LVALUE_ERROR, LVALUE_SYMBOL, LVALUE_SEXPRESSION, LVALUE_QEXPRESSION, LVALUE_FUNCTION} lvalue_type_t;
typedef enum {FOREACH_ERROR(make_enum)} lvalue_error_t;

struct lValue;
typedef struct lValue lValue;

typedef struct{
    char* name;
    lValue* value;
} lSymbol;

typedef struct{
    int count;
    lSymbol **cells;
} lEnv;

typedef lValue*(*lFunc)(lEnv*,lValue*);

struct lValue{
	lvalue_type_t type;
	long value;
	char *symbol;
	char* error_str;
    lFunc fun;
	//for symbolic expressions
	int count;
	struct lValue **cells;
};

#endif
