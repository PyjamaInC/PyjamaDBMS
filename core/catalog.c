#include "catalog.h"
#include "../B_tree/B_tree.h"
#include "../Parser/Sql_enums.h"
#include "create_statement.h"
#include "stm_const.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int tree_comp_fn(B_key *key1, B_key *key2, md_key *key_meta_d,
                        int key_md_size);

B_tree TableCatalog;
static void st_record_free(void *ptr) { free(ptr); };

/**
 @brief this function was meant to first convert information store in `cdata`
 (which is short for column data) into an entry of schema table. Entry of schema table include 2 fields: key <column_name> and record <ptr to corresponding schema_record>. Then they are inserted into the tree schema_table via API from B_tree library B_tree_Insert()
 @param schema_table A pointer to a tree which actually is schema table, is passed as `B_tree TableCatalog` which is extern in other files.
 @param cdata A pointer to a create_stm_data which holds the meta data of a normal DBMS table. Its member col_data is an array stores only data about all of columns in this normal DBMS table
 @return nothing
 */
static void Catalog_new_schema_table(B_tree *schema_table,
                                     create_stm_data *cdata) {

  int i;
  int offset = 0;
  B_key bkey_template;
  schema_record *record;

  for (i = 0; i < cdata->n_cols; i++) {
    // Set up the key
    bkey_template.key = (void *)calloc(1, COLUMN_NAME_MAX_SIZE);
    strncpy((char *)bkey_template.key, cdata->col_data[i].column_name,
            COLUMN_NAME_MAX_SIZE);
    bkey_template.key_size = COLUMN_NAME_MAX_SIZE;

    // setup value of the record
    record = (schema_record *)calloc(1, sizeof(schema_record));
    strncpy(record->column_name, cdata->col_data[i].column_name,
            COLUMN_NAME_MAX_SIZE);
    record->data_type = cdata->col_data[i].dtype;
    record->dtype_size = cdata->col_data[i].dtype_len;
    record->is_primary_key = cdata->col_data[i].is_primary_key;
    record->offset = offset;
    offset += record->dtype_size;

    // Insert into schema table
    assert(B_tree_Insert(schema_table, &bkey_template, (void *)record));
  }
};

static bool initialized = false;

bool catalog_insert_new_table(B_tree *catalog, create_stm_data *cdata) {

  /* Implementation: create Catalog table
   * This table entries has 2 fields:
   * - key: <table_name>
   * - rcc: <pointer to catalog_entry_mdata object>
   */
  int i;
  B_key bkey;

  static md_key mdkey_array[] = {{SQL_STRING, TABLE_NAME_MAX_SIZE}};
   
  if (!initialized){
    B_tree_init(catalog,
                tree_comp_fn,
                NULL, NULL,
                MAX_CHILD_PER_CATALOG_TABLE,
                catalog_t_free_fn,
                mdkey_array,
                sizeof(mdkey_array)/sizeof(md_key));
    initialized = true;
  }

  bkey.key = (void *)calloc(1, TABLE_NAME_MAX_SIZE);
  bkey.key_size = TABLE_NAME_MAX_SIZE;
  strncpy((char *)bkey.key, cdata->table_name, TABLE_NAME_MAX_SIZE);

  // check for table existence
  if (B_tree_Selector_by_Key(catalog, &bkey)){
    printf("Error: Table already exist\n");
    free(bkey.key);
    return false;
  }
  
  catalog_entry_mdata *catalog_entry = (catalog_entry_mdata *)calloc(1, sizeof(catalog_entry_mdata));
  strncpy(catalog_entry->table_name, cdata->table_name, COLUMN_NAME_MAX_SIZE);
  catalog_entry->record_table = NULL; // temporary placeholder
  catalog_entry->schema_table = NULL;
  
  // fill the col_list field
  for (i = 0; i < cdata->n_cols; i++){
    strncpy(catalog_entry->col_list[i],
            cdata->col_data[i].column_name,
            COLUMN_NAME_MAX_SIZE);
  }
  catalog_entry->col_list[cdata->n_cols][0] = '\0'; // this will mark as the end of this col_list array

  // need an array holdis key meta data
  static md_key mdkey_array2[] = {{SQL_STRING, COLUMN_NAME_MAX_SIZE}};
  B_tree *schema_table = (B_tree *)calloc(1, sizeof(B_tree));

  /* Implementation: create schema table
   * This table's entries has 2 fields:
   * * - key: <column_name>
   * * - Record: <pointer to schema_record of column name>
   * Definition: this table store metadata about each column the specified in
   * a`create table abc ()` query
   */
  B_tree_init(schema_table, tree_comp_fn, NULL, NULL,
              MAX_CHILD_PER_SCHEMA_TABLE, st_record_free, mdkey_array2,
              sizeof(mdkey_array2) / sizeof(md_key));
  Catalog_new_schema_table(schema_table, cdata);

  /* Implementation: create a record table
   * Actual recors from the `insert into` query will be stored in this table
   */
  B_tree *record_table = (B_tree *)calloc(1, sizeof(B_tree));
  int key_md_size3;
  md_key *mdkey_array3 = new_table_keymd(cdata, &key_md_size3);

  // A record table cannot have no primary key
  if (!mdkey_array3) {
    B_tree_Destroy(record_table);
    B_tree_Destroy(schema_table);
    printf("A table must have at least one primary key\n");
    free(record_table);
    free(schema_table);
    free(catalog_entry);
    free(bkey.key);
    return false;
  }

  B_tree_init(record_table, tree_comp_fn, NULL, NULL, MAX_CHILD_PER_RDBMS_TABLE,
              free, mdkey_array3, key_md_size3);
  catalog_entry->schema_table = schema_table;
  catalog_entry->record_table = record_table;

  B_tree_Insert(catalog, &bkey, (void *)catalog_entry);
  printf("TABLE CREATED\n");
  return true;

};

static void catalog_t_free_fn(void *ptr) {

  catalog_entry_mdata *ce_mdata = (catalog_entry_mdata *)ptr;

  B_tree_Destroy(ce_mdata->schema_table);
  free(ce_mdata->schema_table);
  B_tree_Destroy(ce_mdata->record_table);
  free(ce_mdata->record_table);
  free(ptr);
}
