#ifndef _B_TREE_H_
#define _B_TREE_H_

#define MAX_CHILD_NUMBER 4
#include <stdint.h>
#include <stdbool.h>

/*

Internal nodes contain only keys, no data
Internal nodes with N keys has N + 1 children
Leaf nodes contain keys and pointer to data
Leaf nodes with N keys contains N records or N pointers to the actual data (1 for each key)
Leaf nodes are connected via linked list
*/

typedef struct _meta_data_key_ {

    int dtype;
    int size;

} md_key;


typedef struct _B_key_ {

    uint16_t key_size;
    void *key;

} B_key;

typedef struct _B_node_ {

    bool isRoot, isLeaf;
    int key_num;
    B_key key[MAX_CHILD_NUMBER + 1];
    void *child[MAX_CHILD_NUMBER + 2];
    struct _B_node_ *father;
    struct _B_node_ *next;
    struct _B_node_ *last;

} B_node;

typedef int (*fn_Bkey_cmp)(B_key *, B_key *, md_key *, int);
typedef int (*fn_Bkey_format)(B_key *, unsigned char *, int);
typedef int (*fn_Bvalue_format)(void *, unsigned char *, int);
typedef void (*fn_Bvalue_free)(void *);

typedef struct _B_tree_ {

    fn_Bkey_cmp comp_fn;
    fn_Bkey_format key_fmt_fn;
    fn_Bvalue_format val_fmt_fn;
    B_node *root;
    uint16_t MaxChildNum;
    fn_Bvalue_free val_free_fn;
    md_key *key_metaD;
    int key_metaD_size;

} B_tree;

extern void B_tree_init(B_tree *, fn_Bkey_cmp, 
                        fn_Bkey_format, fn_Bvalue_format, 
                        uint16_t MaxChildNumber, 
                        fn_Bvalue_free val_free_fn, 
                        md_key *key_meta, int key_mdata_units);

extern void B_tree_SetMaxChildNumber(B_tree *, int);
extern void B_tree_Destroy(B_tree *);
extern void B_tree_Insert(B_tree *, B_key *, void *);
extern void B_tree_Query_by_Key(B_tree *tree, B_key *key);
extern void B_tree_Query_range(B_tree *, B_key *, B_key *);
extern bool B_tree_Modify(B_tree *, B_key *key, void *value);
extern bool B_tree_Erase(B_tree *, B_key *);
void *B_tree_next_record(B_tree *, B_node *, int *, B_key *);

#define BTREE_ITERATE_FIRST(B_tree_ptr, key_ptr, record_ptr)  \
        {   \
            int _iter_;  \
            bool brk_switch = false;    \
            B_node *_bnode_ = B_tree_ptr->root; \
            key_ptr = NULL;     \
            record_ptr = NULL;  \
            while (_bnode_ && !_bnode_->isLeaf) {   \
                _bnode_ = (B_node *)_bnode_->child[0];  \
            }   \
            while (_bnode_){    \
                for (_iter_=0; _iter_<_bnode_->key_num; _iter_++){  \
                    key_ptr = &_bnode_->key[_iter_];    \
                    record_ptr = _bnode_->child[_iter_];    \

#define BTREE_ITER_BRK  \
    brk_ctrl = true;    \
    break;  \

#define BTREE_ITER_CONT \
    continue
#define BTREE_ITER_END(B_tree_ptr, key_ptr, record_ptr) \
                }   \
                if (brk_ctrl) break;    \
                _bnode_ = _bnode_->next;    \
            }}



#endif // _B_TREE_H_
