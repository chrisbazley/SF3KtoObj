/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Materials library generation
 *  Copyright (C) 2016 Christopher Bazley
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

/* StreamLib files */
#include "Reader.h"

/* Local header files */
#include "misc.h"
#include "flags.h"
#include "materials.h"
#include "version.h"
#include "colours.h"

enum {
  NLogicalColours = 320,
  NTintBits = 2,
  NTints = 1 << NTintBits,
  TLowShift = 0,
  THighShift = 1,
  RLowShift = 2,
  BLowShift = 3,
  RHighShift = 4,
  GLowShift = 5,
  GHighShift = 6,
  BHighShift = 7,
  CompMax = (1 << 4) - 1,
};

static void decode_colour(const int colour, double * const red,
                          double * const green, double * const blue,
                          const unsigned int flags)
{
  assert(colour >= 0);
  assert(colour <= UINT8_MAX);
  assert(red != NULL);
  assert(green != NULL);
  assert(blue != NULL);
  assert(!(flags & ~FLAGS_ALL));

  /* Get the tint bits, which are shared between all components */
  const int t = ((colour >> TLowShift) & 1) |
                (((colour >> THighShift) & 1) << 1);

  const int r = ((colour >> RLowShift) & 1) |
                (((colour >> RHighShift) & 1) << 1);

  const int g = ((colour >> GLowShift) & 1) |
                (((colour >> GHighShift) & 1) << 1);

  const int b = ((colour >> BLowShift) & 1) |
                (((colour >> BHighShift) & 1) << 1);
  if (flags & FLAGS_VERBOSE) {
    printf("red:0x%x green:0x%x blue:0x%x tint:0x%x\n", r, g, b, t);
  }

  /* Piece together the final colour component values */
  const int rt = (r << NTintBits) | t;
  const int gt = (g << NTintBits) | t;
  const int bt = (b << NTintBits) | t;
  if (flags & FLAGS_VERBOSE) {
    printf("red:0x%x green:0x%x blue:0x%x\n", rt, gt, bt);
  }

  *red = (double)rt/CompMax;
  *green = (double)gt/CompMax;
  *blue = (double)bt/CompMax;
}

bool sf3k_to_mtl(Reader * const in, FILE * const out,
                 const int first, const int last,
                 const double d, const int illum, double (* const ks)[3],
                 const double ns, const int sharpness, const double ni,
                 double (* const tf)[3], const unsigned int flags)
{
  bool success = true;
  bool phys_output[UCHAR_MAX+1] = {false};
  int start = 0, end = NLogicalColours;

  assert(in != NULL);
  assert(!reader_ferror(in));
  assert(!reader_feof(in));
  assert(out != NULL);
  assert(!ferror(out));
  assert(first >= 0);
  assert(last == -1 || last >= first);
  assert(!(flags & ~FLAGS_ALL));

  if (fprintf(out, "# Star Fighter 3000 material library\n"
                   "# Converted by SF3KtoMtl "VERSION_STRING"\n") < 0) {
    fprintf(stderr,
            "Failed writing to material library file: %s\n",
            strerror(errno));
    success = false;
  } else if (first > 0) {
    /* Seek a particular logical colour, if specified */
    if (reader_fseek(in, first, SEEK_SET)) {
      fprintf(stderr, "Failed to seek logical colour %d\n", first);
      success = false;
    } else {
      start = first;
      if (last != -1) {
        end = last + 1;
      }
    }
  }

  for (int i = start; (i < end) && success; ++i) {
    const int colour = reader_fgetc(in);
    if (colour == EOF) {
      fprintf(stderr, "Failed to read logical colour %d\n", i);
      success = false;
      break;
    }

    if (flags & FLAGS_VERBOSE) {
      printf("logical colour:%d physical colour:%d\n", i, colour);
    }

    double red, green, blue;
    decode_colour(colour, &red, &green, &blue, flags);

    int n;
    if (flags & FLAGS_PHYSICAL_COLOUR) {
      /* Physical colour names may not be unique, so check we
         haven't output this material already */
      if (phys_output[colour]) {
        continue;
      }
      phys_output[colour] = true;
      if (flags & FLAGS_HUMAN_READABLE) {
        n = fprintf(out, "\nnewmtl %s_%d\n",
                    get_colour_name(colour / NTints), colour % NTints);
      } else {
        n = fprintf(out, "\nnewmtl riscos_%d\n", colour);
      }
    } else {
      n = fprintf(out, "\nnewmtl colour_%d\n", i);
    }

    if (!(flags & FLAGS_HUMAN_READABLE) && (n >= 0) && (illum <= 9)) {
      n = fprintf(out, "# %s tint %d\n",
                  get_colour_name(colour / NTints), colour % NTints);
    }

    if ((n >= 0) && (illum >= 1) && (illum <= 9)) {
      /* Diffuse illumination model includes an ambient constant term in
         addition to the diffuse shading term for each light source */
      n = fprintf(out, "Ka %f %f %f\n", red, green, blue);
    }

    if ((n >= 0) && (illum <= 9)) {
      /* Constant colour illumination model uses the diffuse reflectance
         as the colour of the material */
      n = fprintf(out, "Kd %f %f %f\n", red, green, blue);
    }

    if ((n >= 0) && (illum >= 2) && (illum <= 9)) {
      /* Diffuse and specular illumination model requires a specular
         shading term for each light source */
      n = fprintf(out, "Ks %f %f %f\n",
                  ks ? (*ks)[0] : red,
                  ks ? (*ks)[1] : green,
                  ks ? (*ks)[2] : blue);
    }

    if ((n >= 0) && (illum >= 6) && (illum <= 7)) {
      /* Refraction model requires a transmission
         filter for refracted light passing through */
      n = fprintf(out, "Tf %f %f %f\n",
                  (*tf)[0], (*tf)[1], (*tf)[2]);
    }

    if ((n >= 0) && (d != 1.0)) {
      /* Dissolve works on all illumination models */
      n = fprintf(out, "d %f\n", d);
    }

    if ((n >= 0) && (illum >= 2) && (illum <= 9)) {
      n = fprintf(out, "Ns %f\n", ns);
    }

    if ((n >= 0) && (illum >= 3) && (illum <= 9) && (sharpness != 60)) {
      /* Sharpness can be specified for the reflection map if different
         from the default value. */
      n = fprintf(out, "sharpness %d\n", sharpness);
    }

    if ((n >= 0) && (illum >= 6) && (illum <= 7)) {
      /* Refraction model requires optical density */
      n = fprintf(out, "Ni %f\n", ni);
    }

    if (n >= 0) {
      n = fprintf(out, "illum %d\n", illum);
    }

    if (n < 0) {
      fprintf(stderr, "Failed writing to material library file: %s\n",
              strerror(errno));
      success = false;
      break;
    }
  }

  return success;
}
