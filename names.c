/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Star Fighter 3000 object names
 *  Copyright (C) 2017 Christopher Bazley
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public Licence as published by
 *  the Free Software Foundation; either version 2 of the Licence, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public Licence for more details.
 *
 *  You should have received a copy of the GNU General Public Licence
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* ISO library header files */
#include <stdio.h>

/* Local header files */
#include "sfformats.h"
#include "misc.h"
#include "names.h"

const char *get_type_name(const SFObjectType type)
{
  static const char * const type_names[SFObjectType_Aerial+1] =
    { "Ground", "Bit", "Ship" };
  assert((size_t)type < ARRAY_SIZE(type_names));
  return type_names[type];
}

const char *get_obj_name(const SFObjectType type, const int index)
{
  static char buffer[64];
  static const char * const ship_names[] =
  {
    /* Although many other object meshes are recognizable, these are
       the only objects with the same role in every mission. */
    "player",    /* Appearance differs between missions */
    "fighter_1", /*     "         "       "        "    */
    "fighter_2", /*     "         "       "        "    */
    "fighter_3", /*     "         "       "        "    */
    "fighter_4", /*     "         "       "        "    */
    "three_coin",
    "ten_coin",
    "life_coin",
    "fifty_coin",
    "twenty_coin",
    "atg_coin",
    "ata_coin",
    "damage_coin",
    "big_ship_1", /* Appearance differs between missions */
    "mothership",
    "big_ship_2", /* Appearance differs between missions */
    "atg_missile",
    "ata_missile",
    "mine",
    "bomb",
    "parachute",
    "satellite",
    "dock"
  };
  static const char * const ground_names[] =
  {
    /* Although many other object meshes are recognizable, these are
       the only objects with the same role in every game map. Their
       appearance varies between maps. */
    "none",
    "gun_1",
    "gun_2",
    "gun_3",
    "sam_1",
    "sam_2",
    "sam_3",
    "hangar_1",
    "hangar_2"
  };
  const char *n;

  assert(index >= 0);
  assert((type == SFObjectType_Ground) ||
         (type == SFObjectType_Bit) ||
         (type == SFObjectType_Aerial));

  switch (type) {
  case SFObjectType_Ground:
    if ((size_t)index < ARRAY_SIZE(ground_names)) {
      n = ground_names[index];
    } else {
      sprintf(buffer, "ground_%d", index);
      n = buffer;
    }
    break;

  case SFObjectType_Bit:
    sprintf(buffer, "bit_%d", index);
    n = buffer;
    break;

  case SFObjectType_Aerial:
    if ((size_t)index < ARRAY_SIZE(ship_names)) {
      n = ship_names[index];
    } else {
      sprintf(buffer, "ship_%d", index);
      n = buffer;
    }
    break;

  default:
    n = "unknown";
    break;
  }

  return n;
}
