#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <stdint.h>
#include "B_tree.h"

typedef enum _data_t_{

    TYPE_STRING,
    TYPE_INT,
    TYPE_DOUBLE,

} data_type;

/*
Note the key here is actually a combination of: string<varchar(32)>, int<4>, int<4>
An analogy would be: akey is a combo of name,age,salary
The function will first compare the field name, if equal (0) move to age, and so on.
if all 3 fields are the same (call to break the switch each time) then return 0
return values:
-1: key1 > key2
 0 : key1 = key2
 1 : key1 < key2

*/
static int tree_comp_fn(B_key *key1, B_key *key2, md_key *key_metad_array, int key_array_size){

    int result;
    int dtype_size;
    int offset;
    data_type dtype;
    int i;

    char *key1_ptr = (char *)key1->key;
    char *key2_ptr = (char *)key2->key;
    for (i = 0; i<key_array_size; i++){
        dtype = (data_type)key_metad_array[i].dtype;
        dtype_size = key_metad_array[i].size;

        // iterate over individual elements of a key of a B+tree
        switch(dtype){

            case TYPE_STRING:
                {
                    result = strncmp(key1_ptr + offset, key2_ptr + offset, dtype_size);
                    if (result < 0) return 1;
                    if (result > 0) return -1;
                    offset += dtype_size;
                }
                break;

            case TYPE_INT:
                {    int *number1 = (int *)key1_ptr + offset;
                    int *number2 = (int *)key2_ptr + offset;
                    if (*number1 < *number2) return 1;
                    if (*number1 > *number2) return -1;
                    offset += 4;
                }
                break;

            case TYPE_DOUBLE:
                {    
                    double *number1 = (double *)key1_ptr + offset;
                    double *number2 = (double *)key2_ptr + offset;
                    if (*number1 < *number2) return 1;
                    if (*number1 > *number2) return -1;
                    offset += 4;
                }
                break;
        }
    }
    return 0;
}

int main(int argc, char **argv){

    int choice;
    char disc_buffer[2]; // 
    B_tree tree;
    memset(&tree, 0, sizeof(B_tree));
    
    // array stores the structure of a key
    static md_key key_meta_d[] = { {TYPE_STRING, 32}, {TYPE_INT, 4}, {TYPE_INT, 4} };

    B_tree_init(&tree, tree_comp_fn, NULL, NULL, 4, NULL, key_meta_d, sizeof(key_meta_d)/sizeof(md_key));

    while (1){
        printf("1. [I]nsert\n");
        printf("2. [D]elete\n");
        printf("3. [U]pdate\n");
        printf("4. [R]ead\n");
        printf("5. [D]estroy\n");
        printf("6. [I]terate over all records\n");
        printf("7. [E]xit\n");
    
        scanf("%d", &choice);
        fgets((char *)disc_buffer, sizeof(disc_buffer), stdin);
        fflush(stdin);

        switch (choice)
        {
        case 1:
            // Inserting new node
            {
                B_key key;
                char *key_buffer = (char *)calloc(1, 32);
                key.key = (void *)key_buffer;
                key.key_size = 32;
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\n")] = '\0';
                char *value_buffer = (char *)calloc(1, 32);
                fgets(value_buffer, sizeof(value_buffer), stdin);
                B_tree_Insert(&tree, &key, (void *)value_buffer);

            }
            break;
        case 2:
            {
                
                B_key key;
                char key_buffer[32];
                key.key = (void *)key_buffer;
                key.key_size = 32;
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\n")] = '\0';
                B_tree_Erase(&tree, &key);

            }
            break;

        case 3:
            {
                B_key key;
                char key_buffer[32];
                key.key = (void *)key_buffer;
                key.key_size = 32;
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\n")] = '\0';

                char *initial_value_buffer = (char *)B_tree_Selector_by_Key(&tree, &key);

                if (!initial_value_buffer){
                    printf("Key not found!\n");
                    scanf("\n");
                    break;
                }

                printf("Initial record = %s\n", initial_value_buffer);
                char *new_value_buffer = (char *)calloc(1, 32);
                printf("Insert new value:\n");
                fgets(new_value_buffer, 32, stdin);
                new_value_buffer[strcspn(new_value_buffer, "\n")] = '\0';
                B_tree_Modify(&tree, &key, (void *)new_value_buffer);
            }
            break;
        case 4:
            
            {
                B_key key;
                char key_buffer[32];
                key.key = (void *)key_buffer;
                key.key_size = 32;
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\n")] = '\0';

                char *value_buffer = (char *)B_tree_Selector_by_Key(&tree, &key);

                if (!value_buffer){
                    printf("Key not found!\n");
                    scanf("\n");
                    break;
                }

                printf("Value = %s\n", value_buffer);
                
            }
            break;
        
        case 5:
            {
                B_tree_Destroy(&tree);
                printf("Successfully destroy the B+tree\n");
            }
            break;

        case 6:
            {
                B_key *key;
                void *record;

                BTREE_ITERATE_FIRST((&tree), key, record){
                    printf("Key = %s, value = %s\n", (char *)key->key, (char *)record);
                } BTREE_ITER_END(&tree, key, record);

            }
            break;
        }
    }
    return 0;
}