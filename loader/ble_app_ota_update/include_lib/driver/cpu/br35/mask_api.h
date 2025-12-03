#ifndef  __MASK_API_H__
#define  __MASK_API_H__


#include "typedef.h"

#include "maskrom.h"

void exception_analyze(u32 *sp);

static void mask_api_init(void *pchar, void *exp_hook)
{
    struct maskrom_argv table;
    memset((void *)&table, 0, sizeof(struct maskrom_argv));
    table.exp_hook = exp_hook;
    table.pchar = pchar;
    mask_init(&table);
}

#endif  /*MASK_API_H*/

