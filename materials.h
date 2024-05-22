/*
 *  SF3KtoMtl - Converts Star Fighter 3000 palettes to Wavefront format
 *  Materials library generation
 *  Copyright (C) 2016 Christopher Bazley
 */

#ifndef MATERIALS_H
#define MATERIALS_H

/* ISO library header files */
#include <stdbool.h>
#include <stdio.h>

#include "Reader.h"

bool sf3k_to_mtl(Reader *in, FILE *out,
                 int first, int last, double d,
                 int illum, double (*ks)[3],
                 double ns,
                 int sharpness, double ni,
                 double (*tf)[3],
                 unsigned int flags);

#endif /* MATERIALS_H */
