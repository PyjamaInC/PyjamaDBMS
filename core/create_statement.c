#include "create_statement.h"
#include "catalog.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

extern B_tree TableCatalog;

void analyse_create_stm(create_stm_data *cdata) {
  catalog_insert_new_table(&TableCatalog, cdata);
  printf("%s():called...\n", __FUNCTION__);
}

void destroy_cdata(create_stm_data *cdata) { memset(cdata, 0, sizeof(*cdata)); }

/**
* @brief The function will convert information in create_stm_data struct which holds all information we can extract from a typical `create table abc ()` query. Then the function look for columns that act as primary key in a normal DBMS, store their metadata inside struct md_key, which will finally be inserted to the returned array.
* @param cdata A pointer to struct create_stm_data 
* @param key_md_size A pointer to an int variable which will be updated by this function to reflect how many primary keys there are in the table created by this `create table abc()` query.
* @return Return a pointer to the first element of the array whose elements are md_key structs.
*/
md_key *new_table_keymd(create_stm_data *cdata, int *key_md_size) {

  int i, j;
  int pk_count = 0; // count of primary keys

  // count the number of primary key (pk) in cdata
  for (i = 0; i < cdata->n_cols; i++) {
    if (cdata->col_data[i].is_primary_key)
      pk_count++;
  }

  // check if the current cdata has any pk or not, if not, return NULL
  if (pk_count == 0) {
    *key_md_size = 0;
    return NULL;
  }

  // an array storing *key_md_size elements
  md_key *mdkey_array = (md_key *)calloc(pk_count, sizeof(md_key));

  for (i = 0, j = 0; i < cdata->n_cols; i++) {

    if (cdata->col_data[i].is_primary_key) {
      mdkey_array[i].dtype = cdata->col_data[i].dtype;
      mdkey_array[i].size = cdata->col_data[i].dtype_len;
      j++;
    }
  }
  *key_md_size = j;
  return mdkey_array;
}
