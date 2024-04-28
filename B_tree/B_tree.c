#include "B_tree.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

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

    B_node *temp = init_B_node();
    B_node *ch;
    int mid = tree->MaxChildNum >> 1;
    temp->isLeaf = current->isLeaf;
    temp->key_num = tree->MaxChildNum - mid;
    int _iter_;
    for (_iter_ = 0; _iter_ < tree->MaxChildNum; _iter_++){

        temp->child[_iter_ - mid] = current->child[_iter_];
        temp->key[_iter_ - mid] = current->key[_iter_];
        if (temp->isLeaf) {
            // do nothing
        } else {
            ch = (B_node *)temp->child[_iter_ - mid];
            ch->father = temp;
        }

    }

    current->key_num = mid;
    if (current->isRoot){
        tree->root = init_B_node();
        tree->root->key_num = 2;
        tree->root->isRoot = true;
        tree->root->key[0] = current->key[0];
        tree->root->child[0] = current;
        tree->root->key[1] = temp->key[0];
        tree->root->child[1] = temp;
        current->father = temp->father = tree->root;
        current->isRoot = false;
        if (current->isLeaf){
            current->next = temp;
            current->last = current;

        }
    } else {

        temp->father = current->father;
        insert(tree, current->father, &current->key[mid], (void *)temp);
    }

}


void insert(B_tree *tree, B_node *current, B_key *key, void* value){

    // define and determine insert index
    int _iter_, insert_index;
    // case: key array of current node is empty
    if (current->key_num == 0) insert_index = 0;
    // case: first key &current->key[0] > key, return index 0 for the `key`
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
                temp->next = current->child[1];

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

void delete(B_tree *tree, B_node *current, B_key *key){
}

void redistribute(B_tree *tree, B_node *current){
    // base case: current is root, 
    if (current->isRoot){
        if (current->key_num == 1 && !current->isLeaf){
            tree->root = current->child[0];
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
        nextChild = father->child[index + 1];
        if ((nextChild->key_num - 1)*2 >= tree->MaxChildNum){
            resort(current, nextChild);
            father->key[index + 1] = nextChild->key[0];
            return;
        }
    }
    if (index - 1 >= 0){
        prevChild = father->child[index - 1];
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
        delete(tree, father, &nextChild->key[0]);
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
        delete(tree, father, &current->key[0]);
        return;
    }
    printf("what?! you are the only child\n");
}

void delete(B_tree *tree, B_node *current, B_key *key){

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
    tree->free_fn(deleted_child);
    if (current->key_num * 2 < tree->MaxChildNum) redistribute(tree, current);
}