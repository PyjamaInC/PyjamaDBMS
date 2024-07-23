#ifndef _INSERT_STATEMENT_H_
#define _INSERT_STATEMENT_H_


#include "../Parser/Sql_enums.h"
#include "stm_const.h"
#include <stdbool.h>


typedef struct _sql_value_ {

    sql_dtpye dtype;

    union {

        char str_val[STRING_VAL_MAX_LEN];
        int int_val;
        double d_val;

    } u;

} sql_value;

typedef struct _insert_stm_data_ {

    char table_name[TABLE_NAME_MAX_SIZE];
    int m;
    sql_value sql_vals[MAX_COLS_SUPPORTED_PER_TABLE];

} insert_stm_data;




#endif // _INSERT_STATEMENT_H