
#include "../B_tree/B_tree.h"
#include "../Parser/Sql_enums.h"
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -1: key1 > key2
// 0 : key1 = key2
// 1 : key1 < key2
static int tree_comp_fn(B_key *key1, B_key *key2, md_key *key_meta_d,
                        int key_md_size) {

  int result;
  int dtype_size;
  int offset;
  sql_dtpye dtype;
  int i;
  if (!key1 || !key1->key || !key1->key_size)
    return 1;
  if (!key2 || !key2->key || !key2->key_size)
    return -1;

  char *key1_ptr = (char *)key1->key;
  char *key2_ptr = (char *)key2->key;
  for (i = 0; i < key_md_size; i++) {
    dtype = (sql_dtpye)key_meta_d[i].dtype;
    dtype_size = key_meta_d[i].size;

    switch (dtype) {

    case SQL_STRING: {
      result = strncmp(key1_ptr + offset, key2_ptr + offset, dtype_size);
      if (result < 0)
        return 1;
      if (result > 0)
        return -1;
      offset += dtype_size;
    } break;

    case SQL_INT: {
      int *number1 = (int *)key1_ptr + offset;
      int *number2 = (int *)key2_ptr + offset;
      if (*number1 < *number2)
        return 1;
      if (*number1 > *number2)
        return -1;
      offset += 4;
    } break;
    case SQL_DOUBLE: {
      double *number1 = (double *)key1_ptr + offset;
      double *number2 = (double *)key2_ptr + offset;
      if (*number1 < *number2)
        return 1;
      if (*number1 > *number2)
        return -1;
      offset += 4;
    } break;
    }
  }
  return 0;
}
