#ifndef _CREATE_STATEMENT_H_
#define _CREATE_STATEMENT_H_

#include "../Parser/Sql_enums.h"
#include "stm_const.h"
#include "../B_tree/B_tree.h"
#include <stdbool.h>
typedef struct _create_stm_data_ {
  // cdata
  char table_name[TABLE_NAME_MAX_SIZE];
  int n_cols;

  struct {

    char column_name[COLUMN_NAME_MAX_SIZE];
    sql_dtpye dtype;
    int dtype_len;
    bool is_primary_key;

  } col_data[MAX_COLS_SUPPORTED_PER_TABLE];

} create_stm_data;

void analyse_create_stm(create_stm_data *cdata);
void destroy_cdata(create_stm_data *cdata);
md_key *new_table_keymd(create_stm_data *cdata, int *key_md_size);
#endif //_CREATE_STATEMENT_H_
