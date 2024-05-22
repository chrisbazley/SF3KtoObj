/*
 *  SF3KtoObj - Converts Star Fighter 3000 graphics to Wavefront format
 *  Definitions of data formats for the game 'Star Fighter 3000'
 *  Copyright (C) 2003 Christopher Bazley
 */

#ifndef SFFORMATS_H
#define SFFORMATS_H

#include <stdint.h>

/* At the start of the file are command sequences which control plotting of
   complex objects (i.e. those for which (plot_type_and_last_group &
   SFObject_PlotTypeMask) != 0. Each command sequence is terminated by
   SFPlotCommands_EndOfType. A command may be one or two bytes long. Bits
   5-7 of the first byte specify a condition to control whether a group of
   facets should be plotted and, if so, whether facets should then be culled
   individually. Bits 0-4 encode an operand which is usually the vector test
   upon which plotting is predicated. The following byte usually gives the
   group number to be plotted if the vector test passes. Commands with action
   SFPlotAction_FacingAlways are instead encoded as a single byte: bits 0-4
   give the group to be plotted. The end of the commands data is marked by
   SFPlotCommands_EndOfData. */

#define SFPlotCommands_OperandMask 0x1fu
#define SFPlotCommands_ActionMask  0xe0u

enum
{
  SFPlotCommands_OperandShift = 0,
  SFPlotCommands_ActionShift  = 5,
  SFPlotCommands_EndOfType    = 255, /* Followed by data for next plot type,
                                        or SFPlotCommands_EndOfData */
  SFPlotCommands_EndOfData    = 254 /* Indicates no more plot types */
};

typedef enum
{
  SFPlotAction_FacingAlways, /* Always plot facing facets in the group specified
                                by bits 0-4. */
  SFPlotAction_FacingIf,     /* Plot facing facets in the group specified by the
                                next byte if the vector test specified by bits
                                0-4 passes. */
  SFPlotAction_FacingIfNot,  /* Plot facing facets in the group specified by the
                                next byte if the vector test specified by bits
                                0-4 fails. */
  SFPlotAction_AllIf,        /* Plot all facets in the group specified by the
                                next byte if the vector test specified by bits
                                0-4 passes. */
  SFPlotAction_AllIfNot      /* Plot all facets in the group specified by the
                                next byte if the vector test specified by bits
                                0-4 fails. */
}
SFPlotAction;

typedef enum
{
  SFObjectType_Ground, /* Ground objects (trees, buildings) */
  SFObjectType_Bit,    /* (Broken?) bits */
  SFObjectType_Aerial  /* Aerial things (fighters, coins, missiles)*/
}
SFObjectType;

#define SFObjectCollisionSize_YMask 0x0fu
#define SFObjectCollisionSize_XMask 0xf0u

enum
{
  SFObjectCollisionSize_YShift = 0,
  SFObjectCollisionSize_XShift = 4,
};

#define SFObject_PlotTypeMask 0x0fu
#define SFObject_LastGroupMask 0xf0u

enum
{
  SFObject_PlotTypeShift = 0,
  SFObject_LastGroupShift = 4
};

typedef enum
{
  SFVertexCoord_SubMul32 = 85,
  SFVertexCoord_SubMul16,
  SFVertexCoord_SubMul8,
  SFVertexCoord_SubMul4,
  SFVertexCoord_SubMul2,
  SFVertexCoord_SubUnit, /* subtract unit vector from previous coordinate */
  SFVertexCoord_SubDiv2 = 96,
  SFVertexCoord_SubDiv4,
  SFVertexCoord_SubDiv8,
  SFVertexCoord_SubDiv16,
  SFVertexCoord_Zero,    /* no change from previous coordinate */
  SFVertexCoord_AddDiv16,
  SFVertexCoord_AddDiv8,
  SFVertexCoord_AddDiv4,
  SFVertexCoord_AddDiv2,
  SFVertexCoord_AddUnit = 110, /* add unit vector to previous coordinate */
  SFVertexCoord_AddMul2,
  SFVertexCoord_AddMul4,
  SFVertexCoord_AddMul8,
  SFVertexCoord_AddMul16,
  SFVertexCoord_AddMul32,
}
SFVertexCoord;

typedef enum
{
  SFCoordinateScale_Small,
  SFCoordinateScale_Medium,
  SFCoordinateScale_Large
}
SFCoordinateScale;

#define SFObjectFacet_NumSidesMask  0x0fu
#define SFObjectFacet_GroupMask     0x70u
#define SFObjectFacet_SpecialColour 0x80u

enum
{
  SFObjectFacet_NumSidesShift = 0,
  SFObjectFacet_GroupShift    = 4,
  SFObjectFacet_VectorsGroup  = 7
};

/* The graphics data for the next polygonal object follows 4 bytes after the
   last collision box for the preceding object (no obvious reason for this gap).
   The address should already be word aligned. If there are no more objects
   then the next word (i.e. at +8) will be 99. */

enum
{
  SFObjects_EndOfData = 99
};

typedef union
{
  /* Plain and simple */
  uint8_t colour_mappings[320];

  /* Area allocations */
  struct
  {
    uint8_t static_colours[256];
    struct
    {
      uint8_t player_engine[4];
      uint8_t fighter_engine[4];
      uint8_t cruiser_engine[4];
      uint8_t super_engine[4];
    }
    engine_colours;
    struct
    {
      uint8_t enemy_ships[4];
      uint8_t friendly_ships[4];
      uint8_t player_ship[4];
    }
    fast_flashing;
    struct
    {
      uint8_t ground_obj_1[4];
      uint8_t ground_obj_2[4];
      uint8_t misc_1[4];
      uint8_t misc_2[4];
    }
    med_flashing;
    uint8_t player_livery[20];
  }
  areas;
}
SFObjectColours;

#endif /* SFFORMATS_H */
