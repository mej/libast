/*
 * Copyright (C) 1997-2003, Michael Jennings
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

#ifndef _LIBAST_AVL_TREE_H_
#define _LIBAST_AVL_TREE_H_

/*
 * interface goop
 */

/* Standard typecast macros.... */
#define SPIF_AVL_TREE_NODE(obj)                 (SPIF_CAST(avl_tree_node) (obj))
#define SPIF_AVL_TREE(obj)                      (SPIF_CAST(avl_tree) (obj))

#define SPIF_AVL_TREE_NODE_ISNULL(o)            (SPIF_AVL_TREE_NODE(o) == SPIF_NULL_TYPE(avl_tree_node))
#define SPIF_OBJ_IS_AVL_TREE_NODE(o)            (SPIF_OBJ_IS_TYPE((o), avl_tree_node))

SPIF_DEFINE_OBJ(avl_tree_node) {
    SPIF_DECL_PARENT_TYPE(nullobj);
    spif_obj_t data;
    spif_int8_t balance;
    spif_avl_tree_node_t left, right;
};

SPIF_DEFINE_OBJ(avl_tree) {
    SPIF_DECL_PARENT_TYPE(obj);
    size_t len;
    spif_avl_tree_node_t root;
};

extern spif_vectorclass_t SPIF_VECTORCLASS_VAR(avl_tree);
#endif /* _LIBAST_AVL_TREE_H_ */
