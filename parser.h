/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Star Fighter 3000 object mesh parser
 *  Copyright (C) 2018 Christopher Bazley
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>

#include "sfformats.h"

#include "Reader.h"

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

bool sf3k_to_obj(Reader *in, _Optional FILE *out, int first, int last,
                 SFObjectType type, _Optional const char *name,
                 _Optional const SFObjectColours *pal, int frame,
                 const char *mtl_file, unsigned int flags);

#endif /* PARSER_H */
