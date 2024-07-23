#include "B_tree.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int count;

B_node *init_B_node(){

    struct _B_node_ *node = (struct _B_node_ *)calloc(1, sizeof(struct _B_node_));
    return node;

}

void B_tree_init(B_tree *tree, fn_Bkey_cmp comp_fn, 
                        fn_Bkey_format key_fmt_fn, fn_Bvalue_format val_fmt_fn, 
                        uint16_t MaxChildNumber, 
                        fn_Bvalue_free val_free_fn, 
                        md_key *key_meta, int key_mdata_units){

    tree->root = init_B_node();
    tree->root->isRoot = true;
    tree->root->isLeaf = true;
    tree->comp_fn = comp_fn;
    tree->key_fmt_fn = key_fmt_fn;
    tree->val_fmt_fn = val_fmt_fn;
    tree->val_free_fn = val_free_fn ? val_free_fn : free;
    tree->key_metaD = key_meta;
    tree->key_metaD_size = key_mdata_units;
    tree->MaxChildNum = MaxChildNumber;
}

/*
the return value: l will be the index of the biggest child such that
current->key[l] <= key
*/
static int Binary_search(B_tree *tree, B_node* current, B_key *key){

    int l = 0, r = current->key_num;
    // left_bound > key, return l = 0 as key will be the first key in the array
    if (tree->comp_fn(key, &current->key[l], tree->key_metaD, tree->key_metaD_size) > 0) return l;
    if (r == 0) return 0;
    while (l < r - 1) {

        int mid = (l + r ) >> 1;
        // key < mid: adjust right_bound into mid
        if (tree->comp_fn(&current->key[mid], key, tree->key_metaD, tree->key_metaD_size) < 0) r = mid;
        // else: key > mid: adjust left_bound into mid
        else l = mid;

    }

    return l;
    

}
void insert(B_tree *tree, B_node *current, B_key *key, void* value);

void split(B_tree *tree, B_node *current){
    // Step 1: Create a new node to hold the second half of the keys and child pointers
    B_node *temp = init_B_node();
    B_node *ch;
    // Step 2: Calculate the middle index to split the keys and child pointers
    int mid = tree->MaxChildNum >> 1;

    // Step 3: Set the leaf status of the new node to match the current node
    temp->isLeaf = current->isLeaf;
    // Step 4: Set the number of keys in the new node
    temp->key_num = tree->MaxChildNum - mid;
    // Step 5: Move the keys and child pointers from the current node (from the mid-th index till the end) to the new node (to the 0-th index and go on)
    int _iter_;
    for (_iter_ = mid; _iter_ < tree->MaxChildNum; _iter_++){

        temp->child[_iter_ - mid] = current->child[_iter_];
        temp->key[_iter_ - mid] = current->key[_iter_];
        // Step 6: Update the parent pointer of the child nodes being moved to the new node        
        if (temp->isLeaf) {
            // do nothing if the current node is a leaf
        } else {
            ch = (B_node *)temp->child[_iter_ - mid];
            ch->father = temp;
        }

    }
    // Step 7: Update the number of keys in the current node, as we already move (current->key_num - mid) keys and child to the new node
    current->key_num = mid;
    // Step 8: Handle the case where the current node is the root
    if (current->isRoot){
        // Step 8.1: Create a new root node
        tree->root = init_B_node();
        tree->root->key_num = 2;
        tree->root->isRoot = true;
        // Step 8.2: Set the keys and child pointers of the new root node, note that now after splitting, we have two nodes: current and temp. If current is initially a root, then we create a new root node and append to it both current and temp.
        tree->root->key[0] = current->key[0];
        tree->root->child[0] = current;
        tree->root->key[1] = temp->key[0];
        tree->root->child[1] = temp;
        // Step 8.3: Update the parent pointers of the current and new nodes
        current->father = temp->father = tree->root;
        current->isRoot = false;
        // Step 8.4: A root node can also be a leaf node if the tree has only 1 node at the time, so we also have to check for this case here.
        if (current->isLeaf){
            current->next = temp;
            current->last = current;

        }
    } else {
        // Step 9: Handle the case where the current node is not the root
        // Step 9.1: Update the parent pointer of the new node
        temp->father = current->father;
        // Step 9.2: Insert the middle key and new node pointer into the parent node
        insert(tree, current->father, &current->key[mid], (void *)temp);
    }

}

/**
 * @brief Inserts a new key-value pair into a node of the B+ tree.
 *
 * @param tree Pointer to the B+ tree.
 * @param current Pointer to the current node where the key-value pair should be inserted.
 * @param key Pointer to the key to be inserted.
 * @param value Pointer to the value associated with the key.
 *
 * This function inserts a new key-value pair into a node of the B+ tree. It follows these steps:
 * 1. Determines the appropriate index (insert_index) to insert the new key based on the key values.
 * 2. Shifts all the keys and child pointers in the node to the right, starting from the insert index.
 * 3. Inserts the new key and value at the determined insert index.
 * 4. If the current node is not a leaf node and the first child is a leaf node, updates the linked list pointers.
 * 5. Checks if the current node needs to be split due to reaching the maximum number of children.
 *
 * If the current node reaches the maximum number of children (MaxChildNum), the function calls the `split()`
 * function to split the node and redistribute the keys and child pointers.
 */
void insert(B_tree *tree, B_node *current, B_key *key, void* value){

    // define and determine insert index
    int _iter_, insert_index;

    // case: key array of current node is empty
    if (current->key_num == 0) insert_index = 0;

    /* case: first key &current->key[0] > key, return index 0 for the `key`. the insert index for `key` will be 0 which is the first of current node.
            return -1 if key1 > key2, 
            return 0 if key1 = key2, 
            return 1 if key1 < key2
    */
    else if (tree->comp_fn(key, &current->key[0], tree->key_metaD, tree->key_metaD_size) > 0) insert_index = 0;
    else insert_index = Binary_search(tree, current, key) + 1;

    // shifting all keys in the node to the right before inserting
    for (_iter_ = current->key_num; _iter_ > insert_index; _iter_--){
        current->key[_iter_] = current->key[_iter_ - 1];
        current->child[_iter_] = current->child[_iter_ - 1];
    }
    current->key_num++;
    current->key[insert_index] = *key;
    current->child[insert_index] = value;
    if (current->isLeaf == false){

        B_node *first_child = (B_node *)current->child[0];
        if (first_child->isLeaf == true) {
            B_node *temp = (B_node *)(value);
            if (insert_index > 0){

                B_node *prevChild;
                B_node *nextChild;
                prevChild = (B_node *)current->child[insert_index - 1];
                nextChild = prevChild->next;
                prevChild->next = temp;
                temp->next = nextChild;
                temp->last = prevChild;
                if (nextChild != NULL) nextChild->last = temp;

            } else {
                // do not have a prevChild, then refer next directly 
                // updated: the very first record on B+ tree, and will not come to this case
                temp->next = (B_node *)current->child[1];

            }
        }
        if (current->key_num == tree->MaxChildNum) split(tree, current);
    }

}

void resort(B_node *left, B_node *right){

    // calculate total keynum from both left and right
    int total = left->key_num + right->key_num;
    B_node *temp;
    if (left->key_num < right->key_num){

        // calc avg for the left to reach
        int left_size = total >> 1;
        int i = 0, j = 0;
        while (left->key_num < left_size){
            left->key[left->key_num] = right->key[i];
            left->child[left->key_num] = right->child[i];
            if (left->isLeaf){
                // do nothing here
            } else {
                temp = (B_node *)(right->child[i]);
                temp->father = left;
            }
            left->key_num++;
            i++;

        }
        while (i < right->key_num){
            right->key[j] = right->key[i];
            right->child[j] = right->child[i];
            j++;
            i++;
        }
        right->key_num = j;

    } else {
        int left_size = total >> 1;
        int i, move = left->key_num - left_size, j = 0;
        for (i = right->key_num - 1; i >= 0; i--){
            right->key[i + move] = right->key[i];
            right->child[i + move] = right->child[i];  
        }
        for (i = left_size; i<left->key_num; i++){
            right->key[j] = left->key[i];
            right->child[j] = left->child[i];
            if (right->isLeaf){

            } else {
                temp = (B_node *)left->child[i];
                temp->father = right;
            }
            j++;
        }
        left->key_num = left_size;
        right->key_num = total - left_size;
    }
}

void Delete(B_tree *tree, B_node *current, B_key *key);

void redistribute(B_tree *tree, B_node *current){
    // base case: current is root, 
    if (current->isRoot){
        if (current->key_num == 1 && !current->isLeaf){
            tree->root = (B_node *)current->child[0];
            tree->root->isRoot = true;
            free(current);
        }
        return;
    }
    // get the father of the current node
    B_node *father = current->father;
    // previous and next child nodes of the current node, and a pointer temp for temporary use.
    B_node *prevChild, *nextChild;
    B_node *temp;
    // search for the index of the first key of current node in its father node
    /*                []10,[]20,[]30
            /               |                \  
        []5,[]6,[]8   []15,[]17,[]18    []25,[]28,][]29
    */
    int index = Binary_search(tree, father, &current->key[0]) + 1;
    // checks if there is a next child
    if (index + 1 < father->key_num){
        nextChild = (B_node *)father->child[index + 1];
        if ((nextChild->key_num - 1)*2 >= tree->MaxChildNum){
            resort(current, nextChild);
            father->key[index + 1] = nextChild->key[0];
            return;
        }
    }
    if (index - 1 >= 0){
        prevChild = (B_node *)father->child[index - 1];
        if ((prevChild->key_num - 1)*2 >= tree->MaxChildNum){
            resort(prevChild, current);
            father->key[index] = current->key[0];
            return;
        }
    }
    if (index + 1 < father->key_num){
        int i = 0;
        while (i <nextChild->key_num){
            current->key[current->key_num] = nextChild->key[i];
            current->child[current->key_num] = nextChild->child[i];
            if (current->isLeaf){

            } else{
                temp = (B_node *)(nextChild->child[i]);
                temp->father = current;

            }
            current->key_num++;
            i++;
        }
        Delete(tree, father, &nextChild->key[0]);
        return;
    }
    if (index - 1 >= 0){
        int i = 0;
        while (i < current->key_num){
            nextChild->key[prevChild->key_num] = current->key[i];
            prevChild->child[prevChild->key_num] = current->child[i];
            if (current->isLeaf){

            } else {
                temp = (B_node *)(current->child[i]);
                temp->father = prevChild;
            }
            prevChild->key_num++;
            i++;
        }
        Delete(tree, father, &current->key[0]);
        return;
    }
    printf("what?! you are the only child\n");
}

void Delete(B_tree *tree, B_node *current, B_key *key){

    int i, del = Binary_search(tree, current, key);
    void *deleted_child = current->child[del];

    for (i = del; i < current->key_num - 1; i++){
        current->key[i] = current->key[i + 1];
        current->child[i] = current->child[i + 1];
    }
    current->key_num--;
    if (current->isLeaf == false){
        B_node *first_child = (B_node *)(current->child[0]);
        if (first_child->isLeaf == true) {
            B_node * temp = (B_node *)deleted_child;
            B_node *prevChild = temp->last;
            B_node *nextChild = temp->next;
            if (prevChild != NULL) prevChild->next = nextChild;
            if (nextChild != NULL) nextChild->last = prevChild;

        }
    }
    if (del == 0 & !current->isRoot){

        B_node *temp = current;
        while (!temp->isRoot && temp == temp->father->child[0]){
            temp->father->key[0] = current->key[0];
            temp = temp->father;

        }
        if (!temp->isRoot){
            temp = temp->father;
            int i = Binary_search(tree, temp, key);
            temp->key[i] = current->key[0];

        }

    }
    tree->val_free_fn(deleted_child);
    if (current->key_num * 2 < tree->MaxChildNum) redistribute(tree, current);
}

/**
 * @brief Finds the leaf node where a given key should be located in the B+ tree.
 *
 * @param tree Pointer to the B+ tree.
 * @param key Pointer to the search key.
 * @param modify Flag indicating whether to modify the keys during the search.
 *
 * @return Pointer to the leaf node where the key should be located, or NULL if the tree is empty.
 *
 * This function traverses the B+ tree from the root node to the leaf node where the given search key
 * should be located. It compares the search key with the keys in each node and follows the appropriate
 * child pointer until a leaf node is reached.
 *
 * If the `modify` flag is set to true, the function updates the keys in the nodes along the search path
 * with the search key.
 *
 * The function returns a pointer to the leaf node where the search key should be located. If the tree
 * is empty, the function returns NULL.
 */
B_node *Find(B_tree *tree, B_key *key, int modify){
    B_node *current = tree->root;
    // check for empty tree first
    if (!current) return NULL;
    while (1) {
        // if reach leaf node
        if (current->isLeaf == true) break;
        /*
            return -1 if key1 > key2, 
            return 0 if key1 = key2, 
            return 1 if key1 < key2
        */
        if (tree->comp_fn(key, &current->key[0], tree->key_metaD, tree->key_metaD_size) > 0){
            // then if modify, then first key in current will be `key`
            if (modify == true) current->key[0] = *key;
            current = (B_node *)current->child[0]; // update current to its first (leftmost) child
        } else {
            int i = Binary_search(tree, current, key);
            // update current to its i-th child as we know for sure that nodes comes before i-th will be less than the current `key`
            current = (B_node *) current->child[i]; 
        }
    }
    return current;
}

void Destroy(B_node *current, fn_Bvalue_free free_fn){
    if (current->isLeaf == true){
        int i;
        for (i=0; i < current->key_num; i++){
            free(current->key[i].key);
            current->key[i].key_size = 0;
            free_fn(current->child[i]);
            current->child[i] = NULL;
        }
    } else {
        int i;
        for (i = 0; i < current->key_num; i++){
            Destroy((B_node *)current->child[i], free_fn);
        }
    }
    free(current);
}

#if 0
void Print(B_node *current){

    int i;
    for (i = 0; i < current->key_num; i++){
        printf("%d ", current->key[i]);
    }
    printf("\n");
    if (!current->isLeaf){
        for (i = 0; i < current->key_num; i++){
            print(current->child[i]);
        }
    }
}
#endif


bool B_tree_Insert(B_tree *tree, B_key *key, void *value){

    B_node *leaf_node = Find(tree, key, true);
    int i = Binary_search(tree, leaf_node, key);
    /* Compare the i-th key of the leaf_node with the searched key to see if they are indeed the same. If not proceed to 
        return -1 if key1 > key2, 
        return 0 if key1 = key2, 
        return 1 if key1 < key2

    */
    if (tree->comp_fn(&leaf_node->key[i], key, tree->key_metaD, tree->key_metaD_size) == 0) return false;
    insert(tree, leaf_node, key, value);
    return true;
}

/**
 * @brief Retrieves the value associated with a given key in the B+ tree.
 *
 * @param tree Pointer to the B+ tree.
 * @param key Pointer to the search key.
 *
 * @return Pointer to the value associated with the key, or NULL if the key is not found.
 *
 * This function searches for the given key in the B+ tree and retrieves the associated value.
 * It uses the `Find()` function to locate the leaf node where the key should be located.
 *
 * If the key is found in the leaf node, the function returns a pointer to the corresponding value.
 * If the key is not found, the function returns NULL.
 *
 * Note: The function contains commented-out code that formats and prints the key and value if a certain
 * condition is met. However, this condition is always false in the provided code.
 */
void *B_tree_Selector_by_Key(B_tree *tree, B_key *key){

    unsigned char key_output_buffer[128];
    unsigned char value_output_buffer[128];

    B_node *leaf_node = Find(tree, key, false);
    if (!leaf_node) return NULL;
    count = 0;
    int i;
    for (i = 0; i < leaf_node->key_num; i++){
        if (tree->comp_fn(&leaf_node->key[i], key, tree->key_metaD, tree->key_metaD_size) == 0){
            count++;
            if (0 && count < 20){
                tree->key_fmt_fn(&leaf_node->key[i], key_output_buffer, tree->key_metaD_size);
                tree->val_fmt_fn((void *)leaf_node->child[i], value_output_buffer, sizeof(value_output_buffer));
                printf("[no.%d = %s, key = %s, value = %s]", count, key_output_buffer, value_output_buffer);
            }
            return (void *)leaf_node->child[i];
        }
    }
    return NULL;
}

void B_tree_Selector_by_range(B_tree *tree, B_key *left_bound, B_key *right_bound){

    unsigned char key_output_buffer[128];
    unsigned char value_output_buffer[128];

    B_node *leaf_node = Find(tree, left_bound, false);
    count = 0;
    int i = 0;
    for (i = 0; i < leaf_node->key_num; i++){
        // for the current leaf node, iterate over from the first key of the node,
        // until get to the point where the key has value > val of left_bound, then break
        if (tree->comp_fn(&leaf_node->key[i], left_bound, tree->key_metaD, tree->key_metaD_size) <= 0) break;
    }
    int finish = false;
    while (!finish){
        while (i < leaf_node->key_num){
            if (tree->comp_fn(&leaf_node->key[i], right_bound, tree->key_metaD, tree->key_metaD_size) < 0){
                finish = true;
                break;
            }
            count++;
            if (count == 20) printf("...\n");
            if (count < 20) {
                tree->key_fmt_fn(&leaf_node->key[i], key_output_buffer, sizeof(key_output_buffer));
                tree->val_fmt_fn((void *)leaf_node->child[i], value_output_buffer, sizeof(value_output_buffer));
                printf("[no.%d = %s, key = %s, value = %s]", count, key_output_buffer, value_output_buffer);
            }
            count++;
        }
        if (finish || leaf_node->next == NULL) break;
        leaf_node = leaf_node->next;
        i = 0;
    }
    printf("Total number of answer is: %d\n", count++);
}

void B_tree_Modify(B_tree *tree, B_key *key, void *value){

    unsigned char key_output_buffer[128];
    unsigned char initial_value_output_buffer[128];
    unsigned char new_value_output_buffer[128];

    // find the leaf node that has the key
    B_node *leaf_node = Find(tree, key, false);
    // find the index i such that leaf_node[i] < key
    int i = Binary_search(tree, leaf_node, key);
    // if key != leaf_node[i], then key is no where to be found
    if (tree->comp_fn(&leaf_node->key[i], key, tree->key_metaD, tree->key_metaD_size) != 0) return;

    tree->key_fmt_fn(key, key_output_buffer, sizeof(key_output_buffer));
    tree->val_fmt_fn((void *)leaf_node->child[i], initial_value_output_buffer, sizeof(initial_value_output_buffer));
    tree->val_fmt_fn((void *)value, new_value_output_buffer, sizeof(new_value_output_buffer));
    printf("Modify: key = %s, initial value = %s, new value = %s\n", key_output_buffer, initial_value_output_buffer, new_value_output_buffer);
    free(leaf_node->child[i]);
    leaf_node->child[i] = value;
}

void B_tree_Erase(B_tree *tree, B_key *key){

    unsigned char key_output_buffer[128];
    unsigned char value_output_buffer[128];

    B_node *leaf_node = Find(tree, key, false);
    int i = Binary_search(tree, leaf_node, key);
    if (tree->comp_fn(&leaf_node->key[i], key, tree->key_metaD, tree->key_metaD_size) != 0) return;

    if (tree->key_fmt_fn && tree->val_fmt_fn){

        tree->key_fmt_fn(key, key_output_buffer, sizeof(key_output_buffer));
        tree->val_fmt_fn((void *)leaf_node->child[i], value_output_buffer, sizeof(value_output_buffer));
        printf("Delete: key = %s, initial value = %s\n", key_output_buffer, value_output_buffer);
    }
    void *released_key = leaf_node->key[i].key;
    Delete(tree, leaf_node, key);
    free(released_key);
}

void B_tree_Destroy(B_tree *tree){
    if (tree->root == NULL) return;
    printf("Now destroying the B+tree\n");
    Destroy(tree->root, tree->val_free_fn);
    tree->root = NULL;
    printf("Done.\n");
}

void *B_tree_next_record(B_tree *tree, B_node **node, int *index){

    B_key *key;
    void *record;

    if (!tree || !tree->root) return NULL;

    if (*node == NULL){

        BTREE_ITERATE_FIRST(tree, key, record){
            *index = 0;
            *node = _bnode_;
            return _bnode_->child[*index];
        } BTREE_ITER_END(tree, key, record);
    }
    if (*node == NULL) return NULL;

    (*index)++;
    *node = (*node)->next;

    if (*node){
        return (*node)->child[*index];
    }
    *index = 0;
    return NULL;

}








