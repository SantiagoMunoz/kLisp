#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "lEnv.h"
#include "lValue.h"

lEnv* lEnv_new()
{
    lEnv* tmp = (lEnv*)malloc(sizeof(lEnv));
    tmp->count = 0;
    tmp->cells = NULL; 
    return tmp;
}

void lEnv_free(lEnv* e)
{
    if(!e)
        return;
    int i;
    for(i=0;i<e->count;i++){
        free(e->cells[i]->name);
        lValue_free(e->cells[i]->value);
        free(e->cells[i]);
    }
    free(e->cells);
    free(e);
}

lValue* lEnv_get(lEnv* e,lValue* sym)
{
    if((!e) | (!sym))
        lValue_err("Internal error");
    int i;
    for(i=0;i<e->count;i++){
        if(strcmp(sym->symbol,e->cells[i]->name) == 0)
            return lValue_copy(e->cells[i]->value);    
    }
    return lValue_err("Undefined symbol");
}

void lEnv_add(lEnv* e,lValue* value, lValue *symbol)
{
    //Check that the symbol does not exist already
    int i;
    for(i=0;i<e->count;i++)
        if(strcmp(e->cells[i]->name,symbol->symbol)==0)
            return;
    //Not present->add it!
    e->count++;
    if(e->count == 1)
        e->cells = (lSymbol**)malloc(sizeof(lSymbol*));
    else
        e->cells = (lSymbol**)realloc(e->cells,sizeof(lSymbol*)*e->count);
    lSymbol* tmp = (lSymbol*)malloc(sizeof(lSymbol));
    asprintf(&tmp->name,"%s",symbol->symbol);
    tmp->value = lValue_copy(value);
    e->cells[e->count-1] = tmp;
}
