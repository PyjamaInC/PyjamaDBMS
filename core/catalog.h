#ifndef _CATALOG_H_
#define _CATALOG_H_

#include "../Parser/Sql_enums.h"
#include "stm_const.h"
#include <stdbool.h>

typedef struct _create_stm_data_ create_stm_data;
typedef struct _B_tree_ B_tree;

/**
 *  @struct schema_record
 *  @brief each schema_record stores metadata or information about a specific column in a record table. For example: a query `create table abc(age int primary, address varchar(32)), Then this struct will store metadata for each of age, address. 
 *  The collection of this struct will form a table called schema_table which has 2 fields: key - <column_name> and record - <pointer to schema_record>.
 */
typedef struct _schema_record_ {
  // store metadata of table column
  char column_name[COLUMN_NAME_MAX_SIZE];
  sql_dtpye data_type;
  int dtype_size;
  bool is_primary_key;
  bool is_non_null;
  int offset; /**< in record table, data is serialized (continuous in memory), each column and its value will start at a specific offset */

} schema_record;

/**
 * @struct catalog_entry_mdata
 * @brief the value of the field `record` of catalog table
 *
 * Catalog table by definition will hold: key - <table_name>, and record <pointer to catalog_entry_mdata>. Catalog table will be a table that stores every table there is along with a pointer to catalog_entry_mdata which allow us to access more metadata of a specific table.
 */
typedef struct _catalog_entry_mdata_{
  
  char table_name[TABLE_NAME_MAX_SIZE];
  B_tree *record_table; /**< pointer to record (RDBMS) table */
  B_tree *schema_table; /**< pointer to schmea table */
  
  char col_list[MAX_COLS_SUPPORTED_PER_TABLE][COLUMN_NAME_MAX_SIZE]; /**< List of col names in the order defined in `create table abc() query */

} catalog_entry_mdata;

bool catalog_insert_new_table(B_tree *catalog, create_stm_data *cdata);
static void catalog_t_free_fn(void *ptr);
#endif // _CATALOG_H_
