/*
 *  SF3KtoMtl - Converts Star Fighter 3000 palettes to Wavefront format
 *  Command-line parser
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
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>

/* StreamLib headers */
#include "ReaderGKey.h"
#include "ReaderRaw.h"

/* CBUtilLib headers */
#include "ArgUtils.h"
#include "StrExtra.h"
#include "StringBuff.h"

/* Local headers */
#include "misc.h"
#include "flags.h"
#include "materials.h"
#include "version.h"

enum {
  NColours = 320,
  HistoryLog2 = 9 /* Base 2 logarithm of the history size used by
                     the compression algorithm */
};

static bool process_file(const char * const input_file,
                         const char * const output_file,
                         const int first, const int last,
                         const double d, const int illum,
                         double (* const ksp)[3],
                         const double ns, const int sharpness, const double ni,
                         double (* const tf)[3],
                         const unsigned int flags, const bool time,
                         const bool raw)
{
  FILE *out = NULL, *in = NULL;
  bool success = true;

  assert(!(flags & ~FLAGS_ALL));

  if (input_file != NULL) {
    /* An explicit input file name was specified, so open it */
    if (flags & FLAGS_VERBOSE)
      printf("Opening input file '%s'\n", input_file);

    in = fopen(input_file, "rb");
    if (in == NULL) {
      fprintf(stderr, "Failed to open input file '%s': %s\n",
              input_file, strerror(errno));
      success = false;
    }
  } else {
    /* Default input is from standard input stream */
    fprintf(stderr, "Reading from stdin...\n");
    in = stdin;
  }

  if (success) {
    if (output_file != NULL) {
      /* A different output file name was specified, so open it */
      if (flags & FLAGS_VERBOSE)
        printf("Opening output file '%s'\n", output_file);

      out = fopen(output_file, "w");
      if (out == NULL) {
        fprintf(stderr, "Failed to open output file '%s': %s\n",
                output_file, strerror(errno));
        success = false;
      }
    } else {
      /* Default output is to standard output stream */
      out = stdout;
    }
  }

  if (success) {
    const clock_t start_time = time ? clock() : 0;

    Reader r;
    if (raw) {
      reader_raw_init(&r, in);
    } else {
      success = reader_gkey_init(&r, HistoryLog2, in);
    }

    if (success) {
      success = sf3k_to_mtl(&r, out, first, last, d, illum, ksp, ns,
                            sharpness, ni, tf, flags);
      reader_destroy(&r);
    }

    if (success && time)
    {
      printf("Time taken: %.2f seconds\n",
             (double)(clock_t)(clock() - start_time) / CLOCKS_PER_SEC);
    }
  }

  if (in != NULL && in != stdin) {
    if (flags & FLAGS_VERBOSE)
      puts("Closing input file");
    fclose(in);
  }

  if (out != NULL && out != stdout) {
    if (flags & FLAGS_VERBOSE)
      puts("Closing output file");

    if (fclose(out)) {
      fprintf(stderr, "Failed to close output file '%s': %s\n",
                      output_file, strerror(errno));
      success = false;
    }
  }

  /* Delete malformed output unless debugging is enabled */
  if (!success && !(flags & FLAGS_VERBOSE) && out != NULL && out != stdout) {
    remove(output_file);
  }

  return success;
}

static int syntax_msg(FILE * const f, const char * const path)
{
  assert(f != NULL);
  assert(path != NULL);

  const char * const leaf = strtail(path, PATH_SEPARATOR, 1);
  fprintf(f,
          "usage: %s [switches] [<input-file> [<output-file>]]\n"
          "or     %s -batch [switches] <file1> [<file2> .. <fileN>]\n"
          "If no input file is specified, it reads from stdin.\n"
          "If no output file is specified, it writes to stdout.\n"
          "In batch processing mode, output file names are generated by appending\n"
          "extension 'mtl' to the input file names.\n", leaf, leaf);

  fputs("Switches (names may be abbreviated):\n"
        "  -help               Display this text\n"
        "  -batch              Process a batch of files (see above)\n"
        "  -index N            Logical colour to convert (N=0..319, default all)\n"
        "  -first N            First logical colour to convert\n"
        "  -last N             Last logical colour to convert\n"
        "  -outfile <name>     Write output to the named file instead of stdout\n"
        "  -raw                Input is uncompressed raw data\n"
        "  -time               Show the total time for each file processed\n"
        "  -verbose or -debug  Emit debug information (and keep bad output)\n", f);

  fputs("Switches to customize the output:\n"
        "  -physical           Output unique physical colours as materials\n"
        "  -human              Output readable material names (implies -physical)\n"
        "  -d N                Dissolve factor for transparency\n"
        "                      (N=0..1, default 1)\n"
        "  -illum N            Which illumination model to use\n"
        "                      (N=0..10, default 0)\n"
        "     0  Constant colour model\n"
        "     1  Diffuse model\n"
        "     2  Diffuse and specular model\n"
        "     3  Diffuse and specular model with ray tracing and reflection map\n"
        "     4  Like 3 but with the dissolve factor adjusted to simulate glass\n"
        "     5  Like 3 but with additional Fresnel effects\n"
        "     6  Diffuse and specular model with ray tracing, reflection map and\n"
        "        refraction\n"
        "     7  Like 6 but with additional Fresnel effects\n"
        "     8  Diffuse and specular model with reflection map\n"
        "     9  Like 8 but with the dissolve factor adjusted to simulate glass\n"
        "     10 Cast shadows onto invisible surfaces\n", f);

  fputs("Switches for illumination models 2..9:\n"
        "  -ks R[,G,B]         Specular reflectivity (R=0..1, G=0..1, B=0..1)\n"
        "                      Default is the same as the ambient colour.\n"
        "                      Green and blue default to red if not specified.\n"
        "  -ns N               Specular exponent (default 200.0)\n", f);

  fputs("Switches for illumination models 3..9:\n"
        "  -sharpness N        Sharpness of reflection map\n"
        "                      (N=0..1000, default 60)\n", f);

  fputs("Switches for illumination models 6..7:\n"
        "  -ni N               Optical density (N=0.001..10, default 1.0)\n"
        "  -tf R[,G,B]         Transmission filter (R=0..1, G=0..1, B=0..1)\n"
        "                      Default is 1.0.\n"
        "                      Green and blue default to red if not specified.\n", f);

  return EXIT_FAILURE;
}

#ifdef FORTIFY
int real_main(int argc, const char *argv[]);

int main(int argc, const char *argv[])
{
  unsigned long limit;
  int rtn = EXIT_FAILURE;
  for (limit = 0; rtn != EXIT_SUCCESS; ++limit)
  {
    rewind(stdin);
    clearerr(stdout);
    printf("------ Allocation limit %ld ------\n", limit);
    Fortify_SetNumAllocationsLimit(limit);
    Fortify_EnterScope();
    rtn = real_main(argc, argv);
    Fortify_LeaveScope();
  }
  return rtn;
}

int real_main(int argc, const char *argv[])
#else
int main(int argc, const char *argv[])
#endif
{
  int n, first = -1, last = -1, illum = 0, sharpness = 60;
  bool time = false, batch = false, raw = false;
  unsigned int flags = 0;
  bool specular = false, reflection_map = false, refraction = false;
  int rtn = EXIT_SUCCESS;
  const char *output_file = NULL, *input_file = NULL;
  double ks[3], ns = 200.0, ni = 1.0, tf[3] = {1.0, 1.0, 1.0}, d = 1.0;
  double (*ksp)[3] = NULL; /* default is to use material colour */

  assert(argc > 0);
  assert(argv != NULL);

  DEBUG_SET_OUTPUT(DebugOutput_Reporter, "");

  /* Parse any options specified on the command line */
  for (n = 1; n < argc && argv[n][0] == '-'; n++) {
    const char *opt = argv[n] + 1;

    if (is_switch(opt, "batch", 1)) {
      /* Enable batch processing mode */
      batch = true;
    } else if (is_switch(opt, "d", 1)) {
      /* Dissolve factor was specified */
      if (!get_double_arg("dissolve factor", &d, 0.0, 1.0, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
    } else if (is_switch(opt, "first", 1)) {
      /* First colour number to convert was specified */
      long int num;
      if (!get_long_arg("first", &num, 0, NColours-1, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      first = (int)num;
    } else if (is_switch(opt, "help", 2)) {
      /* Output usage information */
      (void)syntax_msg(stdout, argv[0]);
      return EXIT_SUCCESS;
    } else if (is_switch(opt, "human", 2)) {
      /* Enable human-readable material names */
      flags |= FLAGS_HUMAN_READABLE | FLAGS_PHYSICAL_COLOUR;
    } else if (is_switch(opt, "illum", 2)) {
      /* Illumination model was specified */
      long int tmp;
      if (!get_long_arg("illumination model", &tmp, 0, 10, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      illum = (int)tmp;
    } else if (is_switch(opt, "index", 2)) {
      /* Colour number to convert was specified */
      long int num;
      if (!get_long_arg("index", &num, 0, NColours-1, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      first = last = (int)num;
    } else if (is_switch(opt, "ks", 1)) {
      /* Specular reflectivity was specified */
      if (++n >= argc) {
         fputs("Missing specular reflectivity value\n", stderr);
         return syntax_msg(stderr, argv[0]);
      } else {
        specular = true;
        ksp = &ks;
        char *endptr;
        ks[0] = strtod(argv[n], &endptr);
        if (*endptr == ',') {
          ks[1] = strtod(endptr + 1, &endptr);
          if (*endptr == ',') {
            ks[2] = strtod(endptr + 1, &endptr);
          } else {
            fputs("Missing specular reflectivity value\n", stderr);
            return syntax_msg(stderr, argv[0]);
          }
        } else {
          ks[1] = ks[0];
          ks[2] = ks[0];
        }
        if (*endptr != '\0') {
          fputs("Unrecognized characters in specular reflectivity value\n",
                stderr);
          return syntax_msg(stderr, argv[0]);
        }
        const double min = 0.0, max = 1.0;
        for (size_t n = 0; n < ARRAY_SIZE(ks); ++n) {
          if ((ks[n] < min) || (ks[n] > max)) {
            fprintf(stderr,
                    "Specular reflectivity value out of range %f .. %f\n",
                    min, max);
            return EXIT_FAILURE;
          }
        }
      }
    } else if (is_switch(opt, "last", 1)) {
      /* Last colour number to convert was specified */
      long int num;
      if (!get_long_arg("last", &num, 0, NColours-1, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      last = (int)num;
    } else if (is_switch(opt, "ni", 2)) {
      /* Optical density was specified */
      refraction = true;
      if (!get_double_arg("optical density", &ni, 0.001, 10, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
    } else if (is_switch(opt, "ns", 2)) {
      /* Specular exponent was specified */
      specular = true;
      if (!get_double_arg("specular exponent", &ns, DBL_MIN, DBL_MAX, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
    } else if (is_switch(opt, "outfile", 1)) {
      /* Output file path was specified */
      if (++n >= argc || argv[n][0] == '-') {
        fputs("Missing output file name\n", stderr);
        return syntax_msg(stderr, argv[0]);
      }
      output_file = argv[n];
    } else if (is_switch(opt, "physical", 1)) {
      /* Enable output of unique physical colours */
      flags |= FLAGS_PHYSICAL_COLOUR;
    } else if (is_switch(opt, "raw", 1)) {
      /* Enable raw input */
      raw = true;
    } else if (is_switch(opt, "sharpness", 1)) {
      /* Sharpness of reflection map was specified */
      reflection_map = true;
      long int tmp;
      if (!get_long_arg("sharpness", &tmp, 0, 1000, argc, argv, ++n)) {
        return syntax_msg(stderr, argv[0]);
      }
      sharpness = (int)tmp;
    } else if (is_switch(opt, "tf", 2)) {
      /* Transmission filter was specified */
      refraction = true;
      if (++n >= argc) {
         fputs("Missing transmission filter value\n", stderr);
         return syntax_msg(stderr, argv[0]);
      } else {
        char *endptr;
        tf[0] = strtod(argv[n], &endptr);
        if (*endptr == ',') {
          tf[1] = strtod(endptr + 1, &endptr);
          if (*endptr == ',') {
            tf[2] = strtod(endptr + 1, &endptr);
          } else {
            fputs("Missing transmission filter value\n", stderr);
            return syntax_msg(stderr, argv[0]);
          }
        } else {
          tf[1] = tf[0];
          tf[2] = tf[0];
        }
        if (*endptr != '\0') {
          fputs("Unrecognized characters in transmission filter value\n",
                stderr);
          return syntax_msg(stderr, argv[0]);
        }
        const double min = 0.0, max = 1.0;
        for (size_t n = 0; n < ARRAY_SIZE(ks); ++n) {
          if ((tf[n] < min) || (tf[n] > max)) {
            fprintf(stderr,
                    "Transmission filter value out of range %f .. %f\n",
                    min, max);
            return EXIT_FAILURE;
          }
        }
      }
    } else if (is_switch(opt, "time", 2)) {
      /* Enable timing */
      time = true;
    } else if (is_switch(opt, "verbose", 1) || is_switch(opt, "debug", 2)) {
      /* Enable debugging output */
      flags |= FLAGS_VERBOSE;
    } else {
      fprintf(stderr, "Unrecognised switch '%s'\n", opt);
      return syntax_msg(stderr, argv[0]);
    }
  }

  if ((first > last) && (last >= 0)) {
    fputs("First colour number must not exceed last colour number\n", stderr);
    return EXIT_FAILURE;
  }
  if (first == -1) {
    first = 0;
  }

  if (specular && ((illum < 2) || (illum > 9))) {
    fprintf(stderr, "Illumination model %d does not allow specular reflectivity\n", illum);
    return syntax_msg(stderr, argv[0]);
  }

  if (reflection_map && ((illum < 3) || (illum > 9))) {
    fprintf(stderr, "Illumination model %d does not allow a reflection map\n",
            illum);
    return syntax_msg(stderr, argv[0]);
  }

  if (refraction && ((illum < 6) || (illum > 7))) {
    fprintf(stderr, "Illumination model %d does not allow refraction\n", illum);
    return syntax_msg(stderr, argv[0]);
  }

  if (batch) {
    if (output_file != NULL) {
      fputs("Cannot specify an output file in batch processing mode\n", stderr);
      return syntax_msg(stderr, argv[0]);
    }
    if (n >= argc) {
      fputs("Must specify file(s) in batch processing mode\n", stderr);
      return syntax_msg(stderr, argv[0]);
    }
  } else {
    /* If an input file was specified, it should follow the switches */
    if (n < argc) {
      input_file = argv[n++];
    }

    /* An output file name may follow the input file name, but only if not
       already specified */
    if (n < argc) {
      if (output_file != NULL) {
        fputs("Cannot specify more than one output file\n", stderr);
        return syntax_msg(stderr, argv[0]);
      }
      output_file = argv[n++];
    }

    if (output_file == NULL && (time || (flags & FLAGS_VERBOSE))) {
      fputs("Must specify an output file in verbose/timer mode\n", stderr);
      return syntax_msg(stderr, argv[0]);
    }

    if (n < argc) {
      fputs("Too many arguments (did you intend -batch?)\n", stderr);
      return syntax_msg(stderr, argv[0]);
    }
  }

  if (flags & FLAGS_VERBOSE) {
    printf("Star Fighter 3000 to Wavefront mtl convertor, "VERSION_STRING"\n"
           "Copyright (C) 2016, Christopher Bazley\n");
  }

  if (batch) {
    /* In batch processing mode, there remaining arguments are treated as a
       list of file names (output to default file names) */
    for (; n < argc && rtn == EXIT_SUCCESS; n++) {
      /* Invent an output file name */
      assert(argv[n] != NULL);
      StringBuffer default_output;
      stringbuffer_init(&default_output);
      if (!stringbuffer_append(&default_output, argv[n], SIZE_MAX) ||
          !stringbuffer_append_separated(&default_output, EXT_SEPARATOR, "mtl")) {
        fprintf(stderr, "Failed to allocate memory for output file path\n");
        rtn = EXIT_FAILURE;
      } else if (!process_file(argv[n], stringbuffer_get_pointer(&default_output),
                               first, last, d, illum, ksp, ns, sharpness, ni,
                               &tf, flags, time, raw)) {
        rtn = EXIT_FAILURE;
      }
      stringbuffer_destroy(&default_output);
    }
  } else {
    if (!process_file(input_file, output_file, first, last, d, illum, ksp, ns,
                      sharpness, ni, &tf, flags, time, raw)) {
      rtn = EXIT_FAILURE;
    }
  }

  return rtn;
}