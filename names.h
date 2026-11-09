/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Object names
 *  Copyright (C) 2017 Christopher Bazley
 */

#ifndef NAMES_H
#define NAMES_H

#include "sfformats.h"

#include "strb.h"

const char *get_type_name(SFObjectType type);

/* Write the object name at the current position; return EOF on failure, zero on success. */
int get_obj_name(strb_t *name, SFObjectType type, int index);

#endif /* NAMES_H */
