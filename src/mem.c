/*
 * Copyright (C) 1997-2013, Michael Jennings <mej@eterm.org>
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

/**
 * @file mem.c
 * Memory Management Subsystem Source File
 *
 * This file contains the memory management subsystem.
 *
 * @author Michael Jennings <mej@eterm.org>
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libast_internal.h"

/**
 * Allocated resources.
 *
 * This structure keeps track of generic resources.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink, spifmem_memrec_t_struct
 * @ingroup DOXGRP_MEM
 */
static spifmem_memrec_t resource_rec;

/**
 * Initialize memory management system.
 *
 * A call to this function must occur before any other part of the
 * memory management system is used.  This function initializes the
 * pointer lists.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink
 * @ingroup DOXGRP_MEM
 */
void
spifmem_init(void)
{
    D_MEM(("Constructing memory allocation records\n"));
    resource_rec.ptrs = (spifmem_ptr_t *) malloc(sizeof(spifmem_ptr_t));
}

/**
 * Add a resource to be tracked.
 *
 * @param filename The filename where the variable was allocated.
 * @param line     The line number of @a filename where the variable
 *                 was allocated.
 * @param ptr      The allocated variable.
 * @param size     The size of the resource in bytes.
 * @param type     The type of resource being tracked.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink, MALLOC(), libast_malloc()
 * @ingroup DOXGRP_MEM
 */
void
spifmem_add_var(const char *filename, unsigned long line, void *ptr, size_t size, char *type)
{
    register spifmem_ptr_t *p;

    resource_rec.cnt++;
    if (!(resource_rec.ptrs = (spifmem_ptr_t *)realloc(resource_rec.ptrs, sizeof(spifmem_ptr_t) * resource_rec.cnt))) {
        D_MEM(("Unable to reallocate pointer list -- %s\n", strerror(errno)));
    }
    p = resource_rec.ptrs + resource_rec.cnt - 1;
    D_MEM(("Adding variable (%10p, %lu bytes) from %s:%lu.\n", ptr, size, filename, line));
    D_MEM(("Storing as pointer #%lu at %10p (from %10p).\n", resource_rec.cnt, p, resource_rec.ptrs));
    p->ptr = ptr;
    p->size = size;
    spiftool_safe_strncpy(p->file, (const spif_charptr_t) filename, sizeof(p->file));
    p->line = line;
    p->type = type;
}

/**
 * Find a variable within the list of tracked resources.
 *
 * This function searches through the resource list for a given resource ID.
 *
 * @param ptr    The value of the requested pointer.
 * @return       A pointer to the #spifmem_ptr_t object within @a memrec
 *               that matches @a ptr, or NULL if not found.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink, MALLOC(), libast_malloc()
 * @ingroup DOXGRP_MEM
 */
spifmem_ptr_t *
spifmem_find_var(const void *ptr)
{
    register spifmem_ptr_t *p;
    register unsigned long i;

    REQUIRE_RVAL(ptr != NULL, NULL);

    for (i = 0, p = resource_rec.ptrs; i < resource_rec.cnt; i++, p++) {
        if (p->ptr == ptr) {
            D_MEM(("Found pointer #%lu stored at %10p (from %10p)\n", i + 1, p, resource_rec.ptrs));
            return p;
        }
    }
    return NULL;
}

/**
 * Remove a variable from the list of tracked resources.
 *
 * @param var      The variable name being freed (for diagnostic
 *                 purposes only).
 * @param filename The filename where the variable was freed.
 * @param line     The line number of @a filename where the variable
 *                 was freed.
 * @param ptr      The freed variable.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink, FREE(), libast_free()
 * @ingroup DOXGRP_MEM
 */
void
spifmem_rem_var(const char *var, const char *filename, unsigned long line, const void *ptr)
{
    register spifmem_ptr_t *p;

    USE_VAR(var);
    USE_VAR(filename);
    USE_VAR(line);

    if (!(p = spifmem_find_var(ptr))) {
        D_MEM(("ERROR:  File %s, line %d attempted to free resource %s (%10p) which was not registered.\n",
               filename, line, var, ptr));
        return;
    }
    D_MEM(("Removing variable %s (%10p) of size %lu\n", var, ptr, p->size));
    if ((--resource_rec.cnt) > 0) {
        memmove(p, p + 1, sizeof(spifmem_ptr_t) * (resource_rec.cnt - (p - resource_rec.ptrs)));
        resource_rec.ptrs = (spifmem_ptr_t *) realloc(resource_rec.ptrs, sizeof(spifmem_ptr_t) * resource_rec.cnt);
    }
}

/**
 * Resize a tracked resource.
 *
 * @param var      The variable name being resized (for diagnostic
 *                 purposes only).
 * @param filename The filename where the variable was resized.
 * @param line     The line number of @a filename where the variable
 *                 was resized.
 * @param oldp     The old value of the pointer.
 * @param newp     The new value of the pointer.
 * @param size     The new size in bytes.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink, REALLOC(), libast_realloc()
 * @ingroup DOXGRP_MEM
 */
void
spifmem_chg_var(const char *var, const char *filename, unsigned long line, const void *oldp, void *newp, size_t size)
{
    register spifmem_ptr_t *p;

    USE_VAR(var);

    if (!(p = spifmem_find_var(oldp))) {
        D_MEM(("ERROR:  File %s, line %d attempted to realloc untracked resource %s (%10p).\n", filename,
               line, var, oldp));
        return;
    }
    D_MEM(("Changing variable %s (%10p, %lu -> %10p, %lu)\n", var, oldp, p->size, newp, size));
    p->ptr = newp;
    p->size = size;
    spiftool_safe_strncpy(p->file, filename, sizeof(p->file));
    p->line = line;
}

/**
 * Dump listing of tracked resources.
 *
 * @see @link DOXGRP_MEM Memory Management Subsystem @endlink, MALLOC_DUMP(), libast_dump_mem_tables(),
 *      spifmem_dump_pointers()
 * @ingroup DOXGRP_MEM
 */
void
spifmem_dump_resources(void)
{
    register spifmem_ptr_t *p;
    unsigned long i, total;
    unsigned long len;

    len = resource_rec.cnt;
    fprintf(LIBAST_DEBUG_FD, "RES:  %lu resources stored.\n",
            (unsigned long) resource_rec.cnt);
    fprintf(LIBAST_DEBUG_FD, "RES:   Index | Resource ID |       Filename       |  Line  |  Size  |   Type\n");
    fprintf(LIBAST_DEBUG_FD, "RES:  -------+-------------+----------------------+--------+--------+----------\n");
    fflush(LIBAST_DEBUG_FD);

    for (p = resource_rec.ptrs, i = 0, total = 0; i < len; i++, p++) {
        total += p->size;
        fprintf(LIBAST_DEBUG_FD, "RES:   %5lu |  0x%08lx | %20s | %6lu | %6lu | %-8s\n",
                i, (unsigned long) p->ptr, NONULL(p->file), (unsigned long) p->line,
                (unsigned long) p->size, p->type);
        /* Flush after every line in case we crash */
        fflush(LIBAST_DEBUG_FD);
    }
    fprintf(LIBAST_DEBUG_FD, "RES:  Total size: %lu bytes\n", (unsigned long) total);
    fflush(LIBAST_DEBUG_FD);
}

/**
 * Free an array of pointers.
 *
 * This really doesn't relate to the memory management subsystem, per
 * se.  It is simply a convenience function which simplifies the
 * freeing of pointer arrays.  The first @a count pointers in the
 * array are freed, after which the array itself is freed.  If
 * @a count is 0, the array must be NULL-terminated.  All pointers up
 * to the first NULL pointer encountered will be freed.
 *
 * @param list  The pointer array to be freed.  This variable's value
 *              MUST NOT be used after being passed to this function.
 * @param count The number of pointers in the array, or 0 for a
 *              NULL-terminated array.
 *
 * @ingroup DOXGRP_MEM
 */
void
spiftool_free_array(void *list, size_t count)
{
    register size_t i;
    void **l = (void **) list;

    REQUIRE(list != NULL);

    if (count == 0) {
        count = (size_t) (-1);
    }
    for (i = 0; i < count && l[i]; i++) {
        FREE(l[i]);
    }
    FREE(list);
}

/**
 * @defgroup DOXGRP_MEM Memory Management Subsystem
 *
 * This group of functions/defines/macros implements the memory
 * management subsystem within LibAST.
 *
 * LibAST provides a mechanism for tracking resource allocations and
 * deallocations.  This system employs macro-based wrappers around
 * various resource allocators/deallocators such as Xlib GC and Pixmap
 * create/free routines and Imlib2 pixmap functions.
 *
 * To take advantage of this system, simply substitute the macro
 * versions in place of the standard versions throughout your code
 * (e.g., X_FREE_GC() instead of XFreeGC()).  If DEBUG is set to a
 * value higher than DEBUG_MEM, the LibAST-custom versions of these
 * functions will be used.  Of course, if memory debugging has not
 * been requested, the original libc/XLib/Imlib2 versions will be used
 * instead, so that you only incur the debugging overhead when you
 * want it.
 *
 * You can also define your own macros to wrap allocators and
 * deallocators if LibAST doesn't already contain support for the
 * resources you wish to track.  Simply call spifmem_add_var(),
 * spifmem_chg_var(), and spifmem_rem_var() whenever the resource is
 * created, resized, or deleted (respectively).
 *
 * NOTE: If your compiler does not support compound statement
 * expressions (i.e., a code block whose final statement specifies the
 * value of a parenthesized expression, such as:
 * ({int a = 1, b = 2; a *= b; b += a; a+b;})),
 * LibAST's built-in wrapper macros will not be available to you, nor
 * are you likely to be able to wrap many allocator functions with
 * your own macros.  In these situations, you'll need to call the
 * LibAST memory routines separately.
 *
 * A small sample program demonstrating use of LibAST's memory
 * management system can be found
 * @link mem_example.c here @endlink.
 */

/**
 * @example mem_example.c
 * Example code for using the memory management subsystem.
 *
 * This small program demonstrates how to use LibAST's built-in memory
 * management system as well as a few of the errors it can catch for
 * you.
 *
 * The following shows output similar to what you can expect to
 * receive if you build and run this program:
 *
 * @code
 * $ ./mem_example 
 * [1045859036]        mem.c |  246: spifmem_malloc(): 500 bytes requested at mem_example.c:27
 * [1045859036]        mem.c |   74: spifmem_add_var(): Adding variable (0x8049a20, 500 bytes) from mem_example.c:27.
 * [1045859036]        mem.c |   75: spifmem_add_var(): Storing as pointer #1 at 0x8049c18 (from 0x8049c18).
 * [1045859036]        mem.c |  329: spifmem_strdup(): Variable pointer (0x8049a20) at mem_example.c:36
 * [1045859036]        mem.c |  246: spifmem_malloc(): 16 bytes requested at mem_example.c:36
 * [1045859036]        mem.c |   74: spifmem_add_var(): Adding variable (0x8049c40, 16 bytes) from mem_example.c:36.
 * [1045859036]        mem.c |   75: spifmem_add_var(): Storing as pointer #2 at 0x8049c7c (from 0x8049c58).
 * [1045859036]        mem.c |  312: spifmem_free(): Variable dup (0x8049c40) at mem_example.c:39
 * [1045859036]        mem.c |   94: spifmem_find_var(): Found pointer #2 stored at 0x8049c7c (from 0x8049c58)
 * [1045859036]        mem.c |  113: spifmem_rem_var(): Removing variable dup (0x8049c40) of size 16
 * [1045859036]        mem.c |  312: spifmem_free(): Variable dup (   (nil)) at mem_example.c:43
 * [1045859036]        mem.c |  319: spifmem_free(): ERROR:  Caught attempt to free NULL pointer
 * [1045859036]        mem.c |  268: spifmem_realloc(): Variable pointer (0x8049a20 -> 1000) at mem_example.c:46
 * [1045859036]        mem.c |   94: spifmem_find_var(): Found pointer #1 stored at 0x8049c58 (from 0x8049c58)
 * [1045859036]        mem.c |  132: spifmem_chg_var(): Changing variable pointer (0x8049a20, 500 -> 0x8049c80, 1000)
 * Dumping memory allocation table:
 * PTR:  1 pointers stored.
 * PTR:   Pointer |       Filename       |  Line  |  Address |  Size  | Offset  | 00 01 02 03 04 05 06 07 |  ASCII  
 * PTR:  ---------+----------------------+--------+----------+--------+---------+-------------------------+---------
 * PTR:   0000000 |                      |      0 | 0x8049c58 | 000036 | 0000000 | 80 9c 04 08 e8 03 00 00 | €œ..è...
 * PTR:   0000000 |                      |      0 | 0x8049c58 | 000036 | 0000008 | 6d 65 6d 5f 65 78 61 6d | mem_exam
 * PTR:   0000000 |                      |      0 | 0x8049c58 | 000036 | 0000010 | 70 6c 65 2e 63 00 00 00 | ple.c...
 * PTR:   0000000 |                      |      0 | 0x8049c58 | 000036 | 0000018 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000000 |                      |      0 | 0x8049c58 | 000036 | 0000020 | 2e 00 00 00             | ....    
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000000 | 54 68 69 73 20 69 73 20 | This is 
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000008 | 61 20 74 65 73 74 2e 00 | a test..
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000010 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000018 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000020 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000028 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000030 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000038 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000040 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000048 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000050 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000058 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000060 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000068 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000070 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000078 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000080 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000088 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000090 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000098 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000a0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000a8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000b0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000b8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000c0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000c8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000d0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000d8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000e0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000e8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000f0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00000f8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000100 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000108 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000110 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000118 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000120 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000128 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000130 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000138 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000140 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000148 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000150 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000158 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000160 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000168 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000170 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000178 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000180 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000188 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000190 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000198 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001a0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001a8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001b0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001b8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001c0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001c8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001d0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001d8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001e0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001e8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001f0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00001f8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000200 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000208 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000210 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000218 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000220 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000228 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000230 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000238 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000240 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000248 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000250 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000258 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000260 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000268 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000270 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000278 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000280 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000288 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000290 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000298 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002a0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002a8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002b0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002b8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002c0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002c8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002d0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002d8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002e0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002e8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002f0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00002f8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000300 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000308 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000310 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000318 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000320 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000328 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000330 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000338 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000340 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000348 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000350 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000358 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000360 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000368 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000370 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000378 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000380 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000388 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000390 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 0000398 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003a0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003a8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003b0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003b8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003c0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003c8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003d0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003d8 | 00 00 00 00 00 00 00 00 | ........
 * PTR:   0000001 |        mem_example.c |     46 | 0x8049c80 | 001000 | 00003e0 | 00 00 00 00 00 00 00 00 | ........
 * PTR:  Total allocated memory:       1000 bytes
 * @endcode
 *
 * Here is the source code:
 */
