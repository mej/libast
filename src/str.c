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

static const char cvs_ident[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libast_internal.h>

/* Declaration for the spif_str_t classname variable. */
SPIF_DECL_CLASSNAME(str);

spif_str_t
spif_str_new(void)
{
  spif_str_t self;

  self = SPIF_ALLOC(str);
  spif_str_init(self);
  return self;
}

spif_bool_t
spif_str_del(spif_str_t self)
{
  spif_str_done(self);
  SPIF_DEALLOC(self);
  return TRUE;
}

spif_bool_t
spif_str_init(spif_str_t self)
{
  spif_obj_init(SPIF_OBJ(self));
  spif_obj_set_classname(SPIF_OBJ(self), SPIF_CLASSNAME(str));
  /* ... */
  return TRUE;
}

spif_bool_t
spif_str_done(spif_str_t self)
{
  USE_VAR(self);
  /* ... */
  return TRUE;
}
