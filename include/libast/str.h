/*
 * Copyright (C) 1997-2002, Michael Jennings
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

#ifndef _LIBAST_STR_H_
#define _LIBAST_STR_H_

/* Cast an arbitrary object pointer to a str. */
#define SPIF_STR(obj)                ((spif_str_t) (obj))
#define SPIF_STR_STR(obj)            ((const spif_charptr_t) (SPIF_STR(obj)->s))
#define SPIF_STR_SHOW(obj)           (spif_str_show((obj), #obj))

/* Check to see if a pointer references a string object. */
#define SPIF_OBJ_IS_STR(obj)         (SPIF_OBJ_IS_TYPE(obj, str))

/* Types for the string object. */
typedef struct spif_str_t_struct *spif_str_t;
typedef struct spif_str_t_struct spif_const_str_t;

/* An str object stores a string (obviously) */
struct spif_str_t_struct {
  spif_const_obj_t parent;
  spif_charptr_t s;
  size_t mem, len;
};

extern spif_const_class_t SPIF_CLASS_VAR(str);
extern spif_str_t spif_str_new(void);
extern spif_str_t spif_str_new_from_ptr(spif_charptr_t);
extern spif_str_t spif_str_new_from_buff(spif_charptr_t, size_t);
extern spif_str_t spif_str_new_from_fp(FILE *);
extern spif_str_t spif_str_new_from_fd(int);
extern spif_bool_t spif_str_del(spif_str_t);
extern spif_bool_t spif_str_init(spif_str_t);
extern spif_bool_t spif_str_init_from_ptr(spif_str_t, spif_charptr_t);
extern spif_bool_t spif_str_init_from_buff(spif_str_t, spif_charptr_t, size_t);
extern spif_bool_t spif_str_init_from_fp(spif_str_t, FILE *);
extern spif_bool_t spif_str_init_from_fd(spif_str_t, int);
extern spif_bool_t spif_str_done(spif_str_t);
extern spif_str_t spif_str_dup(spif_str_t);
extern int spif_str_cmp(spif_str_t, spif_str_t);
extern int spif_str_cmp_with_ptr(spif_str_t, spif_charptr_t);
extern int spif_str_casecmp(spif_str_t, spif_str_t);
extern int spif_str_casecmp_with_ptr(spif_str_t, spif_charptr_t);
extern int spif_str_ncmp(spif_str_t, spif_str_t, size_t);
extern int spif_str_ncmp_with_ptr(spif_str_t, spif_charptr_t, size_t);
extern int spif_str_ncasecmp(spif_str_t, spif_str_t, size_t);
extern int spif_str_ncasecmp_with_ptr(spif_str_t, spif_charptr_t, size_t);
extern size_t spif_str_index(spif_str_t, spif_char_t);
extern size_t spif_str_rindex(spif_str_t, spif_char_t);
extern size_t spif_str_find(spif_str_t, spif_str_t);
extern size_t spif_str_find_from_ptr(spif_str_t, spif_charptr_t);
extern spif_str_t spif_str_substr(spif_str_t, spif_int32_t, spif_int32_t);
extern spif_charptr_t spif_str_substr_to_ptr(spif_str_t, spif_int32_t, spif_int32_t);
extern size_t spif_str_to_num(spif_str_t, int);
extern double spif_str_to_float(spif_str_t);
extern spif_bool_t spif_str_append(spif_str_t, spif_str_t);
extern spif_bool_t spif_str_append_from_ptr(spif_str_t, spif_charptr_t);
extern spif_bool_t spif_str_trim(spif_str_t);
extern spif_bool_t spif_str_splice(spif_str_t, size_t, size_t, spif_str_t);
extern spif_bool_t spif_str_splice_from_ptr(spif_str_t, size_t, size_t, spif_charptr_t);
extern spif_bool_t spif_str_reverse(spif_str_t);
extern size_t spif_str_get_size(spif_str_t);
extern spif_bool_t spif_str_set_size(spif_str_t, size_t);
extern size_t spif_str_get_len(spif_str_t);
extern spif_bool_t spif_str_set_len(spif_str_t, size_t);
extern spif_bool_t spif_str_show(spif_str_t, spif_charptr_t);
extern spif_classname_t spif_str_type(spif_str_t);

#endif /* _LIBAST_STR_H_ */
