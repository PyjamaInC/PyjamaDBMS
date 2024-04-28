#ifndef _CREATE_STATEMENT_H_
#define _CREATE_STATEMENT_H_

#include <stdbool.h>
#include "stm_const.h"
#include "../Parser/Sql_enums.h"

typedef struct _create_stm_data_{

    char table_name[TABLE_NAME_MAX_SIZE];
    int n_cols;

    struct {

        char column_name[COLUMN_NAME_MAX_SIZE];
        sql_dtpye dtype;
        int dtype_len;
        bool is_primary_key;

    } col_data [MAX_COLS_SUPPORTED_PER_TABLE];

} create_stm_data;

void analyse_create_stm(create_stm_data *cdata);
void destroy_cdata(create_stm_data *cdata);

#endif //_CREATE_STATEMENT_H_
