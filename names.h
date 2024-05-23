/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Object names
 *  Copyright (C) 2017 Christopher Bazley
 */

#ifndef NAMES_H
#define NAMES_H

#include "sfformats.h"

const char *get_type_name(SFObjectType type);

const char *get_obj_name(SFObjectType type, int index);

#endif /* NAMES_H */
