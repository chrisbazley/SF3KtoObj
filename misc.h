/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Miscellaneous macro definitions
 *  Copyright (C) 2018 Christopher Bazley
 */

#ifndef MISC_H
#define MISC_H

#define PI (3.1415926535897896)

/* Modify these definitions for Unix or Windows file paths. */
#define PATH_SEPARATOR '.'
#define EXT_SEPARATOR '/' /* e.g. ADFS::4.$.Star3000.Graphics.Earth1/obj */

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/* Return the nearest word aligned value greater than or equal to a given
 * expression (useful for sprite widths, which must include right hand wastage).
 */
#define WORD_ALIGN(value) (((value) + 3) & ~3)

/* Suppress compiler warnings about an unused function argument. */
#define NOT_USED(x) ((void)(x))

#define HIGHEST(a, b) ((a) > (b) ? (a) : (b))

#ifdef FORTIFY
#include "Fortify.h"
#endif

#ifdef USE_CBDEBUG

#include "Debug.h"
#include "PseudoIO.h"

#else /* USE_CBDEBUG */

#include <stdio.h>
#include <assert.h>

#define DEBUG_SET_OUTPUT(output_mode, log_name)

#ifdef DEBUG_OUTPUT
#define DEBUGF if (1) printf
#else
#define DEBUGF if (0) printf
#endif /* DEBUG_OUTPUT */

#endif /* USE_CBDEBUG */

#ifdef USE_OPTIONAL
#include <stdlib.h>

#undef NULL
#define NULL ((_Optional void *)0)

static inline void optional_free(_Optional void *x)
{
    free((void *)x);
}
#undef free
#define free(x) optional_free(x)

static inline _Optional void *optional_malloc(size_t n)
{
    return malloc(n);
}
#undef malloc
#define malloc(n) optional_malloc(n)

static inline _Optional FILE *optional_fopen(const char *name, const char *mode)
{
    return fopen(name, mode);
}
#undef fopen
#define fopen(p, n) optional_fopen(p, n)

#else
#define _Optional
#endif

#endif /* MISC_H */
