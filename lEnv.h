#ifndef _LENV_H_
#define _LENV_H_
#include "lTypes.h"

lEnv* lEnv_new();
void lEnv_free(lEnv* e);
lValue* lEnv_get(lEnv* e,lValue* sym);
void lEnv_add(lEnv* e,lValue* value, lValue *symbol);

#endif /* _LENV_H_ */
