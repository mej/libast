
/*
 * Copyright (C) 1997-2000, Michael Jennings
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

#include "libast_internal.h"

static void memrec_add_var(memrec_t *, const char *, unsigned long, void *, size_t);
static ptr_t *memrec_find_var(memrec_t *, const void *);
static void memrec_rem_var(memrec_t *, const char *, const char *, unsigned long, const void *);
static void memrec_chg_var(memrec_t *, const char *, const char *, unsigned long, const void *, void *, size_t);
static void memrec_dump_pointers(memrec_t *);
static void memrec_dump_resources(memrec_t *);

/* 
 * These're added for a pretty obvious reason -- they're implemented towards
 * The beginning of each one's respective function. (The ones with capitalized
 * letters. I'm not sure that they'll be useful outside of gdb. Maybe. 
 */
#ifdef MALLOC_CALL_DEBUG
static int malloc_count = 0;
static int calloc_count = 0;
static int realloc_count = 0;
static int free_count = 0;
#endif

static memrec_t malloc_rec, pixmap_rec, gc_rec;

void
memrec_init(void)
{
  D_MEM(("Constructing memory allocation records\n"));
  malloc_rec.ptrs = (ptr_t *) malloc(sizeof(ptr_t));
  pixmap_rec.ptrs = (ptr_t *) malloc(sizeof(ptr_t));
  gc_rec.ptrs = (ptr_t *) malloc(sizeof(ptr_t));
}

static void
memrec_add_var(memrec_t *memrec, const char *filename, unsigned long line, void *ptr, size_t size)
{
  register ptr_t *p;

  ASSERT(memrec != NULL);
  memrec->cnt++;
  if ((memrec->ptrs = (ptr_t *) realloc(memrec->ptrs, sizeof(ptr_t) * memrec->cnt)) == NULL) {
    D_MEM(("Unable to reallocate pointer list -- %s\n", strerror(errno)));
  }
  p = memrec->ptrs + memrec->cnt - 1;
  D_MEM(("Adding variable (%8p, %lu bytes) from %s:%lu.\n", ptr, size, filename, line));
  D_MEM(("Storing as pointer #%lu at %8p (from %8p).\n", memrec->cnt, p, memrec->ptrs));
  p->ptr = ptr;
  p->size = size;
  strncpy(p->file, filename, LIBAST_FNAME_LEN);
  p->file[LIBAST_FNAME_LEN] = 0;
  p->line = line;
}

static ptr_t *
memrec_find_var(memrec_t *memrec, const void *ptr)
{
  register ptr_t *p;
  register unsigned long i;

  ASSERT(memrec != NULL);
  REQUIRE_RVAL(ptr != NULL, NULL);

  for (i = 0, p = memrec->ptrs; i < memrec->cnt; i++, p++) {
    if (p->ptr == ptr) {
      D_MEM(("Found pointer #%lu stored at %8p (from %8p)\n", i + 1, p, memrec->ptrs));
      return p;
    }
  }
  return NULL;
}

static void
memrec_rem_var(memrec_t *memrec, const char *var, const char *filename, unsigned long line, const void *ptr)
{
  register ptr_t *p;

  ASSERT(memrec != NULL);

  if ((p = memrec_find_var(memrec, ptr)) == NULL) {
    D_MEM(("ERROR:  File %s, line %d attempted to free variable %s (%8p) which was not allocated with MALLOC/REALLOC\n", filename, line, var, ptr));
    return;
  }
  D_MEM(("Removing variable %s (%8p) of size %lu\n", var, ptr, p->size));
  if ((--memrec->cnt) > 0) {
    memmove(p, p + 1, sizeof(ptr_t) * (memrec->cnt - (p - memrec->ptrs)));
    memrec->ptrs = (ptr_t *) realloc(memrec->ptrs, sizeof(ptr_t) * memrec->cnt);
  }
}

static void
memrec_chg_var(memrec_t *memrec, const char *var, const char *filename, unsigned long line, const void *oldp, void *newp, size_t size)
{
  register ptr_t *p;

  ASSERT(memrec != NULL);

  if ((p = memrec_find_var(memrec, oldp)) == NULL) {
    D_MEM(("ERROR:  File %s, line %d attempted to realloc variable %s (%8p) which was not allocated with MALLOC/REALLOC\n", filename, line, var, oldp));
    return;
  }
  D_MEM(("Changing variable %s (%8p, %lu -> %8p, %lu)\n", var, oldp, p->size, newp, size));
  p->ptr = newp;
  p->size = size;
  strncpy(p->file, filename, LIBAST_FNAME_LEN);
  p->line = line;
}

static void
memrec_dump_pointers(memrec_t *memrec)
{
  register ptr_t *p;
  unsigned long i, j, k, l, total = 0;
  unsigned long len;
  unsigned char buff[9];

  ASSERT(memrec != NULL);
  fprintf(LIBAST_DEBUG_FD, "PTR:  %lu pointers stored.\n", memrec->cnt);
  fprintf(LIBAST_DEBUG_FD, "PTR:   Pointer |       Filename       |  Line  |  Address |  Size  | Offset  | 00 01 02 03 04 05 06 07 |  ASCII  \n");
  fprintf(LIBAST_DEBUG_FD, "PTR:  ---------+----------------------+--------+----------+--------+---------+-------------------------+---------\n");
  fflush(LIBAST_DEBUG_FD);
  len = sizeof(ptr_t) * memrec->cnt;
  memset(buff, 0, sizeof(buff));

  /* First, dump the contents of the memrec->ptrs[] array. */
  for (p = memrec->ptrs, j = 0; j < len; j += 8) {
    fprintf(LIBAST_DEBUG_FD, "PTR:   %07lu | %20s | %6lu | %8p | %06lu | %07x | ", (unsigned long) 0, "", (unsigned long) 0, memrec->ptrs,
            (unsigned long) (sizeof(ptr_t) * memrec->cnt), (unsigned int) j);
    /* l is the number of characters we're going to output */
    l = ((len - j < 8) ? (len - j) : (8));
    /* Copy l bytes (up to 8) from memrec->ptrs[] (p) to buffer */
    memcpy(buff, ((char *) p) + j, l);
    buff[l] = 0;
    for (k = 0; k < l; k++) {
      fprintf(LIBAST_DEBUG_FD, "%02x ", buff[k]);
    }
    /* If we printed less than 8 bytes worth, pad with 3 spaces per byte */
    for (; k < 8; k++) {
      fprintf(LIBAST_DEBUG_FD, "   ");
    }
    /* Finally, print the printable ASCII string for those l bytes */
    fprintf(LIBAST_DEBUG_FD, "| %-8s\n", safe_str((char *) buff, l));
    /* Flush after every line in case we crash */
    fflush(LIBAST_DEBUG_FD);
  }

  /* Now print out each pointer and its contents. */
  for (i = 0; i < memrec->cnt; p++, i++) {
    /* Add this pointer's size to our total */
    total += p->size;
    for (j = 0; j < p->size; j += 8) {
      fprintf(LIBAST_DEBUG_FD, "PTR:   %07lu | %20s | %6lu | %8p | %06lu | %07x | ", i + 1, NONULL(p->file), p->line, p->ptr, (unsigned long) p->size, (unsigned int) j);
      /* l is the number of characters we're going to output */
      l = ((p->size - j < 8) ? (p->size - j) : (8));
      /* Copy l bytes (up to 8) from p->ptr to buffer */
      memcpy(buff, ((char *) p->ptr) + j, l);
      buff[l] = 0;
      for (k = 0; k < l; k++) {
	fprintf(LIBAST_DEBUG_FD, "%02x ", buff[k]);
      }
      /* If we printed less than 8 bytes worth, pad with 3 spaces per byte */
      for (; k < 8; k++) {
	fprintf(LIBAST_DEBUG_FD, "   ");
      }
      /* Finally, print the printable ASCII string for those l bytes */
      fprintf(LIBAST_DEBUG_FD, "| %-8s\n", safe_str((char *) buff, l));
      /* Flush after every line in case we crash */
      fflush(LIBAST_DEBUG_FD);
    }
  }
  fprintf(LIBAST_DEBUG_FD, "PTR:  Total allocated memory: %10lu bytes\n", total);
  fflush(LIBAST_DEBUG_FD);
}

static void
memrec_dump_resources(memrec_t *memrec)
{
  register ptr_t *p;
  unsigned long i, total;
  unsigned long len;

  ASSERT(memrec != NULL);
  len = memrec->cnt;
  fprintf(LIBAST_DEBUG_FD, "RES:  %lu resources stored.\n", memrec->cnt);
  fprintf(LIBAST_DEBUG_FD, "RES:   Index | Resource ID |       Filename       |  Line  |  Size  \n");
  fprintf(LIBAST_DEBUG_FD, "RES:  -------+-------------+----------------------+--------+--------\n");
  fflush(LIBAST_DEBUG_FD);

  for (p = memrec->ptrs, i = 0, total = 0; i < len; i++, p++) {
    total += p->size;
    fprintf(LIBAST_DEBUG_FD, "RES:   %5lu |  0x%08x | %20s | %6lu | %6lu\n", i, (unsigned) p->ptr, NONULL(p->file), p->line, (unsigned long) p->size);
    /* Flush after every line in case we crash */
    fflush(LIBAST_DEBUG_FD);
  }
  fprintf(LIBAST_DEBUG_FD, "RES:  Total size: %lu bytes\n", total);
  fflush(LIBAST_DEBUG_FD);
}

/******************** MEMORY ALLOCATION INTERFACE ********************/
void *
libast_malloc(const char *filename, unsigned long line, size_t size)
{
  void *temp;

#ifdef MALLOC_CALL_DEBUG
  ++malloc_count;
  if (!(malloc_count % MALLOC_MOD)) {
    fprintf(LIBAST_DEBUG_FD, "Calls to malloc(): %d\n", malloc_count);
  }
#endif

  D_MEM(("%lu bytes requested at %s:%lu\n", size, filename, line));

  temp = (void *) malloc(size);
  ASSERT_RVAL(temp != NULL, NULL);
  if (DEBUG_LEVEL >= DEBUG_MEM) {
    memrec_add_var(&malloc_rec, filename, line, temp, size);
  }
  return (temp);
}

void *
libast_realloc(const char *var, const char *filename, unsigned long line, void *ptr, size_t size)
{
  void *temp;

#ifdef MALLOC_CALL_DEBUG
  ++realloc_count;
  if (!(realloc_count % REALLOC_MOD)) {
    D_MEM(("Calls to realloc(): %d\n", realloc_count));
  }
#endif

  D_MEM(("Variable %s (%8p -> %lu) at %s:%lu\n", var, ptr, (unsigned long) size, filename, line));
  if (ptr == NULL) {
    temp = (void *) libast_malloc(__FILE__, __LINE__, size);
  } else {
    temp = (void *) realloc(ptr, size);
    ASSERT_RVAL(temp != NULL, ptr);
    if (DEBUG_LEVEL >= DEBUG_MEM) {
      memrec_chg_var(&malloc_rec, var, filename, line, ptr, temp, size);
    }
  }
  return (temp);
}

void *
libast_calloc(const char *filename, unsigned long line, size_t count, size_t size)
{
  void *temp;

#ifdef MALLOC_CALL_DEBUG
  ++calloc_count;
  if (!(calloc_count % CALLOC_MOD)) {
    fprintf(LIBAST_DEBUG_FD, "Calls to calloc(): %d\n", calloc_count);
  }
#endif

  D_MEM(("%lu units of %lu bytes each requested at %s:%lu\n", count, size, filename, line));
  temp = (void *) calloc(count, size);
  ASSERT_RVAL(temp != NULL, NULL);
  if (DEBUG_LEVEL >= DEBUG_MEM) {
    memrec_add_var(&malloc_rec, filename, line, temp, size * count);
  }
  return (temp);
}

void
libast_free(const char *var, const char *filename, unsigned long line, void *ptr)
{
#ifdef MALLOC_CALL_DEBUG
  ++free_count;
  if (!(free_count % FREE_MOD)) {
    fprintf(LIBAST_DEBUG_FD, "Calls to free(): %d\n", free_count);
  }
#endif

  D_MEM(("Variable %s (%8p) at %s:%lu\n", var, ptr, filename, line));
  if (ptr) {
    if (DEBUG_LEVEL >= DEBUG_MEM) {
      memrec_rem_var(&malloc_rec, var, filename, line, ptr);
    }
    free(ptr);
  } else {
    D_MEM(("ERROR:  Caught attempt to free NULL pointer\n"));
  }
}

char *
libast_strdup(const char *var, const char *filename, unsigned long line, const char *str)
{
  register char *newstr;
  register size_t len;

  D_MEM(("Variable %s (%8p) at %s:%lu\n", var, str, filename, line));

  len = strlen(str) + 1;  /* Copy NUL byte also */
  newstr = (char *) libast_malloc(filename, line, len);
  strcpy(newstr, str);
  return (newstr);
}

void
libast_dump_mem_tables(void)
{
  fprintf(LIBAST_DEBUG_FD, "Dumping memory allocation table:\n");
  memrec_dump_pointers(&malloc_rec);
}

#ifdef LIBAST_X11_SUPPORT

/******************** PIXMAP ALLOCATION INTERFACE ********************/

Pixmap
libast_x_create_pixmap(const char *filename, unsigned long line, Display *d, Drawable win, unsigned int w, unsigned int h, unsigned int depth)
{
  Pixmap p;

  D_MEM(("Creating %ux%u pixmap of depth %u for window 0x%08x at %s:%lu\n", w, h, depth, win, filename, line));

  p = XCreatePixmap(d, win, w, h, depth);
  ASSERT_RVAL(p != None, None);
  if (DEBUG_LEVEL >= DEBUG_MEM) {
    memrec_add_var(&pixmap_rec, filename, line, (void *) p, w * h * (depth / 8));
  }
  return (p);
}

void
libast_x_free_pixmap(const char *var, const char *filename, unsigned long line, Display *d, Pixmap p)
{
  D_MEM(("Freeing pixmap %s (0x%08x) at %s:%lu\n", var, p, filename, line));
  if (p) {
    if (DEBUG_LEVEL >= DEBUG_MEM) {
      memrec_rem_var(&pixmap_rec, var, filename, line, (void *) p);
    }
    XFreePixmap(d, p);
  } else {
    D_MEM(("ERROR:  Caught attempt to free NULL pixmap\n"));
  }
}

# ifdef LIBAST_IMLIB2_SUPPORT
void
libast_imlib_register_pixmap(const char *var, const char *filename, unsigned long line, Pixmap p)
{
  D_MEM(("Registering pixmap %s (0x%08x) created by Imlib2 at %s:%lu\n", var, p, filename, line));
  if (p) {
    if (DEBUG_LEVEL >= DEBUG_MEM) {
      if (!memrec_find_var(&pixmap_rec, (void *) p)) {
        memrec_add_var(&pixmap_rec, filename, line, (void *) p, 1);
      } else {
        D_MEM(("Pixmap 0x%08x already registered.\n"));
      }
    }
  } else {
    D_MEM(("ERROR:  Refusing to register a NULL pixmap\n"));
  }
}

void
libast_imlib_free_pixmap(const char *var, const char *filename, unsigned long line, Pixmap p)
{
  D_MEM(("Freeing pixmap %s (0x%08x) at %s:%lu using Imlib2\n", var, p, filename, line));
  if (p) {
    if (DEBUG_LEVEL >= DEBUG_MEM) {
      memrec_rem_var(&pixmap_rec, var, filename, line, (void *) p);
    }
    imlib_free_pixmap_and_mask(p);
  } else {
    D_MEM(("ERROR:  Caught attempt to free NULL pixmap\n"));
  }
}
# endif

void
libast_dump_pixmap_tables(void)
{
  fprintf(LIBAST_DEBUG_FD, "Dumping X11 Pixmap allocation table:\n");
  memrec_dump_resources(&pixmap_rec);
}



/********************** GC ALLOCATION INTERFACE **********************/

GC
libast_x_create_gc(const char *filename, unsigned long line, Display *d, Drawable win, unsigned long mask, XGCValues *gcv)
{
  GC gc;

  D_MEM(("Creating gc for window 0x%08x at %s:%lu\n", win, filename, line));

  gc = XCreateGC(d, win, mask, gcv);
  ASSERT_RVAL(gc != None, None);
  if (DEBUG_LEVEL >= DEBUG_MEM) {
    memrec_add_var(&gc_rec, filename, line, (void *) gc, sizeof(XGCValues));
  }
  return (gc);
}

void
libast_x_free_gc(const char *var, const char *filename, unsigned long line, Display *d, GC gc)
{
  D_MEM(("libast_x_free_gc() called for variable %s (0x%08x) at %s:%lu\n", var, gc, filename, line));
  if (gc) {
    if (DEBUG_LEVEL >= DEBUG_MEM) {
      memrec_rem_var(&gc_rec, var, filename, line, (void *) gc);
    }
    XFreeGC(d, gc);
  } else {
    D_MEM(("ERROR:  Caught attempt to free NULL GC\n"));
  }
}

void
libast_dump_gc_tables(void)
{
  fprintf(LIBAST_DEBUG_FD, "Dumping X11 GC allocation table:\n");
  memrec_dump_resources(&gc_rec);
}

#endif
