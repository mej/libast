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

static const char cvs_ident[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libast_internal.h>

/* *INDENT-OFF* */
static SPIF_CONST_TYPE(class) s_class = {
    SPIF_DECL_CLASSNAME(str),
    (spif_func_t) spif_str_new,
    (spif_func_t) spif_str_init,
    (spif_func_t) spif_str_done,
    (spif_func_t) spif_str_del,
    (spif_func_t) spif_str_show,
    (spif_func_t) spif_str_cmp,
    (spif_func_t) spif_str_dup,
    (spif_func_t) spif_str_type
};
SPIF_TYPE(class) SPIF_CLASS_VAR(str) = &s_class;
/* *INDENT-ON* */

const size_t buff_inc = 4096;

spif_str_t
spif_str_new(void)
{
    spif_str_t self;

    self = SPIF_ALLOC(str);
    spif_str_init(self);
    return self;
}

spif_str_t
spif_str_new_from_ptr(spif_charptr_t old)
{
    spif_str_t self;

    self = SPIF_ALLOC(str);
    spif_str_init_from_ptr(self, old);
    return self;
}

spif_str_t
spif_str_new_from_buff(spif_charptr_t buff, size_t size)
{
    spif_str_t self;

    self = SPIF_ALLOC(str);
    spif_str_init_from_buff(self, buff, size);
    return self;
}

spif_str_t
spif_str_new_from_fp(FILE * fp)
{
    spif_str_t self;

    self = SPIF_ALLOC(str);
    spif_str_init_from_fp(self, fp);
    return self;
}

spif_str_t
spif_str_new_from_fd(int fd)
{
    spif_str_t self;

    self = SPIF_ALLOC(str);
    spif_str_init_from_fd(self, fd);
    return self;
}

spif_str_t
spif_str_new_from_num(long num)
{
    spif_str_t self;

    self = SPIF_ALLOC(str);
    spif_str_init_from_num(self, num);
    return self;
}

spif_bool_t
spif_str_del(spif_str_t self)
{
    D_OBJ(("Deleting string %8p\n", self));
    spif_str_done(self);
    SPIF_DEALLOC(self);
    return TRUE;
}

spif_bool_t
spif_str_init(spif_str_t self)
{
    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS_VAR(str));
    self->s = SPIF_NULL_TYPE(charptr);
    self->len = 0;
    self->size = 0;
    return TRUE;
}

spif_bool_t
spif_str_init_from_ptr(spif_str_t self, spif_charptr_t old)
{
    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS_VAR(str));
    self->len = strlen(SPIF_CONST_CAST_C(char *) old);
    self->size = self->len + 1;
    self->s = SPIF_CAST(charptr) MALLOC(self->size);
    memcpy(self->s, old, self->size);
    return TRUE;
}

spif_bool_t
spif_str_init_from_buff(spif_str_t self, spif_charptr_t buff, size_t size)
{
    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS_VAR(str));
    self->size = size;
    if (buff != SPIF_NULL_TYPE(charptr)) {
        self->len = strnlen(SPIF_CONST_CAST_C(char *) buff, size);
    } else {
        self->len = 0;
    }
    if (self->size == self->len) {
        self->size++;
    }
    self->s = SPIF_CAST(charptr) MALLOC(self->size);
    if (buff != SPIF_NULL_TYPE(charptr)) {
        memcpy(self->s, buff, self->len);
    }
    self->s[self->len] = 0;
    return TRUE;
}

spif_bool_t
spif_str_init_from_fp(spif_str_t self, FILE * fp)
{
    spif_charptr_t p, end = NULL;

    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS_VAR(str));
    self->size = buff_inc;
    self->len = 0;
    self->s = SPIF_CAST(charptr) MALLOC(self->size);

    for (p = self->s; fgets(SPIF_CAST_C(char *)p, buff_inc, fp); p += buff_inc) {
        if ((end = SPIF_CAST(charptr)
             strchr(SPIF_CONST_CAST_C(char *)p, '\n')) == NULL) {
            self->size += buff_inc;
            self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
        } else {
            *end = 0;
            break;
        }
    }
    self->len = (size_t) ((end)
                          ? (end - self->s)
                          : ((int) strlen(SPIF_CONST_CAST_C(char *)self->s)));
    self->size = self->len + 1;
    self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
    return TRUE;
}

spif_bool_t
spif_str_init_from_fd(spif_str_t self, int fd)
{
    int n;
    spif_charptr_t p;

    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS_VAR(str));
    self->size = buff_inc;
    self->len = 0;
    self->s = SPIF_CAST(charptr) MALLOC(self->size);

    for (p = self->s; ((n = read(fd, p, buff_inc)) > 0) || (errno == EINTR);) {
        self->size += n;
        self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
        p += n;
    }
    self->len = self->size - buff_inc;
    self->size = self->len + 1;
    self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
    self->s[self->len] = 0;
    return TRUE;
}

spif_bool_t
spif_str_init_from_num(spif_str_t self, long num)
{
    char buff[28];

    spif_obj_init(SPIF_OBJ(self));
    spif_obj_set_class(SPIF_OBJ(self), SPIF_CLASS_VAR(str));

    snprintf(buff, sizeof(buff), "%ld", num);
    self->len = strlen(buff);
    self->size = self->len + 1;
    self->s = SPIF_CAST(charptr) MALLOC(self->size);
    strcpy(self->s, buff);

    return TRUE;
}

spif_bool_t
spif_str_done(spif_str_t self)
{
    REQUIRE_RVAL(!SPIF_STR_ISNULL(self), FALSE);
    if (self->size) {
        FREE(self->s);
        self->len = 0;
        self->size = 0;
        self->s = SPIF_NULL_TYPE(charptr);
    }
    return TRUE;
}

spif_str_t
spif_str_dup(spif_str_t orig)
{
    spif_str_t self;

    REQUIRE_RVAL(!SPIF_STR_ISNULL(orig), FALSE);
    self = SPIF_ALLOC(str);
    memcpy(self, orig, SPIF_SIZEOF_TYPE(str));
    self->s = SPIF_CAST(charptr) STRDUP(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(orig));
    self->len = orig->len;
    self->size = orig->size;
    return self;
}

spif_cmp_t
spif_str_cmp(spif_str_t self, spif_str_t other)
{
    return SPIF_CMP_FROM_INT(strcmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)SPIF_STR_STR(other)));
}

spif_cmp_t
spif_str_cmp_with_ptr(spif_str_t self, spif_charptr_t other)
{
    return SPIF_CMP_FROM_INT(strcmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)other));
}

spif_cmp_t
spif_str_casecmp(spif_str_t self, spif_str_t other)
{
    return SPIF_CMP_FROM_INT(strcasecmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)SPIF_STR_STR(other)));
}

spif_cmp_t
spif_str_casecmp_with_ptr(spif_str_t self, spif_charptr_t other)
{
    return SPIF_CMP_FROM_INT(strcasecmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)other));
}

spif_cmp_t
spif_str_ncmp(spif_str_t self, spif_str_t other, size_t cnt)
{
    return SPIF_CMP_FROM_INT(strncmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)SPIF_STR_STR(other), cnt));
}

spif_cmp_t
spif_str_ncmp_with_ptr(spif_str_t self, spif_charptr_t other, size_t cnt)
{
    return SPIF_CMP_FROM_INT(strncmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)other, cnt));
}

spif_cmp_t
spif_str_ncasecmp(spif_str_t self, spif_str_t other, size_t cnt)
{
    return SPIF_CMP_FROM_INT(strncasecmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)SPIF_STR_STR(other), cnt));
}

spif_cmp_t
spif_str_ncasecmp_with_ptr(spif_str_t self, spif_charptr_t other, size_t cnt)
{
    return SPIF_CMP_FROM_INT(strncasecmp(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), SPIF_CONST_CAST_C(char *)other, cnt));
}

size_t
spif_str_index(spif_str_t self, spif_char_t c)
{
    return (size_t) ((long) (index(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), c)) - (long) (SPIF_STR_STR(self)));
}

size_t
spif_str_rindex(spif_str_t self, spif_char_t c)
{
    return (size_t) ((long) (rindex(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), c)) - (long) (SPIF_STR_STR(self)));
}

size_t
spif_str_find(spif_str_t self, spif_str_t other)
{
    return (size_t) ((long) (strstr(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self),
                                    SPIF_CONST_CAST_C(char *) SPIF_STR_STR(other))) - (long) (SPIF_STR_STR(self)));
}

size_t
spif_str_find_from_ptr(spif_str_t self, spif_charptr_t other)
{
    return (size_t) ((long) (strstr(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self),
                                    SPIF_CONST_CAST_C(char *) other)) - (long) (SPIF_STR_STR(self)));
}

spif_str_t
spif_str_substr(spif_str_t self, spif_int32_t idx, spif_int32_t cnt)
{
    return spif_str_new_from_buff(SPIF_STR_STR(self) + ((idx < 0) ? (self->len) : (0)) + idx, cnt);
}

spif_charptr_t
spif_str_substr_to_ptr(spif_str_t self, spif_int32_t idx, spif_int32_t cnt)
{
    spif_charptr_t newstr;

    newstr = SPIF_CAST(charptr) MALLOC(cnt + 1);
    memcpy(newstr, SPIF_STR_STR(self) + ((idx < 0) ? (self->len) : (0)) + idx, cnt);
    newstr[cnt] = 0;
    return newstr;
}

size_t
spif_str_to_num(spif_str_t self, int base)
{
    return (size_t) (strtoul(SPIF_CONST_CAST_C(char *) SPIF_STR_STR(self), (char **) NULL, base));
}

double
spif_str_to_float(spif_str_t self)
{
    return (double) (strtod(SPIF_CONST_CAST_C(char *)SPIF_STR_STR(self), (char **) NULL));
}

spif_bool_t
spif_str_append(spif_str_t self, spif_str_t other)
{
    self->size += other->size - 1;
    self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
    memcpy(self->s + self->len, SPIF_STR_STR(other), other->len + 1);
    self->len += other->len;
    return TRUE;
}

spif_bool_t
spif_str_append_char(spif_str_t self, spif_char_t c)
{
    self->len++;
    if (self->size <= self->len) {
        self->size++;
        self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
    }
    SPIF_CAST(char) (self->s[self->len - 1]) = c;
    self->s[self->len] = 0;
    return TRUE;
}

spif_bool_t
spif_str_append_from_ptr(spif_str_t self, spif_charptr_t other)
{
    size_t len;

    len = strlen(SPIF_CONST_CAST_C(char *) other);
    self->size += len;
    self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
    memcpy(self->s + self->len, other, len + 1);
    self->len += len;
    return TRUE;
}

spif_bool_t
spif_str_clear(spif_str_t self, spif_char_t c)
{
    memset(self->s, c, self->size);
    self->s[self->len] = 0;
    return TRUE;
}

spif_bool_t
spif_str_trim(spif_str_t self)
{
    spif_charptr_t start, end;

    start = self->s;
    end = self->s + self->len - 1;
    for (; isspace((spif_uchar_t) (*start)) && (start < end); start++);
    for (; isspace((spif_uchar_t) (*end)) && (start < end); end--);
    if (start > end) {
        return spif_str_done(self);
    }
    *(++end) = 0;
    self->len = (size_t) (end - start);
    self->size = self->len + 1;
    memmove(self->s, start, self->size);
    self->s = SPIF_CAST(charptr) REALLOC(self->s, self->size);
    return TRUE;
}

spif_bool_t
spif_str_splice(spif_str_t self, size_t idx, size_t cnt, spif_str_t other)
{
    spif_charptr_t tmp, ptmp;
    size_t newsize;

    newsize = self->len + ((SPIF_OBJ_ISNULL(other)) ? (0) : (other->len)) - cnt + 1;
    ptmp = tmp = SPIF_CAST(charptr) MALLOC(newsize);
    memcpy(tmp, self->s, idx);
    ptmp += idx;
    if (!SPIF_OBJ_ISNULL(other)) {
        memcpy(ptmp, other->s, other->len);
        ptmp += other->len;
    }
    memcpy(ptmp, self->s + idx + cnt, self->len - idx - cnt + 1);
    if (self->size < newsize) {
        self->s = SPIF_CAST(charptr) REALLOC(self->s, newsize);
        self->size = newsize;
    }
    self->len = newsize - 1;
    memcpy(self->s, tmp, newsize);
    FREE(tmp);
    return TRUE;
}

spif_bool_t
spif_str_splice_from_ptr(spif_str_t self, size_t idx, size_t cnt, spif_charptr_t other)
{
    spif_charptr_t tmp, ptmp;
    size_t len, newsize;

    len = (other ? strlen(SPIF_CONST_CAST_C(char *) other) : 0);
    newsize = self->len + len - cnt + 1;
    ptmp = tmp = SPIF_CAST(charptr) MALLOC(newsize);
    memcpy(tmp, self->s, idx);
    ptmp += idx;
    if (len) {
        memcpy(ptmp, other, len);
        ptmp += len;
    }
    memcpy(ptmp, self->s + idx + cnt, self->len - idx - cnt + 1);
    if (self->size < newsize) {
        self->s = SPIF_CAST(charptr) REALLOC(self->s, newsize);
        self->size = newsize;
    }
    self->len = newsize - 1;
    memcpy(self->s, tmp, newsize);
    FREE(tmp);
    return TRUE;
}

spif_bool_t
spif_str_reverse(spif_str_t self)
{
    return ((strrev(SPIF_CAST_C(char *) self->s)) ? TRUE : FALSE);
}

SPIF_DEFINE_PROPERTY_FUNC_C(str, size_t, size);
SPIF_DEFINE_PROPERTY_FUNC_C(str, size_t, len);

spif_str_t
spif_str_show(spif_str_t self, spif_charptr_t name, spif_str_t buff, size_t indent)
{
    char tmp[4096];

    if (SPIF_STR_ISNULL(self)) {
        SPIF_OBJ_SHOW_NULL(str, name, buff, indent, tmp);
        return buff;
    }

    memset(tmp, ' ', indent);
    snprintf(tmp + indent, sizeof(tmp) - indent, "(spif_str_t) %s:  %8p { \"", name, self);
    if (SPIF_STR_ISNULL(buff)) {
        buff = spif_str_new_from_ptr(tmp);
    } else {
        spif_str_append_from_ptr(buff, tmp);
    }

    spif_str_append(buff, self);

    snprintf(tmp, sizeof(tmp), "\", len %lu, size %lu }\n", (unsigned long) self->len,
             (unsigned long) self->size);
    spif_str_append_from_ptr(buff, tmp);
    return buff;
}

spif_classname_t
spif_str_type(spif_str_t self)
{
    return SPIF_OBJ_CLASSNAME(self);
}
