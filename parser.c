/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Star Fighter 3000 object mesh parser
 *  Copyright (C) 2018 Christopher Bazley
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
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>

/* StreamLib headers */
#include "Reader.h"

/* 3DObjLib headers */
#include "Vector.h"
#include "Coord.h"
#include "Vertex.h"
#include "Primitive.h"
#include "Group.h"
#include "Clip.h"
#include "ObjFile.h"

/* Local header files */
#include "sfformats.h"
#include "misc.h"
#include "flags.h"
#include "parser.h"
#include "version.h"
#include "names.h"
#include "colours.h"

/* Unless we do something about it, all of the objects appear reflected in
   the Z axis. */
#define FLIP_Z (1)

/* Radar dishes rotate anti-clockwise (viewed from above) */
#if FLIP_Z
#define ROTATION_SPEED ((PI*32)/1024)
#else
#define ROTATION_SPEED (-(PI*32)/1024)
#endif

enum {
  MaxPlotType = 10,
  MaxPlotCommands = 16, /* unknown what the game limit is */
  NColours = 256,
  NTints = 1 << 2, /* bits per tint */
};

typedef struct {
  int max_polygon;
  int num_commands;
  uint8_t group_mask;
  int group_order[MaxPlotCommands];
} PlotType;

typedef struct {
  SFObjectType type;
  int coll_x;
  int coll_y;
  int score;
  int hits_or_min_z;
  int explosion_style;
  int plot_type;
  int expected_max_group;
  uint16_t clip_size[2];
  int32_t clip_dist;
} ObjectInfo;

typedef struct
{
  int frame;
  const SFObjectColours *pal;
  int false_colour;
} ColourInfo;

static int parse_vertices(Reader * const r, const int object_count,
                          const SFCoordinateScale scale,
                          const SFObjectType object_type,
                          VertexArray * const varray, const int rot,
                          const bool convert, const int frame,
                          const unsigned int flags)
{
  assert(r != NULL);
  assert(!reader_ferror(r));
  assert(object_count >= 0);
  assert((object_type == SFObjectType_Ground) ||
         (object_type == SFObjectType_Bit) ||
         (object_type == SFObjectType_Aerial));

  assert(varray != NULL);
  assert(frame >= 0);
  assert(!(flags & ~FLAGS_ALL));

  const int nvertices = reader_fgetc(r);
  if (nvertices == EOF) {
    fprintf(stderr, "Failed to read no. of vertices (object %d)\n",
            object_count);
    return -1;
  }
  if (nvertices < 1) {
    fprintf(stderr, "Bad vertex count %d (object %d)\n", nvertices,
            object_count);
    return -1;
  }
  if (flags & FLAGS_VERBOSE) {
    long int const pos = reader_ftell(r);
    printf("Found %d vertices at offset %ld (0x%lx)\n", nvertices, pos, pos);
  }

  if (convert) {
    if (vertex_array_alloc_vertices(varray, nvertices) < nvertices) {
      fprintf(stderr, "Failed to allocate memory for %d vertices "
              "(object %d)\n", nvertices, object_count);
      return -1;
    }

    int s = 1;
    switch (scale) {
      case SFCoordinateScale_Small:
        s = (object_type == SFObjectType_Ground) ? 4 : 1;
        break;
      case SFCoordinateScale_Medium:
        s = (object_type == SFObjectType_Ground) ? 8 : 2;
        break;
      case SFCoordinateScale_Large:
        s = (object_type == SFObjectType_Ground) ? 16 : 8 /* not log2 */;
        break;
    }

    Coord transform[3][3] = {
      {s, 0, 0}, /* coefficients for x dimension */
      {0, s, 0}, /* coefficients for y dimension */
#if FLIP_Z
      {0, 0, -s}
#else
      {0, 0, s}  /* coefficients for z dimension */
#endif
    };
    Coord pos[3] = {0,0,0};

    for (int v = 0; v < nvertices; ++v) {
      char vbytes[3];
      if (reader_fread(vbytes, sizeof(vbytes), 1, r) != 1) {
        fprintf(stderr, "Failed to read vertex %d\n", v);
        return -1;
      }

      /* It's impossible to rotate all of the vertices belonging to
         an object model: the first coordinates are always unchanged. */
      if ((rot > 0) && (v == rot)) {
        /* rotate unit vector around the Z axis */
        transform[0][0] = cos(frame * ROTATION_SPEED) * s; /* 1 at frame 0 */
        transform[0][1] = -sin(frame * ROTATION_SPEED) * s; /* 0 at frame 0 */
        transform[1][0] = -transform[0][1]; /* 0 at frame 0 */
        transform[1][1] = transform[0][0]; /* 1 at frame 0 */
      }

      Coord offset[3] = {0.0, 0.0, 0.0};
      for (size_t dim = 0; dim < ARRAY_SIZE(offset); ++dim) {
        const SFVertexCoord vc = (SFVertexCoord)vbytes[dim];
        if (vc <= SFVertexCoord_SubUnit) {
          offset[dim] = -1.0 * (int)(1 << (SFVertexCoord_SubUnit - vc));
        } else if (vc < SFVertexCoord_Zero) {
          offset[dim] = -1.0 / (int)(2 << (vc - SFVertexCoord_SubDiv2));
        } else if (vc == SFVertexCoord_Zero) {
        } else if (vc <= SFVertexCoord_AddDiv2) {
          offset[dim] = 1.0 / (int)(2 << (SFVertexCoord_AddDiv2 - vc));
        } else if (vc >= SFVertexCoord_AddUnit) {
          offset[dim] = 1.0 * (int)(1 << (vc - SFVertexCoord_AddUnit));
        }
      } /* next dimension */

      for (size_t dim = 0; dim < ARRAY_SIZE(transform); ++dim) {
        for (size_t coeff = 0; coeff < ARRAY_SIZE(transform[0]); ++coeff) {
          pos[dim] += (transform[dim][coeff] * offset[coeff]);
        }
      } /* next dimension */

      if (vertex_array_add_vertex(varray, &pos) < 0) {
        fprintf(stderr,
                "Failed to allocate vertex memory "
                "(vertex %d of object %d)\n", v, object_count);
        return -1;
      }

      /* If we're keeping unused vertices then we need to mark them upon
         creation because otherwise they will never be marked. */
      if (flags & FLAGS_UNUSED) {
        vertex_array_set_used(varray, v);
      }

      if (flags & FLAGS_VERBOSE) {
        vertex_array_print_vertex(varray, v);
        puts("");
      }

    } /* next vertex */
  } else {
    /* Skip the vertex data */
    if (reader_fseek(r, 3l * nvertices, SEEK_CUR)) {
      fprintf(stderr, "Failed to seek end of vertices (object %d)\n",
              object_count);
      return -1;
    }
  }

  return nvertices;
}

static int parse_polygons(Reader * const r, const int object_count,
                          VertexArray * const varray,
                          Group (* const groups)[
                            SFObjectFacet_VectorsGroup+1],
                          int (* const npolygons)[
                            SFObjectFacet_VectorsGroup+1],
                          const int expected_max_group, const bool convert,
                          const unsigned int flags)
{
  assert(r != NULL);
  assert(object_count >= 0);
  assert(!reader_ferror(r));
  assert(groups != NULL);
  assert(npolygons != NULL);
  assert(expected_max_group < SFObjectFacet_VectorsGroup);
  assert(!(flags & ~FLAGS_ALL));

  /* Get number of polygons */
  const int num_polygons = reader_fgetc(r);
  if (num_polygons == EOF) {
    fprintf(stderr, "Failed to read no. of polygons (object %d)\n",
            object_count);
    return -1;
  }
  if (num_polygons < 1) {
    fprintf(stderr, "Bad polygon count %d (object %d)\n",
            num_polygons, object_count);
    return -1;
  }
  if (flags & FLAGS_VERBOSE) {
    long int const pos = reader_ftell(r);
    printf("Found %d polygons at offset %ld (0x%lx)\n", num_polygons,
           pos, pos);
  }

  int max_group = 0;
  const int nvertices = vertex_array_get_num_vertices(varray);

  for (int p = 0; p < num_polygons; ++p) {
    const int num_sides_and_group = reader_fgetc(r);
    if (num_sides_and_group == EOF) {
      fprintf(stderr, "Failed to read no. of sides and plot group "
                      "(polygon %d of object %d)\n", p, object_count);
      return -1;
    }

    const int num_sides =
      (num_sides_and_group & SFObjectFacet_NumSidesMask) >>
      SFObjectFacet_NumSidesShift;
    const int group = (num_sides_and_group & SFObjectFacet_GroupMask) >>
                       SFObjectFacet_GroupShift;
    const int colour_high =
      (num_sides_and_group & SFObjectFacet_SpecialColour);

    if (group != SFObjectFacet_VectorsGroup) {
      max_group = HIGHEST(group, max_group);
      if (group < 0 || group > expected_max_group) {
        fprintf(stderr, "Bad plot group %d (polygon %d of object %d)\n",
                group, p, object_count);
        return -1;
      }
    }

    if (num_sides < 3) {
      fprintf(stderr, "Bad side count %d (polygon %d of object %d)\n",
              num_sides, p, object_count);
      return -1;
    }

    if (flags & FLAGS_VERBOSE) {
      long int const pos = reader_ftell(r);
      printf("Found %d sides in group %d at offset %ld (0x%lx)\n",
             num_sides, group, pos, pos);
    }

    /* Store the number of polygons in this group for validation purposes
       even if we're not converting the object */
    ++(*npolygons)[group];

    if (convert) {
      Primitive * const pp = group_add_primitive((*groups) + group);
      if (pp == NULL) {
        fprintf(stderr, "Failed to allocate primitive memory "
                "(polygon %d of object %d)\n", p, object_count);
        return -1;
      }
      primitive_set_id(pp, group_get_num_primitives((*groups) + group));

      /* We need to read the polygon definition into a temporary array so
         that we can get its colour byte at the end before outputting vertex
         indices. */

      /* Get the vertex indices and colour byte */
      for (int s = 0; s < num_sides; ++s) {
        int v = reader_fgetc(r);
        if (v == EOF) {
          fprintf(stderr, "Failed to read side %d of polygon %d "
                  "of object %d\n", s, p, object_count);
          return -1;
        }

        /* Validate the vertex indices */
        if (v < 1 || v > nvertices) {
          fprintf(stderr, "Bad vertex %lld "
                  "(side %d of polygon %d of object %d)\n",
                  (long long signed)v - 1, s, p, object_count);
          return -1;
        }

        /* Vertex indices are stored using offset-1 encoding */
        --v;

        if (primitive_add_side(pp, v) < 0) {
          fprintf(stderr, "Failed to add side: too many sides? "
                          "(side %d of polygon %d of object %d)\n",
                  s, p, object_count);
          return -1;
        }
      }

#if FLIP_Z
      /* Inverting the Z coordinate axis makes all polygons back-facing
         unless we also reverse the order in which their vertices are
         specified. */
      primitive_reverse_sides(pp);
#endif

      if (flags & FLAGS_VERBOSE) {
        printf("Primitive %d in group %d:\n",
               group_get_num_primitives((*groups) + group), group);
        primitive_print(pp, varray);
        puts("");
      }

      int const side = primitive_get_skew_side(pp, varray);
      if (side >= 0) {
        fprintf(stderr, "Warning: skew polygon detected "
                        "(side %d of primitive %d of object %d)\n",
                side, p, object_count);
      }

      const int colour_low = reader_fgetc(r);
      if (colour_low == EOF) {
        fprintf(stderr, "Failed to read colour "
                "(polygon %d of object %d)\n", p, object_count);
        return -1;
      }

      primitive_set_colour(pp, colour_low + (colour_high ? 256 : 0));
    } else {
      /* Skip the vertex indices and colour byte */
      if (reader_fseek(r, num_sides + (long int)1, SEEK_CUR)) {
        fprintf(stderr, "Failed to seek end of polygon "
                "(polygon %d of object %d)\n", p, object_count);
        return -1;
      }
    }
  } /* next polygon */

  if (max_group < expected_max_group) {
    fprintf(stderr, "Warning: highest plot group is %d not %d (object %d)\n",
            max_group, expected_max_group, object_count);
  }

  return num_polygons;
}

static int get_false_colour(const Primitive *pp, void *arg)
{
  NOT_USED(pp);
  ColourInfo * const info = arg;
  assert(info != NULL);

  ++info->false_colour;
  const int colour = (info->false_colour * NTints) % NColours;
  return colour;
}

static int get_colour(const Primitive *const pp, void *arg)
{
  assert(pp != NULL);
  assert(arg != NULL);

  int colour = primitive_get_colour(pp);
  const ColourInfo * const info = arg;
  assert(info != NULL);
  assert(info->frame >= 0);
  const SFObjectColours * const pal = info->pal;

  if ((colour >= (int)ARRAY_SIZE(pal->areas.static_colours)) &&
      (colour < (int)ARRAY_SIZE(pal->colour_mappings) -
                (int)ARRAY_SIZE(pal->areas.player_livery))) {
    const int start_frame =
        colour % (int)ARRAY_SIZE(pal->areas.engine_colours.player_engine);
    unsigned int special_frame = 0;
    const int special_colour =
        colour - (int)ARRAY_SIZE(pal->areas.static_colours);

    if (special_colour < (int)
        (ARRAY_SIZE(pal->areas.engine_colours.player_engine) +
         ARRAY_SIZE(pal->areas.engine_colours.fighter_engine) +
         ARRAY_SIZE(pal->areas.engine_colours.cruiser_engine) +
         ARRAY_SIZE(pal->areas.engine_colours.super_engine) +
         ARRAY_SIZE(pal->areas.fast_flashing.enemy_ships) +
         ARRAY_SIZE(pal->areas.fast_flashing.friendly_ships) +
         ARRAY_SIZE(pal->areas.fast_flashing.player_ship))) {
      /* Fast flashing colours */
      special_frame = info->frame;
    } else {
      /* Slow flashing colours
         +1 because the first change happens on the 2nd not 3rd frame */
      special_frame = (info->frame + 1) / 2;
    }

    DEBUGF("Frame number for flashing colour %d is %d\n",
           colour, special_frame);

    colour = (colour - start_frame) +
             ((start_frame + special_frame) %
              (int)ARRAY_SIZE(pal->areas.engine_colours.player_engine));

    DEBUGF("Updated colour is %d\n", colour);
  }
  if (pal) {
    colour = pal->colour_mappings[colour];
  }
  return colour;
}

static int get_material(char *const buf, size_t const buf_size,
                        int const colour, void *arg)
{
  /* Emit logical colour number */
  NOT_USED(arg);
  return snprintf(buf, buf_size, "colour_%d", colour);
}

static int get_phys_material(char *const buf, size_t const buf_size,
                            int const colour, void *arg)
{
  /* Emit physical colour number */
  NOT_USED(arg);
  return snprintf(buf, buf_size, "riscos_%d", colour);
}

static int get_human_material(char *const buf, size_t const buf_size,
                              int const colour, void *arg)
{
  /* Emit physical colour name */
  NOT_USED(arg);
  return snprintf(buf, buf_size, "%s_%d",
                  get_colour_name(colour / NTints), colour % NTints);
}

static bool output_object(FILE * const out, const int type_count,
                          const char *const object_name,
                          const ObjectInfo *const o)
{
  assert(out != NULL);
  assert(!ferror(out));
  assert(object_name != NULL);
  assert(o != NULL);

  if (fprintf(out, "\no %s\n", object_name) < 0) {
    return false;
  }

  if (o->type == SFObjectType_Ground) {
    if (fprintf(out, "# Collision size %d,%d\n", o->coll_x, o->coll_y) < 0) {
      return false;
    }
  }

  if (o->type == SFObjectType_Ground ||
      o->type == SFObjectType_Aerial) {
    if (fprintf(out, "# Clip size: %" PRIu16 ",%" PRIu16 "\n",
                o->clip_size[0] << 1, o->clip_size[1] << 1) < 0) {
      return false;
    }
  }

  if (o->type == SFObjectType_Ground ||
      o->type == SFObjectType_Aerial) {
    if (fprintf(out, "# Score: %d\n", o->score) < 0) {
      return false;
    }
  }

  int n = 0;
  if (o->type == SFObjectType_Ground) {
    n = fprintf(out, "# Hitpoints %d\n", o->hits_or_min_z);
  } else if (o->type == SFObjectType_Aerial &&
             type_count >= 13 && type_count <= 15) {
    n = fprintf(out, "# Minimum altitude: %d\n", o->hits_or_min_z << 18);
  }
  if (n < 0)
  {
    return false;
  }

  if (o->type == SFObjectType_Ground ||
      o->type == SFObjectType_Aerial) {
    if (fprintf(out, "# Explosion style: %d\n", o->explosion_style) < 0) {
      return false;
    }
  }

  if (fprintf(out, "# Plot type: %d\n"
                   "# Highest plot group: %d\n",
                   o->plot_type, o->expected_max_group) < 0) {
    return false;
  }

  if (fprintf(out, "# Clip distance: %d\n", o->clip_dist) < 0) {
    return false;
  }

  return true;
}

static int parse_plot_types(Reader * const r,
                            PlotType (* const plot_types)[MaxPlotType+1],
                            const unsigned int flags)
{
  assert(r != NULL);
  assert(!reader_ferror(r));
  assert(plot_types != NULL);
  assert(!(flags & ~FLAGS_ALL));

  /* Read plot type definitions */
  int command = reader_fgetc(r);
  if (command == EOF) {
    fprintf(stderr, "Failed to read plot type definition\n");
    return -1;
  }

  /* Parse each plot type definition in turn until finding an end marker.
     There must be at least one. */
  int plot_type_count = 1; /* plot type 0 means plot all polygons */
  do {
    int command_count = 0;

    if (plot_type_count > MaxPlotType) {
      fprintf(stderr, "Too many plot types (max %d)\n", MaxPlotType);
      return -1;
    }

    if (flags & FLAGS_VERBOSE) {
      long int const pos = reader_ftell(r)-1;
      printf("Plot type %d is defined at offset %ld (0x%lx)\n",
             plot_type_count, pos, pos);
    }

    (*plot_types)[plot_type_count].max_polygon = -1;
    (*plot_types)[plot_type_count].group_mask = 0;

    /* Parse each plot command in turn until finding an end marker.
       There must be at least one. */
    do {
      if (command_count >= MaxPlotCommands) {
        fprintf(stderr, "Too many commands (max %d) for plot type %d\n",
                MaxPlotCommands, plot_type_count);
        return -1;
      }

      const int operand = (command & SFPlotCommands_OperandMask) >>
                          SFPlotCommands_OperandShift;
      const SFPlotAction action = (SFPlotAction)
                                  ((command & SFPlotCommands_ActionMask) >>
                                  SFPlotCommands_ActionShift);
      int group, polygon;
      if (action == SFPlotAction_FacingAlways)
      {
        polygon = 0;
        group = operand;
      }
      else
      {
        polygon = operand;
        if (polygon > (*plot_types)[plot_type_count].max_polygon) {
          (*plot_types)[plot_type_count].max_polygon = polygon;
        }

        /* Next byte is a group number */
        group = reader_fgetc(r);
        if (group == EOF) {
          fprintf(stderr, "Failed to read plot group "
                  "(command %d of plot type %d)\n", command_count,
                  plot_type_count);
          return -1;
        }
      }

      if ((group < 0) || (group >= SFObjectFacet_VectorsGroup)) {
        fprintf(stderr, "Bad plot group %d (command %d of plot type %d)\n",
                group, command_count, plot_type_count);
        return -1;
      }

      (*plot_types)[plot_type_count].group_mask |= 1u << group;

      if (flags & FLAGS_VERBOSE) {
        switch (action) {
           case SFPlotAction_FacingAlways:
             printf("Plot front-facing polygons in group %d\n",
                    group);
             break;
           case SFPlotAction_FacingIf:
             printf("Plot front-facing polygons in group %d "
                    "if polygon %d in group 7 is front-facing\n",
                    group, polygon);
             break;
           case SFPlotAction_FacingIfNot:
             printf("Plot front-facing polygons in group %d "
                    "if polygon %d in group 7 is back-facing\n",
                    group, polygon);
             break;
           case SFPlotAction_AllIf:
             printf("Plot group %d if polygon %d in group 7 is "
                    "front-facing\n", group, polygon);
             break;
           case SFPlotAction_AllIfNot:
             printf("Plot group %d if polygon %d in group 7 is "
                    "back-facing\n", group, polygon);
             break;
           default:
             fprintf(stderr, "Bad plot action %d "
                     "(command %d of plot type %d)\n",
                     action, command_count, plot_type_count);
             return -1;
        }
      }

      (*plot_types)[plot_type_count].group_order[command_count] = group;

      command = reader_fgetc(r);
      if (command == EOF) {
        fprintf(stderr, "Failed to read command or terminator "
                "(plot type %d)\n", plot_type_count);
        return -1;
      }
      ++command_count;
    } while (command != SFPlotCommands_EndOfType);

    (*plot_types)[plot_type_count].num_commands = command_count;

    command = reader_fgetc(r);
    if (command == EOF) {
      fprintf(stderr, "Failed to read plot type definition or terminator\n");
      return -1;
    }
    ++plot_type_count;
  } while (command != SFPlotCommands_EndOfData);

  return plot_type_count;
}

static void mark_vertices(
                       VertexArray * const varray,
                       Group (* const groups)[SFObjectFacet_VectorsGroup+1],
                       const int object_count, const unsigned int flags)
{
  assert(groups != NULL);
  assert(object_count >= 0);
  assert(!(flags & ~FLAGS_ALL));

  if (flags & FLAGS_UNUSED) {
    /* We're keeping all vertices */
    vertex_array_set_all_used(varray);
  } else {
    /* Mark only the used vertices */
    for (int g = 0; g <= SFObjectFacet_VectorsGroup; ++g) {
      group_set_used((*groups) + g, varray);
    }

    /* Report the unused vertices */
    if (flags & FLAGS_VERBOSE) {
      int count = 0;
      const int nvertices = vertex_array_get_num_vertices(varray);
      for (int v = 0; v < nvertices; ++v) {
        if (!vertex_array_is_used(varray, v)) {
          Coord (*coords)[3] = vertex_array_get_coords(varray, v);
          printf("Vertex %d {%g,%g,%g} is unused (object %d)\n", v,
                 (*coords)[0], (*coords)[1], (*coords)[2], object_count);
          ++count;
        }
      }
      printf("Object %d has %d unused vertices\n", object_count, count);
    }
  }
}

static bool parse_objects(Reader * const r, FILE * const out,
                          const int first, const int last,
                          const SFObjectType type, const char * const name,
                          const SFObjectColours * const pal, const int frame,
                          const unsigned int flags,
                          PlotType (* const plot_types)[MaxPlotType+1],
                          const int num_plot_types)
{
  int object_count = 0, vtotal = 0, max_plot_type = -1;
  int type_counts[SFObjectType_Aerial+1] = {0, 0, 0};
  long int obj_start = 0;
  bool success = false, list_title = false;

  assert(r != NULL);
  assert(!reader_ferror(r));
  assert(first >= 0);
  assert(last == -1 || last >= first);
  assert((type == (SFObjectType)-1) || (type == SFObjectType_Ground) ||
         (type == SFObjectType_Bit) || (type == SFObjectType_Aerial));
  assert(frame >= 0);
  assert(!(flags & ~FLAGS_ALL));
  assert(plot_types != NULL);
  assert(num_plot_types >= 1);
  assert(num_plot_types <= MaxPlotType+1);

  Group groups[SFObjectFacet_VectorsGroup+1];
  for (int g = 0; g <= SFObjectFacet_VectorsGroup; ++g) {
    group_init(groups + g);
  }
  VertexArray varray;
  vertex_array_init(&varray);

  if (flags & FLAGS_LIST) {
    obj_start = reader_ftell(r);
  }

  int32_t last_explosion_num;
  if (!reader_fread_int32(&last_explosion_num, r)) {
    fprintf(stderr, "Failed to read no. of explosions (object %d)\n",
            object_count);
    return false;
  }

  /* Parse each object definition in turn until finding an end marker.
     There must be at least one. */
  do {
    ObjectInfo o = { SFObjectType_Ground };
    SFCoordinateScale scale = SFCoordinateScale_Small;
    bool convert = false, match = false, stop = false;
    int rot = 0;
    long int expl_size = 36l * (last_explosion_num + 1l);
    if (flags & FLAGS_VERBOSE) {
      long int const pos = reader_ftell(r);
      printf("Found %"PRId32" explosion lines (%ld bytes) "
             "at offset %ld (0x%lx)\n", last_explosion_num + 1, expl_size,
             pos, pos);
    }

    /* Skip the explosions data */
    if (reader_fseek(r, expl_size, SEEK_CUR)) {
      fprintf(stderr, "Failed to seek object attributes (object %d)\n",
              object_count);
      break;
    }

    /* Get object type */
    const int byte = reader_fgetc(r);
    if (byte == EOF) {
      fprintf(stderr, "Failed to read object type (object %d)\n",
              object_count);
      break;
    }
    if ((byte != SFObjectType_Aerial) &&
        (byte != SFObjectType_Ground) &&
        (byte != SFObjectType_Bit)) {
      fprintf(stderr, "Bad object type %d (object %d)\n", byte,
              object_count);
      break;
    }
    o.type = (SFObjectType)byte;
    if (flags & FLAGS_VERBOSE) {
      long int const pos = reader_ftell(r)-1;
      printf("Found object %d of type %d at offset %ld (0x%lx)\n",
             object_count, (int)o.type, pos, pos);
    }

    assert((size_t)o.type < ARRAY_SIZE(type_counts));
    const int type_count = type_counts[o.type];
    const char * const object_name = get_obj_name(o.type, type_count);

    if (type == (SFObjectType)-1 || o.type == type) {
      int req_index = object_count;
      if (type != (SFObjectType)-1) {
        req_index = type_count;
      }
      if ((req_index >= first) && (last == -1 || req_index <= last)) {
        /* Within the specified range of object numbers */
        if (name != NULL) {
          /* Only match the named object */
          if (!strcmp(name, object_name)) {
            match = true;
            /* Stop after finding the named object (assuming there are
               no others of the same name) */
            stop = true;
          }
        } else {
          /* No object name so match any object in the range */
          match = true;
        }
      }
      if ((last != -1) && (req_index >= last)) {
        /* Stop after the end of the specified range of object numbers */
        stop = true;
      }
    }

    if (match && (out != NULL)) {
      convert = true;

      const int byte = reader_fgetc(r);
      if (byte == EOF) {
        fprintf(stderr, "Failed to read scale (object %d)\n", object_count);
        break;
      }
      scale = (SFCoordinateScale)byte;

      rot = reader_fgetc(r);
      if (rot == EOF) {
        fprintf(stderr, "Failed to read rotator (object %d)\n",
                object_count);
        break;
      }

      const int gr_obj_coll_size = reader_fgetc(r);
      if (gr_obj_coll_size == EOF) {
        fprintf(stderr,
                "Failed to read packed collision size (object %d)\n",
                object_count);
        break;
      }

      if (o.type == SFObjectType_Ground) {
        o.coll_x = (gr_obj_coll_size & SFObjectCollisionSize_XMask) >>
                   SFObjectCollisionSize_XShift;
        o.coll_y = (gr_obj_coll_size & SFObjectCollisionSize_YMask) >>
                   SFObjectCollisionSize_YShift;
      }

      if (!reader_fread_uint16(o.clip_size, r) ||
          !reader_fread_uint16(o.clip_size + 1, r)) {
        fprintf(stderr, "Failed to read clip size (object %d)\n",
                object_count);
        break;
      }

      o.score = reader_fgetc(r) * 25;
      if (o.score == EOF) {
        fprintf(stderr, "Failed to read score (object %d)\n", object_count);
        break;
      }

      o.hits_or_min_z = reader_fgetc(r);
      if (o.hits_or_min_z == EOF) {
        fprintf(stderr, "Failed to read hitpoints (object %d)\n",
                object_count);
        break;
      }

      o.explosion_style = reader_fgetc(r);
      if (o.explosion_style == EOF) {
        fprintf(stderr, "Failed to read explosion style (object %d)\n",
                object_count);
        break;
      }
    } else {
      /* Skip the rest of the object attributes */
      if (reader_fseek(r, 10, SEEK_CUR)) {
        fprintf(stderr, "Failed to seek vertex data (object %d)\n",
                object_count);
        break;
      }
    }

    const int plot_type_and_last_group = reader_fgetc(r);
    if (plot_type_and_last_group == EOF) {
      fprintf(stderr,
              "Failed to read plot type and max plot group (object %d)\n",
              object_count);
      break;
    }
    o.plot_type = (plot_type_and_last_group &
                   SFObject_PlotTypeMask) >> SFObject_PlotTypeShift;

    if (o.plot_type >= num_plot_types) {
      fprintf(stderr, "Bad plot type %d (object %d)\n", o.plot_type,
              object_count);
      break;
    }
    if (o.plot_type > max_plot_type) {
      max_plot_type = o.plot_type;
    }

    o.expected_max_group = (plot_type_and_last_group &
                            SFObject_LastGroupMask) >>
                               SFObject_LastGroupShift;

    if ((o.expected_max_group < 0) ||
        (o.expected_max_group >= SFObjectFacet_VectorsGroup)) {
      fprintf(stderr, "Bad highest plot group %d (object %d)\n",
              o.expected_max_group, object_count);
      break;
    }
    if ((o.expected_max_group > 0) && (o.plot_type == 0)) {
      fprintf(stderr, "Warning: highest plot group %d is higher than "
                      "expected for plot type 0 (object %d)\n",
                      o.expected_max_group, object_count);
    }

    vertex_array_clear(&varray);

    /* Get number of vertices */
    const int nvertices = parse_vertices(r, object_count, scale, o.type,
                                         &varray, rot, convert, frame,
                                         flags);
    if (nvertices == -1) {
      break;
    }

    if (rot >= nvertices) {
      fprintf(stderr, "Bad rotator %d (object %d)\n", rot, object_count);
      break;
    }

    /* Find the first word-aligned offset ahead of the vertex data */
    if (reader_fseek(r, WORD_ALIGN(reader_ftell(r)), SEEK_SET)) {
      fprintf(stderr, "Failed to seek clip distance (object %d)\n",
              object_count);
      break;
    }

    if (!reader_fread_int32(&o.clip_dist, r)) {
      fprintf(stderr, "Failed to read clip distance (object %d)\n",
              object_count);
      break;
    }

    int npolygons[SFObjectFacet_VectorsGroup + 1] = {0};
    if (convert) {
      for (int g = 0; g <= SFObjectFacet_VectorsGroup; ++g) {
        group_delete_all(groups + g);
      }
    }

    const int num_polygons = parse_polygons(r, object_count, &varray,
                                            &groups, &npolygons,
                                            o.expected_max_group,
                                            convert, flags);
    if (num_polygons == -1) {
      break;
    }

    /* Validate the object's plot type. We can do this even if we didn't
       read its vertex coordinates or polygon sides. */
    if (o.plot_type != 0) {
      /* Check that the referenced polygons exist */
      const int max_polygon = (*plot_types)[o.plot_type].max_polygon;
      if (max_polygon >= npolygons[SFObjectFacet_VectorsGroup]) {
        fprintf(stderr,
                "Plot type %d is predicated on undefined polygon %d "
                "(object %d)\n", o.plot_type, max_polygon, object_count);
        break;
      }

      /* Check that the referenced polygon groups exist. */
      const unsigned int group_mask = (*plot_types)[o.plot_type].group_mask;
      int g;
      for (g = 0; g <= SFObjectFacet_VectorsGroup; ++g) {
        if (group_mask & (1u << g)) {
          /* This group may be plotted */
          if (npolygons[g] == 0) {
            break;
          }
        } else {
          /* This group cannot be plotted */
          if (npolygons[g] > 0) {
            if (g != SFObjectFacet_VectorsGroup) {
              fprintf(stderr,
                      "Warning: plot type %d hides group %d (object %d)\n",
                      o.plot_type, g, object_count);
            }
            if ((flags & FLAGS_HIDDEN_POLYGONS) == 0) {
              group_delete_all(groups + g);
            }
          }
        }
      }
      if (g <= SFObjectFacet_VectorsGroup) {
        fprintf(stderr,
                "Plot type %d references undefined group %d (object %d)\n",
                o.plot_type, g, object_count);
        break;
      }
    }

    if (convert) {
      /* In cases of overlapping coplanar polygons,
         split the underlying polygon */
      if (flags & FLAGS_CLIP_POLYGONS) {
        const int *group_order;
        int group_order_len;
        const int first_group[] = {0};
        if (o.plot_type == 0) {
          group_order = first_group;
          group_order_len = ARRAY_SIZE(first_group);
        } else {
          /* The group order array may contain duplicate values because
             there is no single canonical order for all possible scenes.
             (It depends on the tested surface normals.) */
          group_order = (*plot_types)[o.plot_type].group_order;
          group_order_len = (*plot_types)[o.plot_type].num_commands;
        }

        if (!clip_polygons(&varray, groups, group_order, group_order_len,
                           (flags & FLAGS_VERBOSE) != 0)) {
          fprintf(stderr,
                  "Clipping of overlapping coplanar polygons failed\n");
          break;
        }
      }

      /* Mark the vertices in preparation for culling unused ones. */
      mark_vertices(&varray, &groups, object_count, flags);

      if (!(flags & FLAGS_DUPLICATE)) {
        /* Unmark duplicate vertices in preparation for culling them. */
        if (vertex_array_find_duplicates(&varray,
                                         (flags & FLAGS_VERBOSE) != 0) < 0) {
          fprintf(stderr, "Detection of duplicate vertices failed\n");
          break;
        }
      }

      int vobject;
      if (!(flags & FLAGS_UNUSED) || !(flags & FLAGS_DUPLICATE)) {
        /* Cull unused and/or duplicate vertices */
        vobject = vertex_array_renumber(&varray,
                                        (flags & FLAGS_VERBOSE) != 0);
        DEBUGF("Renumbered %d vertices\n", vobject);
      } else {
        vobject = vertex_array_get_num_vertices(&varray);
        DEBUGF("No need to renumber %d vertices\n", vobject);
      }

      VertexStyle vstyle = VertexStyle_Positive;
      if (flags & FLAGS_NEGATIVE_INDICES) {
        vstyle = VertexStyle_Negative;
      }

      MeshStyle mstyle = MeshStyle_NoChange;
      if (flags & FLAGS_TRIANGLE_FANS) {
        mstyle = MeshStyle_TriangleFan;
      } else if (flags & FLAGS_TRIANGLE_STRIPS) {
        mstyle = MeshStyle_TriangleStrip;
      }

      ColourInfo info = {
        .frame = frame,
        .pal = pal,
        .false_colour = 0
      };

      int (*get_material_cb)(char *, size_t, int, void *) = NULL;
      if (flags & FLAGS_PHYSICAL_COLOUR) {
        if (flags & FLAGS_HUMAN_READABLE) {
          get_material_cb = get_human_material;
        } else {
          get_material_cb = get_phys_material;
        }
      } else {
        assert(!(flags & FLAGS_HUMAN_READABLE));
        get_material_cb = get_material;
      }

      if (!output_object(out, type_count, object_name, &o) ||
          !output_vertices(out, vobject, &varray, (rot > 0) ? rot : -1) ||
          !output_primitives(out, object_name, vtotal, vobject,
                           &varray, groups, ARRAY_SIZE(groups),
                           (flags & FLAGS_FALSE_COLOUR) ?
                             get_false_colour : get_colour,
                           get_material_cb,
                           &info, vstyle, mstyle)) {
        fprintf(stderr,
                "Failed writing to output file: %s\n",
                strerror(errno));
        break;
      }

      vtotal += vobject;
    }

    /* Find the first word-aligned offset ahead of the polygons data */
    if (reader_fseek(r, WORD_ALIGN(reader_ftell(r)), SEEK_SET)) {
      fprintf(stderr, "Failed to seek collision data (object %d)\n",
              object_count);
      break;
    }

    if (flags & FLAGS_VERBOSE) {
      long int const pos = reader_ftell(r);
      printf("Collision is defined at offset %ld (0x%lx)\n", pos, pos);
    }

    int32_t last_collision_num;
    if (!reader_fread_int32(&last_collision_num, r)) {
      fprintf(stderr, "Failed to read no. of collision boxes (object %d)\n",
              object_count);
      break;
    }

    long int coll_size = 28l * (last_collision_num + 1l);
    if (flags & FLAGS_VERBOSE) {
      long int const pos = reader_ftell(r) + 8;
      printf("Found %" PRId32 " collision boxes (%ld bytes) "
             "at offset %ld (0x%lx)\n", last_collision_num + 1, coll_size,
             pos, pos);
    }

    /* Skip the collision boxes */
    if (reader_fseek(r, 8 + coll_size + 4, SEEK_CUR)) {
      fprintf(stderr, "Failed to seek end of object (object %d)\n",
              object_count);
      break;
    }

    if (flags & FLAGS_LIST) {
      if (match && !list_title) {
        puts("\nIndex  Type    Index  Name          Verts  "
             "Faces      Offset        Size");
        list_title = true;
      }

      const long int obj_size = reader_ftell(r) - obj_start;
      if (match) {
        printf("%5d  %-6.6s  %5d  %-12.12s  %5d  %5d  %10ld  %10ld\n",
               object_count, get_type_name(o.type), type_count,
               object_name, nvertices, num_polygons, obj_start,
               obj_size);
      }

      obj_start += obj_size;
    }

    if (!reader_fread_int32(&last_explosion_num, r)) {
      fprintf(stderr, "Failed to read no. of explosions (object %d)\n",
              object_count);
      break;
    }

    if (last_explosion_num == SFObjects_EndOfData) {
      if (flags & FLAGS_VERBOSE) {
        printf("Found file terminator at %ld\n",
               reader_ftell(r) - sizeof(int32_t));
      }
      success = true;
    } else if (!(flags & FLAGS_SUMMARY) && stop) {
      /* Force early exit if we just converted the object sought
         or we went too far. */
      success = true;
      break;
    }

    ++object_count;
    ++type_counts[o.type];
  } while (last_explosion_num != SFObjects_EndOfData);

  for (int g = 0; g <= SFObjectFacet_VectorsGroup; ++g) {
    group_free(groups + g);
  }
  vertex_array_free(&varray);

  if (success && (flags & FLAGS_SUMMARY)) {
    printf("\nFound %d object definition%s, comprising:\n",
           object_count, object_count > 1 ? "s" : "");

    for (size_t i = 0; i < ARRAY_SIZE(type_counts); ++i) {
      printf("  %d %s object%s\n", type_counts[i],
             get_type_name((SFObjectType)i),
             type_counts[i] > 1 ? "s" : "");
    }
  }

  if (last_explosion_num == SFObjects_EndOfData) {
    if ((max_plot_type + 1) < num_plot_types) {
      fprintf(stderr, "Warning: plot types %d .. %d are unused\n",
              max_plot_type + 1, num_plot_types - 1);
    }
  }

  return success;
}

bool sf3k_to_obj(Reader * const in, FILE * const out, const int first,
                 const int last, const SFObjectType type,
                 const char * const name, const SFObjectColours * const pal,
                 const int frame, const char * const mtl_file,
                 const unsigned int flags)
{
  bool success = true;
  int num_plot_types = 0;

  assert(in != NULL);
  assert(!reader_ferror(in));
  assert(!reader_feof(in));
  assert(first >= 0);
  assert(last == -1 || last >= first);
  assert((type == (SFObjectType)-1) || (type == SFObjectType_Ground) ||
         (type == SFObjectType_Bit) || (type == SFObjectType_Aerial));
  assert(frame >= 0);
  assert(mtl_file != NULL);
  assert(!(flags & ~FLAGS_ALL));

  if ((out != NULL) &&
      fprintf(out, "# Star Fighter 3000 graphics\n"
                   "# Converted by SF3KtoObj "VERSION_STRING"\n"
                   "# Animation frame: %d\n\n"
                   "mtllib %s\n", frame, mtl_file) < 0) {
    fprintf(stderr, "Failed writing to output file: %s\n", strerror(errno));
    success = false;
  } else {
    PlotType plot_types[MaxPlotType+1];
    num_plot_types = parse_plot_types(in, &plot_types, flags);
    if (num_plot_types == -1) {
      success = false;
    } else {
      /* Find the first word-aligned offset at least 4 bytes ahead of the
         plot type definitions terminator */
      if (reader_fseek(in, WORD_ALIGN(reader_ftell(in)+3), SEEK_SET)) {
        fprintf(stderr, "Failed to seek first object\n");
        success = false;
      } else {
        success = parse_objects(in, out, first, last,  type, name, pal,
                                frame, flags, &plot_types, num_plot_types);
      }
    }
  }

  return success;
}
