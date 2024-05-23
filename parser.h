/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Star Fighter 3000 object mesh parser
 *  Copyright (C) 2018 Christopher Bazley
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

#include "sfformats.h"

#include "Reader.h"

bool sf3k_to_obj(Reader *in, FILE *out, int first, int last,
                 SFObjectType type, const char *name,
                 const SFObjectColours *pal, int frame,
                 const char *mtl_file, unsigned int flags);

#endif /* PARSER_H */
