/*
 * Copyright (C) 1997-2004, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

static const char cvs_ident[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libast_internal.h>

#define LEFT_HEAVY    ((spif_int8_t) (-1))
#define BALANCED      ((spif_int8_t) (0))
#define RIGHT_HEAVY   ((spif_int8_t) (1))

static spif_avl_tree_node_t spif_avl_tree_node_new(void);
static spif_bool_t spif_avl_tree_node_init(spif_avl_tree_node_t);
static spif_bool_t spif_avl_tree_node_done(spif_avl_tree_node_t);
static spif_bool_t spif_avl_tree_node_del(spif_avl_tree_node_t);
static spif_str_t spif_avl_tree_node_show(spif_avl_tree_node_t, spif_charptr_t, spif_str_t, size_t);
static spif_cmp_t spif_avl_tree_node_comp(spif_avl_tree_node_t, spif_avl_tree_node_t);
static spif_avl_tree_node_t spif_avl_tree_node_dup(spif_avl_tree_node_t);
static spif_classname_t spif_avl_tree_node_type(spif_avl_tree_node_t);
static spif_obj_t spif_avl_tree_node_get_data(spif_avl_tree_node_t);
static spif_bool_t spif_avl_tree_node_set_data(spif_avl_tree_node_t, spif_obj_t);

static spif_avl_tree_t spif_avl_tree_new(void);
static spif_bool_t spif_avl_tree_init(spif_avl_tree_t);
static spif_bool_t spif_avl_tree_done(spif_avl_tree_t);
static spif_bool_t spif_avl_tree_del(spif_avl_tree_t);
static spif_str_t spif_avl_tree_show(spif_avl_tree_t, spif_charptr_t, spif_str_t, size_t);
static spif_cmp_t spif_avl_tree_comp(spif_avl_tree_t, spif_avl_tree_t);
static spif_avl_tree_t spif_avl_tree_dup(spif_avl_tree_t);
static spif_classname_t spif_avl_tree_type(spif_avl_tree_t);
static spif_bool_t spif_avl_tree_contains(spif_avl_tree_t, spif_obj_t);
static size_t spif_avl_tree_count(spif_avl_tree_t);
static spif_obj_t spif_avl_tree_find(spif_avl_tree_t, spif_obj_t);
static spif_bool_t spif_avl_tree_insert(spif_avl_tree_t, spif_obj_t);
static spif_bool_t spif_avl_tree_iterator(spif_avl_tree_t);
static spif_obj_t spif_avl_tree_next(spif_avl_tree_t);
static spif_obj_t spif_avl_tree_remove(spif_avl_tree_t, spif_obj_t);
static spif_obj_t *spif_avl_tree_to_array(spif_avl_tree_t);

static spif_avl_tree_node_t insert_node(spif_avl_tree_node_t, spif_avl_tree_node_t, spif_uint8_t *);
static spif_avl_tree_node_t left_balance(spif_avl_tree_node_t);
static spif_avl_tree_node_t right_balance(spif_avl_tree_node_t);
static spif_avl_tree_node_t rotate_left(spif_avl_tree_node_t);
static spif_avl_tree_node_t rotate_right(spif_avl_tree_node_t);

/* *INDENT-OFF* */
static SPIF_CONST_TYPE(class) atn_class = {
    SPIF_DECL_CLASSNAME(avl_tree_node),
    (spif_func_t) spif_avl_tree_node_new,
    (spif_func_t) spif_avl_tree_node_init,
    (spif_func_t) spif_avl_tree_node_done,
    (spif_func_t) spif_avl_tree_node_del,
    (spif_func_t) spif_avl_tree_node_show,
    (spif_func_t) spif_avl_tree_node_comp,
    (spif_func_t) spif_avl_tree_node_dup,
    (spif_func_t) spif_avl_tree_node_type
};
SPIF_TYPE(class) SPIF_CLASS_VAR(avl_tree_node) = &atn_class;

static spif_const_vectorclass_t at_class = {
    {
        SPIF_DECL_CLASSNAME(avl_tree),
        (spif_func_t) spif_avl_tree_new,
        (spif_func_t) spif_avl_tree_init,
        (spif_func_t) spif_avl_tree_done,
        (spif_func_t) spif_avl_tree_del,
        (spif_func_t) spif_avl_tree_show,
        (spif_func_t) spif_avl_tree_comp,
        (spif_func_t) spif_avl_tree_dup,
        (spif_func_t) spif_avl_tree_type
    },
    (spif_func_t) spif_avl_tree_contains,
    (spif_func_t) spif_avl_tree_count,
    (spif_func_t) spif_avl_tree_find,
    (spif_func_t) spif_avl_tree_insert,
    (spif_func_t) spif_avl_tree_iterator,
    (spif_func_t) spif_avl_tree_next,
    (spif_func_t) spif_avl_tree_remove,
    (spif_func_t) spif_avl_tree_to_array
};
SPIF_TYPE(vectorclass) SPIF_VECTORCLASS_VAR(avl_tree) = &at_class;
/* *INDENT-ON* */

static spif_avl_tree_node_t
spif_avl_tree_node_new(void)
{
    spif_avl_tree_node_t self;

    self = SPIF_ALLOC(avl_tree_node);
    spif_avl_tree_node_init(self);
    return self;
}

static spif_bool_t
spif_avl_tree_node_init(spif_avl_tree_node_t self)
{
    self->data = SPIF_NULL_TYPE(obj);
    self->balance = BALANCED;
    self->left = self->right = SPIF_NULL_TYPE(avl_tree_node);
    return TRUE;
}

static spif_bool_t
spif_avl_tree_node_done(spif_avl_tree_node_t self)
{
    if (self->data != SPIF_NULL_TYPE(obj)) {
        SPIF_OBJ_DEL(self->data);
        self->data = SPIF_NULL_TYPE(obj);
    }
    if (self->left != SPIF_NULL_TYPE(avl_tree_node)) {
        spif_avl_tree_node_done(self->left);
        self->left = SPIF_NULL_TYPE(avl_tree_node);
    }
    if (self->right != SPIF_NULL_TYPE(avl_tree_node)) {
        spif_avl_tree_node_done(self->right);
        self->right = SPIF_NULL_TYPE(avl_tree_node);
    }
    self->balance = BALANCED;
    return TRUE;
}

static spif_bool_t
spif_avl_tree_node_del(spif_avl_tree_node_t self)
{
    spif_avl_tree_node_done(self);
    SPIF_DEALLOC(self);
    return TRUE;
}

static spif_str_t
spif_avl_tree_node_show(spif_avl_tree_node_t self, spif_charptr_t name, spif_str_t buff, size_t indent)
{
    char tmp[4096];

    memset(tmp, ' ', indent);
    snprintf(tmp + indent, sizeof(tmp) - indent, "(spif_avl_tree_node_t) %s:  %010p {\n",
             name, self);
    if (SPIF_STR_ISNULL(buff)) {
        buff = spif_str_new_from_ptr(tmp);
    } else {
        spif_str_append_from_ptr(buff, tmp);
    }
    if (SPIF_OBJ_ISNULL(self->data)) {
        spif_str_append_from_ptr(buff, SPIF_NULLSTR_TYPE(obj));
    } else {
        buff = SPIF_OBJ_SHOW(self->data, buff, 0);
    }
    memset(tmp, ' ', indent + 2);
    snprintf(tmp + indent + 2, sizeof(tmp) - indent - 2, "(spif_int8_t) balance:  %s (%d)\n",
             ((self->balance == LEFT_HEAVY)
              ? ("LEFT_HEAVY")
              : ((self->balance == RIGHT_HEAVY)
                 ? ("RIGHT_HEAVY")
                 : ((self->balance == BALANCED)
                    ? ("BALANCED")
                    : ("UNKNOWN")))), (int) self->balance);
    spif_str_append_from_ptr(buff, tmp);

    if (!SPIF_AVL_TREE_NODE_ISNULL(self->left)) {
        buff = spif_avl_tree_node_show(self->left, "left", buff, indent + 2);
    }
    if (!SPIF_AVL_TREE_NODE_ISNULL(self->right)) {
        buff = spif_avl_tree_node_show(self->right, "right", buff, indent + 2);
    }
    memset(tmp, ' ', indent);
    snprintf(tmp + indent, sizeof(tmp) - indent, "}\n");
    spif_str_append_from_ptr(buff, tmp);
    return buff;
}

static spif_cmp_t
spif_avl_tree_node_comp(spif_avl_tree_node_t self, spif_avl_tree_node_t other)
{
    return (SPIF_CAST(cmp) SPIF_OBJ_COMP(SPIF_OBJ(self->data), SPIF_OBJ(other->data)));
}

static spif_avl_tree_node_t
spif_avl_tree_node_dup(spif_avl_tree_node_t self)
{
    spif_avl_tree_node_t tmp;

    tmp = spif_avl_tree_node_new();
    tmp->data = SPIF_OBJ_DUP(self->data);
    tmp->balance = self->balance;
    if (!SPIF_AVL_TREE_NODE_ISNULL(self->left)) {
        tmp->left = spif_avl_tree_node_dup(self->left);
    }
    if (!SPIF_AVL_TREE_NODE_ISNULL(self->right)) {
        tmp->right = spif_avl_tree_node_dup(self->right);
    }
    return tmp;
}

static spif_classname_t
spif_avl_tree_node_type(spif_avl_tree_node_t self)
{
    USE_VAR(self);
    return SPIF_CLASS_VAR(avl_tree_node)->classname;
}

static spif_obj_t
spif_avl_tree_node_get_data(spif_avl_tree_node_t self)
{
    return SPIF_OBJ(self->data);
}

static spif_bool_t
spif_avl_tree_node_set_data(spif_avl_tree_node_t self, spif_obj_t obj)
{
    self->data = obj;
    return TRUE;
}

static spif_avl_tree_t
spif_avl_tree_new(void)
{
    spif_avl_tree_t self;

    self = SPIF_ALLOC(avl_tree);
    spif_avl_tree_init(self);
    return self;
}

static spif_bool_t
spif_avl_tree_init(spif_avl_tree_t self)
{
    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS(SPIF_VECTORCLASS_VAR(avl_tree)));
    self->len = 0;
    self->root = SPIF_NULL_TYPE(avl_tree_node);
    return TRUE;
}

static spif_bool_t
spif_avl_tree_done(spif_avl_tree_t self)
{
    if (self->len) {
        spif_avl_tree_node_del(self->root);
        self->root = SPIF_NULL_TYPE(avl_tree_node);
        self->len = 0;
    }
    return TRUE;
}

static spif_bool_t
spif_avl_tree_del(spif_avl_tree_t self)
{
    spif_avl_tree_done(self);
    SPIF_DEALLOC(self);
    return TRUE;
}

static spif_str_t
spif_avl_tree_show(spif_avl_tree_t self, spif_charptr_t name, spif_str_t buff, size_t indent)
{
    char tmp[4096];

    memset(tmp, ' ', indent);
    snprintf(tmp + indent, sizeof(tmp) - indent, "(spif_avl_tree_t) %s:  {\n", name);
    if (SPIF_STR_ISNULL(buff)) {
        buff = spif_str_new_from_ptr(tmp);
    } else {
        spif_str_append_from_ptr(buff, tmp);
    }

    snprintf(tmp + indent, sizeof(tmp) - indent, "  len:  %lu\n", SPIF_CAST_C(unsigned long) self->len);
    spif_str_append_from_ptr(buff, tmp);

    if (SPIF_AVL_TREE_NODE_ISNULL(self->root)) {
        spif_str_append_from_ptr(buff, SPIF_NULLSTR_TYPE(obj));
    } else {
        buff = spif_avl_tree_node_show(self->root, "root", buff, indent + 2);
    }

    memset(tmp, ' ', indent);
    snprintf(tmp + indent, sizeof(tmp) - indent, "}\n");
    spif_str_append_from_ptr(buff, tmp);
    return buff;
}

static spif_cmp_t
spif_avl_tree_comp(spif_avl_tree_t self, spif_avl_tree_t other)
{
    return (SPIF_OBJ_COMP(SPIF_OBJ(self), SPIF_OBJ(other)));
}

static spif_avl_tree_t
spif_avl_tree_dup(spif_avl_tree_t self)
{
    spif_avl_tree_t tmp;

    tmp = spif_avl_tree_new();
    tmp->root = spif_avl_tree_node_dup(self->root);
    tmp->len = self->len;
    return tmp;
}

static spif_classname_t
spif_avl_tree_type(spif_avl_tree_t self)
{
    return SPIF_OBJ_CLASSNAME(self);
}

static spif_bool_t
spif_avl_tree_contains(spif_avl_tree_t self, spif_obj_t obj)
{
    return ((SPIF_LIST_ISNULL(spif_avl_tree_find(self, obj))) ? (FALSE) : (TRUE));
}

static size_t
spif_avl_tree_count(spif_avl_tree_t self)
{
    return self->len;
}

static spif_obj_t
spif_avl_tree_find(spif_avl_tree_t self, spif_obj_t obj)
{
    spif_avl_tree_node_t current;

    for (current = self->root; current; ) {
        spif_cmp_t cmp;

        cmp = SPIF_OBJ_COMP(current->data, obj);
        if (SPIF_CMP_IS_EQUAL(cmp)) {
            return current->data;
        } else if (SPIF_CMP_IS_LESS(cmp)) {
            current = current->right;
        } else {
            current = current->left;
        }
    }
    return SPIF_NULL_TYPE(obj);
}

static spif_bool_t
spif_avl_tree_insert(spif_avl_tree_t self, spif_obj_t obj)
{
    spif_avl_tree_node_t item;

    item = spif_avl_tree_node_new();
    spif_avl_tree_node_set_data(item, obj);

    if (SPIF_AVL_TREE_NODE_ISNULL(self->root)) {
        self->root = item;
    } else {
        spif_uint8_t taller;

        insert_node(self->root, item, &taller);
    }
    self->len++;
    return TRUE;
}

static spif_bool_t
spif_avl_tree_iterator(spif_avl_tree_t self)
{
    USE_VAR(self);
    return FALSE;
}

static spif_obj_t
spif_avl_tree_next(spif_avl_tree_t self)
{
    USE_VAR(self);
    return SPIF_NULL_TYPE(obj);
}

static spif_obj_t
spif_avl_tree_remove(spif_avl_tree_t self, spif_obj_t item)
{
    spif_avl_tree_node_t current, tmp;

    if (SPIF_AVL_TREE_NODE_ISNULL(self->root)) {
        return SPIF_NULL_TYPE(obj);
    } else if (SPIF_CMP_IS_EQUAL(SPIF_OBJ_COMP(item, self->root->data))) {
        tmp = self->root;
        self->root = self->root->next;
    } else {
        for (current = self->root; current->next && !SPIF_CMP_IS_EQUAL(SPIF_OBJ_COMP(item, current->next->data)); current = current->next);
        if (current->next) {
            tmp = current->next;
            current->next = current->next->next;
        } else {
            return SPIF_NULL_TYPE(obj);
        }
    }
    item = tmp->data;
    tmp->data = SPIF_NULL_TYPE(obj);
    spif_avl_tree_node_del(tmp);

    self->len--;
    return item;
}

static spif_obj_t *
spif_avl_tree_to_array(spif_avl_tree_t self)
{
    spif_obj_t *tmp;
    spif_avl_tree_node_t current;
    size_t i;

    tmp = SPIF_CAST_PTR(obj) MALLOC(SPIF_SIZEOF_TYPE(obj) * self->len);
    for (i = 0, current = self->root; i < self->len; current = current->next, i++) {
        tmp[i] = SPIF_CAST(obj) SPIF_OBJ(spif_avl_tree_node_get_data(current));
    }
    return tmp;
}

/**********************************************************************/

static spif_avl_tree_node_t
insert_node(spif_avl_tree_node_t root, spif_avl_tree_node_t node, spif_uint8_t *taller)
{
    int taller_subnode = 0;
    spif_cmp_t diff;

    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(root), SPIF_NULL_TYPE(avl_tree_node));
    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node), root);

    diff = SPIF_OBJ_COMP(node->data, root->data);
    if (SPIF_CMP_IS_LESS(diff)) {
        /* node needs to go in the left subtree of root. */
        if (SPIF_AVL_TREE_NODE_ISNULL(root->left)) {
            /* Nothing there.  Make this the new left child of root. */
            root->left = node;
            /* Update balance factor. */
            switch (root->balance) {
            case RIGHT_HEAVY:
                root->balance = BALANCED;
                *taller = 0;
                break;
            case BALANCED:
                root->balance = LEFT_HEAVY;
                *taller = 1;
                break;
            }
        } else {
            /* We already have a left child, so insert it under there. */
            root->left = insert_node(root->left, node, &taller_subtree);

            /* If the subtree is now taller, we need to rebalance. */
            if (taller_subtree == 1) {
                switch (root->balance) {
                case LEFT_HEAVY:
                    root = left_balance(root);
                    *taller = 0;
                    break;
                case RIGHT_HEAVY:
                    root->balance = BALANCED;
                    *taller = 0;
                    break;
                case BALANCED:
                    root->balance = LEFT_HEAVY;
                    *taller = 1;
                    break;
                }
            } else {
                *taller = 0;
            }
        }
    } else if (SPIF_CMP_IS_GREATER(diff)) {
        /* node needs to go in the right subtree of root. */
        if (SPIF_AVL_TREE_NODE_ISNULL(root->right)) {
            /* Nothing there.  Make this the new right child of root. */
            root->right = node;
            /* Update balance factor. */
            switch (root->balance) {
            case LEFT_HEAVY:
                root->balance = BALANCED;
                *taller = 0;
                break;
            case BALANCED:
                root->balance = RIGHT_HEAVY;
                *taller = 1;
                break;
            }
        } else {
            /* We already have a right child, so insert it under there. */
            root->right = insert_node(root->right, node, &taller_subtree);

            /* If the subtree is now taller, we need to rebalance. */
            if (taller_subtree == 1) {
                switch (root->balance) {
                case LEFT_HEAVY:
                    root->balance = BALANCED;
                    *taller = 0;
                    break;
                case RIGHT_HEAVY:
                    root = right_balance(root);
                    *taller = 0;
                    break;
                case BALANCED:
                    root->balance = RIGHT_HEAVY;
                    *taller = 1;
                    break;
                }
            } else {
                *taller = 0;
            }
        }
    } else {
        /* The node in question is equal to the current node, so update the current node's data. */
        if (!SPIF_OBJ_ISNULL(root->data)) {
            SPIF_OBJ_DEL(root->data);
        }
        root->data = node->data;
    }
    return root;
}

static spif_avl_tree_node_t
left_balance(spif_avl_tree_node_t root)
{
    spif_avl_tree_node_t node, subnode;

    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(root), SPIF_NULL_TYPE(avl_tree_node));

    node = root->left;
    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node), root);

    subnode = SPIF_NULL_TYPE(avl_tree_node);

    switch (node->balance) {
    case LEFT_HEAVY:
        root->balance = node->balance = BALANCED;
        root = rotate_right(root);
        break;
    case BALANCED:
        ASSERT_NOTREACHED_RVAL(root);
        break;
    case RIGHT_HEAVY:
        subnode = node->right;
        ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(subnode), root);

        switch (subnode->balance) {
        case BALANCED:
            root->balance = node->balance = BALANCED;
            break;
        case LEFT_HEAVY:
            root->balance = RIGHT_HEAVY;
            node->balance = BALANCED;
            break;
        case RIGHT_HEAVY:
            root->balance = BALANCED;
            node->balance = LEFT_HEAVY;
            break;
        }
        subnode->balance = BALANCED;
        root->left = rotate_left(node);
        root = rotate_right(root);
        break;
    }
    return root;
}

static spif_avl_tree_node_t
right_balance(spif_avl_tree_node_t root)
{
    spif_avl_tree_node_t node, subnode;

    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(root), SPIF_NULL_TYPE(avl_tree_node));

    node = root->right;
    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node), root);

    subnode = SPIF_NULL_TYPE(avl_tree_node);

    switch (node->balance) {
    case RIGHT_HEAVY:
        root->balance = node->balance = BALANCED;
        root = rotate_left(root);
        break;
    case BALANCED:
        ASSERT_NOTREACHED_RVAL(root);
        break;
    case LEFT_HEAVY:
        subnode = node->left;
        ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(subnode), root);

        switch (subnode->balance) {
        case BALANCED:
            root->balance = node->balance = BALANCED;
            break;
        case LEFT_HEAVY:
            root->balance = BALANCED;
            node->balance = RIGHT_HEAVY;
            break;
        case RIGHT_HEAVY:
            root->balance = LEFT_HEAVY;
            node->balance = BALANCED;
            break;
        }
        subnode->balance = BALANCED;
        root->right = rotate_right(node);
        root = rotate_left(root);
        break;
    }
    return root;
}

static spif_avl_tree_node_t
rotate_left(spif_avl_tree_node_t node)
{
    spif_avl_tree_node_t right_child;

    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node), SPIF_NULL_TYPE(avl_tree_node));
    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node->right), node);

    right_child = node->right;
    node->right = right_child->left;
    right_child->left = node;

    return right_child;
}

static spif_avl_tree_node_t
rotate_right(spif_avl_tree_node_t node)
{
    spif_avl_tree_node_t left_child;

    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node), SPIF_NULL_TYPE(avl_tree_node));
    ASSERT_RVAL(!SPIF_AVL_TREE_NODE_ISNULL(node->left), node);

    left_child = node->left;
    node->left = left_child->right;
    left_child->right = node;

    return left_child;
}
