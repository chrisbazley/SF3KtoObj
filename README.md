# SF3KtoObj
Star Fighter 3000 to Wavefront convertor

(C) Christopher Bazley, 2016

Version 0.10 (13 Apr 2025)


-----------------------------------------------------------------------------
 1   Introduction and Purpose
-----------------------------

  These two command-line programs can be used to convert object meshes and
colour palettes belonging to the 32-bit Acorn RISC OS game 'Star Fighter
3000' from their original format into the simple object geometry and
material formats developed by Wavefront for their Advanced Visualizer
software. Wavefront OBJ format is a de-facto standard for 3D computer
graphics and it has the advantage of being human-readable.

  Different missions in the game use different palettes, for example
'Strategic Bomber Test' (M9) colours the player's ship in yellow, ruby and
scarlet instead of its normal livery of fern green.

  The colour and other visual properties of objects are defined separately
from the object geometry (OBJ) file in a companion file known as a material
library (MTL) file. The supplied MTL file defines all of the colours in the
default RISC OS 256-colour palette as named materials, assuming a constant
colour illumination model (no texture map, ambient or specular reflectance).
Alternatively, the game's mission-specific palettes can be converted to MTL
files.

-----------------------------------------------------------------------------
2   Requirements
----------------

  The supplied executable files will only work on RISC OS machines. They have
not been tested on any version of RISC OS earlier than 4, although they
should also work on earlier versions provided that a suitable version of the
'SharedCLibrary' module is active. They should be compatible with 32 bit
versions of RISC OS.

  You will obviously require a copy of the game 'Star Fighter 3000' from
which to rip the graphics. Object definition files can be found inside the
!Star3000 application, in !Star3000.LandScapes.Graphics; palette files can
be found in !Star3000.LandScapes.Palette. Depending on your version of the
game, the LandScapes directory might be inside another named 'Disc2'.

-----------------------------------------------------------------------------
3   Quick Guide
---------------

  Ensure that the !Star3000 application has been 'seen' by the Filer, so that
the system variable Star3000$Dir has been set.

  Hold down the Shift key and double-click on the !Star3000 application to
open it.

  Set the current selected directory to that containing the 'SF3KtoObj'
program. On RISC OS 6, that can be done by opening the relevant directory
display and choosing 'Set work directory' from the menu (or giving the
directory display the input focus and then pressing F11).

  Press Ctrl-F12 to open a task window and then invoke the conversion program
using the following command:
```
  *SF3KtoObj -list <Star3000$Dir>.Landscapes.Graphics.Earth1
```
  This should list all of the object definitions within the compressed
graphics file 'Earth1'. One of those should be named 'player', which is the
definition of the 'Predator Mk-IV' combat fighter normally flown by the
player.

  To convert the 'Predator Mk-IV' object mesh into a Wavefront OBJ file named
'player/obj', use the following command:
```
  *SF3KtoObj -name player -palette <Star3000$Dir>.LandScapes.Palette.Default <Star3000$Dir>.LandScapes.Graphics.Earth1 player/obj
```
  The '-name' argument selects which object is converted and the '-palette'
argument selects which of the five palettes supplied with the game is used
for the conversion. The 'Default' palette applies training colours (fern
green with teal and blue stripes) to the 'Predator Mk-IV'.

  By varying the arguments to the 'SF3KtoObj' program, any of the other
files in the !Star3000.LandScapes.Graphics directory can be converted to
Wavefront OBJ format.

-----------------------------------------------------------------------------
4   Common usage information
----------------------------

4.1 Command line syntax
-----------------------
  The basic syntax of both programs is the same, as summarised below. They
have two principle modes of operation: single file mode and batch processing.
All switches are optional, but some are incompatible with each other (e.g.
'-batch' with '-outfile'). Switch names may be abbreviated provided that
doing so does not make the command ambiguous.
```
usage: SF3KtoObj [switches] [<input-file> [<output-file>]]
or     SF3KtoObj -batch [switches] <file1> [<file2> .. <fileN>]
```

4.2 Input and output
--------------------
Switches:
```
  -batch              Process a batch of files (see above)
  -raw                Input is uncompressed raw data
  -outfile <file>     Write output to the named file instead of stdout
```

  Single file mode is the default mode of operation. Unlike batch mode, the
input and output files can be specified separately. An output file name can
be specified after the input file name, or before it using the '-outfile'
parameter.

  If no input file is specified then input is read from 'stdin' (the
standard input stream; keyboard unless redirected). If no output file is
specified then output is written to 'stdout' (the standard output stream;
screen unless redirected).

  All of the following examples read input from a file named 'foo' and
write to a file named 'bar':
```
  *SF3KtoObj foo bar
  *SF3KtoObj -outfile bar foo
  *SF3KtoObj -outfile bar <foo
  *SF3KtoObj foo >bar
```

  Under UNIX-like operating systems, output can be piped directly into
another program.

  Search for a named object in a compressed graphics file named 'foo':
```
  SF3KtoObj foo | grep -i 'bit_10'
```

  Batch mode is enabled by the switch '-batch'. In this mode, multiple
files can be processed using a single command. The output is always saved to
a file with a name derived from the input file's name, which means that
programs cannot be chained using pipes. At least one file name must be
specified.

  Convert compressed graphics files named 'foo', 'bar' and 'baz' to
Wavefront OBJ files named 'foo/obj', 'bar/obj' and 'baz/obj':
```
  *SF3KtoObj -batch foo bar baz
```
  By default, all input is assumed to be compressed. The switch '-raw'
allows uncompressed input, which may be useful if input has already been
decompressed.

  It isn't possible to mix compressed and uncompressed input, for example by
using compressed graphics data with an uncompressed palette file.

4.3 Getting diagnostic information
----------------------------------
Switches:
```
  -time               Show the total time for each file processed
  -verbose or -debug  Emit debug information (and keep bad output)
```

  If either of the switches '-verbose' and '-debug' is used then the two
programs emit information about their internal operation on the standard
output stream. However, this makes them slower and prevents output being
piped to another program.

  If the switch '-time' is used then the total time for each file processed
(to centisecond precision) is printed. This can be used independently of
'-verbose' and '-debug'.

  When debugging output or the timer is enabled, you must specify an output
file name. This is to prevent the MTL or OBJ format output being sent to the
standard output stream and becoming mixed up with the diagnostic information.

-----------------------------------------------------------------------------
5   SF3KtoObj usage information
-------------------------------

5.1 Object selection
--------------------
```
  -index N     Object number to convert or list (default is all)
  -first N     First object number to convert or list
  -last N      Last object number to convert or list
  -type G|B|S  Object type to convert or list (default is all)
  -name <name> Object name to convert or list (default is all)
```

  The contents of a graphics file can be filtered using the '-index' or
'-first' and '-last' parameters to select a single object or range of
objects to be processed. Using the '-index' parameter is equivalent to
setting the first and last numbers to the same value. The lowest object
number is 0.

  A file can also be filtered using the '-type' and '-name' parameters to
select single objects or types of object to be processed. Any filter
specified applies when listing object models as well as when converting
them.

  If no range of object numbers, type or name is specified then all objects
defined in the input file are processed.

  If an object type is specified then only objects of the given type are
processed. The three types of object are [S]hip, [G]round and [B]it.

  Convert all ship objects in file 'Earth1':
```
  *SF3KtoObj -type S <Star3000$Dir>.LandScapes.Graphics.Earth1
```
  Convert all ground objects in file 'CyberZ':
```
  *SF3KtoObj -type G <Star3000$Dir>.LandScapes.Graphics.CyberZ
```
  If an object index is specified then the specified number of object
definitions are skipped before processing a single object with the given
index (counting upwards from 0).

  Convert the first object of any type in file 'Earth2', outputting to the
screen:
```
  *SF3KtoObj -index 0 <Star3000$Dir>.LandScapes.Graphics.Earth2
```
  If both an object index and an object type are specified then the index is
interpreted differently: objects of other types are not counted when
searching for the object with the given index.

  Specifying both '-type' and '-index' is usually more useful than only
specifying '-index' because the game only treats the order of object
definitions as significant relative to other objects of the same type.

  Convert the eighth ground object in file 'Earth2' (see below for object
numbering):
```
  *SF3KtoObj -type G -index 7 <Star3000$Dir>.LandScapes.Graphics.Earth2
```

  Convert the fifteenth ship object in file 'Earth2', outputting to the
screen:
```
  *SF3KtoObj -type S -index 14 <Star3000$Dir>.LandScapes.Graphics.Earth2
```
  If an object name is specified then the single object of the given name
is processed (provided that it falls within the specified range of object
numbers, if any).

  Convert the mothership object in file 'Warrior' (see below for object
naming):
```
  *SF3KtoObj -name mothership <Star3000$Dir>.LandScapes.Graphics.Warrior
```

  Convert one of the two hangar objects in file 'Warrior', outputting to the
screen:
```
  *SF3KtoObj -name hangar_2 <Star3000$Dir>.LandScapes.Graphics.Warrior
```

Ground objects:

| Number | Name     | Object
|--------|----------|--------------------------------
|      0 | none     | None
|      1 | gun_1    | Ground gun
|      2 | gun_2    | Ground gun
|      3 | gun_3    | Ground gun
|      4 | sam_1    | Surface-to-air missile launcher
|      5 | sam_2    | Surface-to-air missile launcher
|      6 | sam_3    | Surface-to-air missile launcher
|      7 | hangar_1 | Hangar
|      8 | hangar_2 | Hangar

Ship objects:

| Number | Name        | Object
|--------|-------------|-------------------------------
|      0 | player      | Player's ship
|      1 | fighter_1   | Fighter
|      2 | fighter_2   | Fighter
|      3 | fighter_3   | Fighter
|      4 | fighter_4   | Fighter
|      5 | three_coin  | Bonus coin (3 credits)
|      6 | ten_coin    | Bonus coin (10 credits)
|      7 | life_coin   | Bonus coin (extra life)
|      8 | fifty_coin  | Bonus coin (50 credits)
|      9 | twenty_coin | Bonus coin (20 credits)
|     10 | atg_coin    | Bonus coin (10 A-T-G missiles)
|     11 | ata_coin    | Bonus coin (10 A-T-A missiles)
|     12 | damage_coin | Damage coin
|     13 | big_ship_1  | Big ship
|     14 | mothership  | Mothership
|     15 | big_ship_2  | Big ship
|     16 | atg_missile | A-T-G missile
|     17 | ata_missile | A-T-A missile
|     18 | mine        | Aerial mine
|     19 | bomb        | Freefall bomb
|     20 | parachute   | Parachute
|     21 | satellite   | Satellite
|     22 | dock        | Docking bay

5.2 Listing and summarizing objects
-----------------------------------
  -list     List objects instead of converting them
  -summary  Summarize objects instead of converting them

  If the switch '-list' is used then SF3KtoObj lists object definitions
found in the specified input file(s) instead of converting them to Wavefront
OBJ format. Only object models matching any filter specified using the
'-index', '-first', '-last', '-type' and '-name' parameters are listed.

  No output file can be specified because no OBJ-format output is generated
and the object list or summary is always sent to the standard output stream.

  List all object definitions in file 'Chemical':
```
  *SF3KtoObj -list <Star3000$Dir>.LandScapes.Graphics.Chemical
```

  List all ship objects in file 'CyberZ':
```
  *SF3KtoObj -list -type s <Star3000$Dir>.LandScapes.Graphics.CyberZ
```

  Output is a table with the following format:
```
Index  Type    Index  Name          Verts  Faces      Offset        Size
    0  Bit         0  bit_0             8      6         100         168
```
  If the switch '-summary' is used then SF3KtoObj summarizes object
definitions found in the specified input file(s) instead of converting them
to Wavefront OBJ format. This switch can be used in conjunction with '-list',
in which case the summary is displayed after the list.

  Summarize all object definitions in file 'Earth1':
```
  *SF3KtoObj -summary <Star3000$Dir>.LandScapes.Graphics.Earth1
```

  Output is a list of totals in the following format:
```
Found 69 object definitions, comprising:
  34 Ground objects
  12 Bit objects
  23 Ship objects
```

5.3 Palettes and materials
--------------------------
```
  -mtllib name   Specify a material library file (default sf3k.mtl)
  -palette name  Specify a palette file in which to look up
                 physical colours (default is none)
  -human         Output readable material names (needs -palette)
  -false         Assign false colours for visualization
```

  If no palette file is specified then SF3KtoObj emits 'usemtl' commands
that refer to logical colours by derived material names such as 'colour_31'.
The advantage of this mode of operation is that palette swapping can be done
simply by amending the name of the MTL file referenced by the OBJ file.

  Convert the player's ship in file 'Earth1', outputting logical colour
numbers to the screen:
```
  *SF3KtoObj -type s -index 0 <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
g player player_0
usemtl colour_302
f  23 22 21 44
usemtl colour_304
f  23 24 1 22
f  44 21 20 43
usemtl colour_316
f  49 46 47 48
...
```
  If a palette file is specified then SF3KtoObj uses it to look up the
physical colour associated with each logical colour. An advantage of doing
this look-up during conversion is that all OBJ files can use the same MTL
file, which need only define the standard 256 RISC OS colours.

  Convert the player's ship in file 'Earth1', specifying palette file
'RedShip' and outputting physical colour numbers to the screen:
```
  *SF3KtoObj -type s -index 0 -palette <Star3000$Dir>.LandScapes.Palette.RedShip <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
g player player_0
usemtl riscos_119
f 23 22 21 44
usemtl riscos_30
f 23 24 1 22
f 44 21 20 43
usemtl riscos_200
f 49 46 47 48
...
```

  If the switch '-human' is used then SF3KtoObj goes one step further and
refers to physical colours by human-readable material names such as
'black_0'. This can only be enabled if a palette file is specified.

  Convert the player's ship in file 'Earth1', outputting human-readable
colour names to the screen:
```
  *SF3KtoObj -type s -index 0 -human -palette <Star3000$Dir>.LandScapes.Palette.RedShip <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
g player player_0
usemtl peridot_3
f 23 22 21 44
usemtl crimson_2
f 23 24 1 22
f 44 21 20 43
usemtl honolulublue_0
f 49 46 47 48
...
```

  By default, SF3KtoObj emits a 'mtllib' command which references 'sf3k.mtl'
as the material library file to be used when drawing objects; this is the
same as the name of the supplied MTL file.

  An alternative material library file can be specified using the switch
'-mtllib'. The named file is not created, read or written by SF3KtoObj.

  False colours can be assigned to help visualise boundaries between
polygons, especially between coplanar polygons of the same colour. This
mode, which is mainly useful for debugging, is enabled by specifying the
switch '-false'.

  When false colours are enabled, the logical colours in the input file are
ignored. If a palette file was specified then its content is ignored too.

5.4 Animation
-------------
```
  -frame N  Animation frame to convert (default is 0)
```
  Logical colours in the range 256..283 are animated every frame and those
in the range 284..299 are animated every second frame. Groups of four
physical colours are rotated in the palette, giving a total of 11
animations. The faster animations are used for spacecraft engines and
lights; the slower ones for ground installations.

  Certain objects include sets of vertices that rotate anti-clockwise (as
viewed from above) at a rate of 5.625 degrees or 1/64th of a full circle
per frame. An example is the 'hangar_2' object in files 'Earth1', 'Earth2'
and 'Warrior'.

  If the parameter '-frame' is used then the following argument controls
which animation frame is converted. The default value is 0, which is the
first frame. If a palette file was specified then different physical
colours are looked up in the palette; either way, the 'usemtl' commands
in the output are likely to differ. Rotating vertices are also affected.

  Convert animation frame 0 of a radar installation in file 'Earth1':
```
  *SF3KtoObj -name ground_26 -frame 0 <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
...
v 16.000000 16.000000 -128.000000
v 16.000000 -16.000000 -128.000000
v 0.000000 0.000000 -128.000000
# Following vertices rotate
v 16.000000 -16.000000 -128.000000
v 16.000000 16.000000 -128.000000
v -16.000000 16.000000 -128.000000
...
```

  Convert animation frame 1 of the same object:
```
  *SF3KtoObj -name ground_26 -frame 1 <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
...
v 16.000000 16.000000 -128.000000
v 16.000000 -16.000000 -128.000000
v 0.000000 0.000000 -128.000000
# Following vertices rotate
v 14.354681 -17.491230 -128.000000
v 17.491230 14.354681 -128.000000
v -14.354681 17.491230 -128.000000
...
```
5.5 Clipping
------------
```
  -clip  Clip overlapping coplanar polygons
```
  Some objects are liable to suffer from a phenomenon known as "Z-fighting"
if they are part of a scene rendered using a depth (Z) buffer. It is caused
by overlapping faces in the same geometric plane and typically manifests as
a shimmering pattern in a rendered image. Essentially two or more polygons
occupy the same points in 3D space.

  The game uses painter's algorithm to ensure that overlapping objects are
drawn correctly (drawing more distant objects before nearer ones), instead
of using a depth buffer. It also draws the polygons of each object in a
predictable order, which allows overlapping polygons to be used as decals
(e.g. as doors and windows on buildings).

  If the switch '-clip' is used then the rearmost of two overlapping polygons
is split by dividing it along the line obtained by extending one edge of the
front polygon to infinity in both directions. This process is repeated until
no two edges intersect. Any polygons that are fully hidden (typically behind
decals) are deleted.

  The following diagrams illustrate how one polygon (B: 1 2 3 4) overlapped
by another (A: 5 6 7 8) is split into five polygons (B..F) during the
clipping process. The last polygon (F) is then deleted because it duplicates
the overlapping polygon (A).

```
     Original         First split       Second split
  (A overlaps B)     (A overlaps C)    (A overlaps D)
                               :
  3_____________2   3__________9__2   3__________9__2
  |      B      |   |          |  |   |      C   |  |
  |  7_______6  |   |          |  | ..|__________6..|..
  |  |   A   |  |   |      C   |B | 11|          |B |
  |  |_______|  |   |          |  |   |      D   |  |
  |  8       5  |   |          |  |   |          |  |
  |_____________|   |__________|__|   |__________|__|
  4             1   4          10 1   4          10 1
                               :
    Third split       Fourth split     Final clipped
  (A overlaps E)     (A overlaps F)     (F deleted)
  3__:_______9__2   3__________9__2   3__________9__2
  |  :   C   |  |   |      C   |  |   |      C   |  |
11|__7_______6  | 11|__7_______6  | 11|__7_______6  |
  |  |       |B |   |D |   F   |B |   |D |   A   |B |
  |D |   E   |  | ..|..|_______|..|.. |  |_______|  |
  |  |       |  |   |  8   E   5  |   |  8   E   5  |
  |__|_______|__|   |__|_______|__|   |__|_______|__|
  4  12      10 1   4  12      10 1   4  12      10 1
     :
```

5.6 Output of faces
-------------------
```
  -fans      Split complex polygons into triangle fans
  -strips    Split complex polygons into triangle strips
  -negative  Use negative vertex indices
```
  The Wavefront OBJ format specification does not restrict the maximum
number of vertices in a face element. Nevertheless, some programs cannot
correctly display faces with more than three vertices. Two switches, '-fans'
and '-strips', are provided to split complex polygons into triangles.

```
       Original          Triangle fans       Triangle strips
     3_________2          3_________2          3_________2
     /         \          / `-._    \          /\`-._    \
    /           \        /      `-._ \        /  `\  `-._ \
 4 /             \ 1  4 /___________`-\ 1  4 /     \.    `-\ 1
   \             /      \         _.-`/      \`-._   \     /
    \           /        \    _.-`   /        \   `-. \.  /
     \_________/          \.-`______/          \_____`-.\/
     5         6          5         6          5         6

 f 1 2 3 4 5 6        f 1 2 3              f 1 2 3
                      f 1 3 4              f 6 1 3
                      f 1 4 5              f 6 3 4
                      f 1 5 6              f 5 6 4
```

  Vertices in a face element are normally indexed by their position in the
output file, counting upwards from 1. If the output comprises more than one
object definition then it can be more useful to count backwards from the
most recent vertex definition, which is assigned index -1. The '-negative'
switch enables this output mode, which allows object models to be
separated, extracted or rearranged later.

  Convert the mothership in file 'Earth1' with positive vertex indices:
```
  *SF3KtoObj -type s -index 14 <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
g mothership mothership_0
usemtl colour_44
f 29 32 31 30
usemtl colour_45
f 1 2 32 29
...
```

  Convert the same object with negative vertex indices:
```
  *SF3KtoObj -type s -index 14 -negative <Star3000$Dir>.LandScapes.Graphics.Earth1
```

```
g mothership mothership_0
usemtl colour_44
f -67 -64 -65 -66
usemtl colour_45
f -95 -94 -64 -67
...
```

5.7 Hidden data
---------------
```
  -hidden     Include hidden polygons in the output
  -unused     Include unused vertices in the output
  -duplicate  Include duplicate vertices in the output
```
  'Star Fighter 3000' groups polygons belonging to complex objects to allow
it to determine more efficiently which face the camera and which do not.
Complex objects have a non-zero plot type. Groups are only drawn if a
surface normal designated by the plot type of the object points towards (or
away from) the viewer. The surface normals to be tested must belong to
polygons in group 7.

  If the switch '-hidden' is used then SF3KtoObj outputs groups which are
hidden because they are not referenced by an object's plot type
definition. It does not actually compute any surface normals to determine
visibility of groups.

  'Star Fighter 3000' specifies vertex coordinates using logarithmic offsets
from the coordinates of a previous vertex. This encoding scheme may require
intermediate vertices to reach the desired coordinates. Such vertices are not
referenced by polygons and therefore they are not included in the output
unless the '-unused' switch is used.

  Some object models also include two or more vertices with the same
coordinates, where both vertexes are referenced by polygons. Examples include
vertex 111/117 of big_ship_1 in file 'Chemical' and vertex 49/57 of fighter_3
in 'Earth1'. Such pairs of vertices are automatically merged unless the
'-duplicate' switch is specified.

-----------------------------------------------------------------------------
6   SF3KtoMtl usage information
-------------------------------

6.1 Colour selection
--------------------
```
  -index N  Logical colour to convert (N=0..319, default all)
  -first N  First logical colour to convert
  -last N   Last logical colour to convert
```

  The contents of a palette file can be filtered using the '-index' or
'-first' and '-last' parameters to select a range of logical colours to be
processed. Using the '-index' parameter is equivalent to setting the first
and last numbers to the same value.

  If no range of indices is specified then all logical colours defined in the
input file are processed.

  The following example converts only the last logical colour in the default
palette:
```
  *SF3KtoMtl -index 319 <Star3000$Dir>.LandScapes.Palette.Default
```

```
newmtl colour_319
# grey tint 2
Kd 0.666667 0.666667 0.666667
illum 0
```

6.2 Material names
------------------
```
  -physical  Output unique physical colours as materials
  -human     Output readable material names (implies -physical)
```
  By default, SF3KtoMtl emits one 'newmtl' command for each of the 320
logical colours, giving each a derived material name such as 'colour_31'.
This mode of operation is designed to complement SF3KtoObj when invoked
without a palette file. The palette can then be switched at any time by
referencing a different MTL file from the OBJ file.

  If the switch '-physical' is used then SF3KtoMtl instead uses the
physical colour associated with each logical colour to generate a material
name. The physical colour numbers refer to the standard RISC OS 256 colour
palette. This mode of operation is designed to complement SF3KtoObj when
invoked with a palette file. The palette is fixed when the OBJ file is
generated.

  Convert all unique physical colours in file 'FastShip':
```
  *SF3KtoMtl -physical <Star3000$Dir>.LandScapes.Palette.FastShip
```

```
newmtl riscos_0
# black tint 0
Kd 0.000000 0.000000 0.000000
illum 0

newmtl riscos_1
# black tint 1
Kd 0.066667 0.066667 0.066667
illum 0
...
```

  If the switch '-human' is used then SF3KtoMtl generates a human-readable
material name such as 'black_0' for each unique physical colour. This switch
implies '-physical', which needn't be passed explicitly.

  Convert all physical colours in file 'FastShip', generating human-readable
material names:
```
  *SF3KtoMtl -human <Star3000$Dir>.LandScapes.Palette.FastShip
```

```
newmtl black_0
Kd 0.000000 0.000000 0.000000
illum 0

newmtl black_1
Kd 0.066667 0.066667 0.066667
illum 0
...
```

  It does not really matter which palette file is used as input to SF3KtoMtl
when the switch '-physical' or '-human' is used because every game palette
maps at least one logical colour to every available physical colour.
SF3KtoMtl never outputs duplicate material definitions, so it always
defines exactly 256 materials; one for each physical colour.

6.3 Material properties
-----------------------
```
  -d N                Dissolve factor for transparency
                      (N=0..1, default 1)
  -illum N            Which illumination model to use
                      (N=0..10, default 0)
     0  Constant colour model
     1  Diffuse model
     2  Diffuse and specular model
     3  Diffuse and specular model with ray tracing and reflection map
     4  Like 3 but with the dissolve factor adjusted to simulate glass
     5  Like 3 but with additional Fresnel effects
     6  Diffuse and specular model with ray tracing, reflection map and
        refraction
     7  Like 6 but with additional Fresnel effects
     8  Diffuse and specular model with reflection map
     9  Like 8 but with the dissolve factor adjusted to simulate glass
     10 Cast shadows onto invisible surfaces
Switches for illumination models 2..9:
  -ks R[,G,B]         Specular reflectivity (R=0..1, G=0..1, B=0..1)
                      Default is the same as the ambient colour.
                      Green and blue default to red if not specified.
  -ns N               Specular exponent (default 200.0)
Switches for illumination models 3..9:
  -sharpness N        Sharpness of reflection map
                      (N=0..1000, default 60)
Switches for illumination models 6..7:
  -ni N               Optical density (N=0.001..10, default 1.0)
  -tf R[,G,B]         Transmission filter (R=0..1, G=0..1, B=0..1)
                      Default is 1.0.
                      Green and blue default to red if not specified.
```
  These switches are for advanced users only and map directly to commands in
the output MTL file. They are best understood by reading the Wavefront
documentation.

-----------------------------------------------------------------------------
7   Colour names
----------------

  The colour names were taken from a variety of online sources including
Wikipedia articles, W3C standards, the X Window System, and the results of
the web comic XKCD's colour naming survey.

  On RISC OS, the default 256 colour palette is generated by using two bits
of the colour index to encode each component (red, green or blue), with two
additional 'tint' bits shared between all components (2+2+2+2=8 bits per
pixel). Mixing white into a colour by setting tint bits increases its value
(aka brightness) but reduces its saturation. Consequently the palette has a
characteristic lack of saturated colours.

  In natural language, the word 'purple' is used to describe many colours of
the same hue and saturation but very different brightness (e.g. the X11
colours 'magenta', 'darkmagenta' and 'purple'). Consequently a weighted
least-squares algorithm such as that used by the ColourTrans module is ill-
suited to finding matching colour names. Normalizing the {red, green, blue}
vector for each palette entry and named colour before comparing them
produces better results; a weighted sum of the unit vector difference and
vector length difference (70:30) ensures that the name closest to the
desired brightness is still chosen.

  Initially, I tried to find the named colour closest to each palette entry
and discovered that tints of the same primary colour had unrelated names or
the gradation from dark to bright tints was unclear. I therefore abandoned
my original list of 256 colour names and instead generated names for each of
the 64 fully-saturated colours (tint 0).

  Colours 16, 128 and 144 are named using the short X11 names 'maroon',
'navy' and 'purple' instead of the slightly better-matching X11 names
'darkred', 'darkblue' and 'darkmagenta'. That is to avoid confusion between
those colours and the even darker colours 4, 8 and 12. Colours 20 and 96 are
not actual X11 colour names but were named in the same style as 'mediumblue'
so that all three primaries at the same brightness have similar names.

  The final colours are listed below:

| Index | Web RGB | Name                 | Notes                         |
|-------|---------|----------------------|-------------------------------|
|     0 | #000000 | black              * | Identical to HTML 'black'     |
|     4 | #440000 | darkmaroon           | By analogy with 'darkred'     |
|     8 | #000044 | darknavy             | By analogy with 'darkblue'    |
|    12 | #440044 | darkpurple           | By analogy with 'darkmagenta' |
|    16 | #880000 | maroon             * | Like HTML colour #800000      |
|    20 | #cc0000 | mediumred            | By analogy with 'mediumblue'  |
|    24 | #880044 | tyrianpurple         | Like Wikipedia colour #66023c |
|    28 | #cc0044 | crimson            * | Like X11 colour #dc143c       |
|    32 | #004400 | darkgreen          + | Like X11 colour #006400       |
|    36 | #444400 | darkolive            | Like XKCD colour #373e02      |
|    40 | #004444 | darkteal             | Like XKCD colour #014d4e      |
|    44 | #444444 | darkgrey           + | Darker than X11 'dimgray'     |
|    48 | #884400 | brown              * | Like Wikipedia colour #964b00 |
|    52 | #cc4400 | mahogany             | Like Wikipedia colour #c04000 |
|    56 | #884444 | cordovan             | Like Wikipedia colour #893f45 |
|    60 | #cc4444 | brickred             | Like Wikipedia colour #cb4154 |
|    64 | #008800 | green              * | Like HTML colour #008000      |
|    68 | #448800 | avocado              | Like Wikipedia colour #568203 |
|    72 | #008844 | pigmentgreen         | Like Wikipedia colour #00a550 |
|    76 | #448844 | ferngreen            | Like Wikipedia colour #4f7942 |
|    80 | #888800 | olive              * | Like HTML colour #808000      |
|    84 | #cc8800 | harvestgold          | Like Wikipedia colour #da9100 |
|    88 | #888844 | darktan              | Like Wikipedia colour #918151 |
|    92 | #cc8844 | peru               * | Like X11 colour #cd853f       |
|    96 | #00cc00 | mediumgreen          | By analogy with 'mediumblue'  |
|   100 | #44cc00 | napiergreen          | Like Wikipedia colour #2a8000 |
|   104 | #00cc44 | darkpastelgreen      | Like Wikipedia colour #03c03c |
|   108 | #44cc44 | limegreen          * | Like X11 colour #32cd32       |
|   112 | #88cc00 | applegreen           | Like Wikipedia colour #8db600 |
|   116 | #cccc00 | peridot              | Like Wikipedia colour #e6e200 |
|   120 | #88cc44 | yellowgreen        * | Like X11 colour #9acd32       |
|   124 | #cccc44 | oldgold              | Like Wikipedia colour #cfb53b |
|   128 | #000088 | navy               * | Like HTML colour #000080      |
|   132 | #440088 | indigo             * | Like X11 colour #4b0082       |
|   136 | #0000cc | mediumblue         * | Like X11 colour #0000cd       |
|   140 | #4400cc | violetblue           | Like XKCD colour #510ac9      |
|   144 | #880088 | purple             * | Like HTML colour #800080      |
|   148 | #cc0088 | mediumvioletred    * | Like X11 colour #c71585       |
|   152 | #8800cc | darkviolet         * | Like X11 colour #9400d3       |
|   156 | #cc00cc | deepmagenta          | Like Wikipedia colour #cc00cc |
|   160 | #004488 | mediumelectricblue   | Like Wikipedia colour #035096 |
|   164 | #444488 | darkslateblue      * | Like X11 colour #483d8b       |
|   168 | #0044cc | royalazure           | Like Wikipedia colour #0038a8 |
|   172 | #4444cc | pigmentblue          | Like Wikipedia colour #333399 |
|   176 | #884488 | plum               + | Like Wikipedia colour #8e4585 |
|   180 | #cc4488 | mulberry             | Like Wikipedia colour #c54b8c |
|   184 | #8844cc | lavenderindigo       | Like Wikipedia colour #9457eb |
|   188 | #cc44cc | deepfuchsia          | Like Wikipedia colour #c154c1 |
|   192 | #008888 | teal               * | Like HTML colour #008080      |
|   196 | #448888 | dustyteal            | Like XKCD colour #4c9085      |
|   200 | #0088cc | honolulublue         | Like Wikipedia colour #007fbf |
|   204 | #4488cc | celestialblue        | Like Wikipedia colour #4997d0 |
|   208 | #888888 | grey               * | Like HTML colour #808080      |
|   212 | #cc8888 | oldrose              | Like Wikipedia colour #c08081 |
|   216 | #8888cc | ube                  | Like Wikipedia colour #8878c3 |
|   220 | #cc88cc | pastelviolet         | Like Wikipedia colour #cb99c9 |
|   224 | #00cc88 | caribbeangreen       | Like Wikipedia colour #00cc99 |
|   228 | #44cc88 | mint                 | Like Wikipedia colour #3eb479 |
|   232 | #00cccc | darkturquoise      * | Like X11 colour #00ced1       |
|   236 | #44cccc | mediumturquoise    * | Like X11 colour #48d1cc       |
|   240 | #88cc88 | darkseagreen       * | Like X11 colour #8fbc8f       |
|   244 | #cccc88 | lightbeige           | Like Ford colour #d2d08e      |
|   248 | #88cccc | pearlaqua            | Like Wikipedia colour #88d8c0 |
|   252 | #cccccc | lightgrey          * | Like X11 colour #d3d3d3       |

 '*' means that a colour has the same name as one of the standard X11 colours
     and closely resembles it.

 '+' means that a colour has the same name as one of the standard X11 colours
     but is significantly darker.

-----------------------------------------------------------------------------
8   File formats
----------------

8.1 Compression format
----------------------
  The game's data files are encoded using Gordon Key's esoteric lossless
compression algorithm.

  The first 4 bytes of a compressed file give the expected size of the data
when decompressed, as a 32 bit signed little-endian integer. Gordon Key's
file decompression module 'FDComp', which is presumably normative, rejects
input files where the top bit of the fourth byte is set (i.e. negative
values).

  Thereafter, the compressed data consists of tightly packed groups of 1, 8
or 9 bits without any padding between them or alignment with byte boundaries.
A decompressor must deal with two main types of directive: The first (store a
byte) consists of 1+8=9 bits and the second (copy previously-decompressed
data) consists of 1+9+8=18 or 1+9+9=19 bits.

The type of each directive is determined by whether its first bit is set:

0.   The next 8 bits of the input file give a literal byte value (0-255) to
   be written at the current output position.

     When attempting to compress input that contains few repeating patterns,
   the output may actually be larger than the input because each byte value
   is encoded using 9 rather than 8 bits.

1.   The next 9 bits of the input file give an offset (0-511) within the data
   already decompressed, relative to a point 512 bytes behind the current
   output position.

     If the offset is greater than or equal to 256 (i.e. within the last 256
   bytes written) then the next 8 bits give the number of bytes (0-255) to be
   copied to the current output position. Otherwise, the number of bytes
   (0-511) to be copied is encoded using 9 bits.

     If the read pointer is before the start of the output buffer then zeros
   should be written at the output position until it becomes valid again. This
   is a legitimate method of initialising areas of memory with zeros.

     It isn't possible to replicate the whole of the preceding 512 bytes in
   one operation.

The decompressors written by the Fourth Dimension and David O'Shea always
copy at least 1 byte from the source offset, even if the compressed bitstream
specified 0 as the number of bytes to be copied. A well-written compressor
should not insert directives to copy 0 bytes and no instances are known in
the wild. CBLibrary's decompressor treats directives to copy 0 bytes as
invalid input.

8.2 Graphics file
-----------------
  When decompressed, the content is interpreted as follows.

  The number of objects in a polygonal objects set can only be found by
iterating over them until the end-of-file marker is encountered. Their order
is significant relative to other objects of the same type because certain
object numbers are treated specially by the game engine.

  The maximum possible size of a polygonal objects set is limited by the
fact that the game uses a buffer size of 256 KB. Also, the size of the ARM
code generated from a polygonal objects set must not exceed 200 KB. The
relationship between source data and generated code size is complex.

  A polygonal objects set starts with shared command sequences used to plot
complex objects (i.e. those that do not use plot type 0). Each plot type is
defined as a sequence of commands that specify polygon groups to be plotted
and normal vectors to be tested to determine whether a whole group is back-
facing and can be culled (see below for format).

  This data is a variable length sequence of bytes with no alignment
requirements. The command sequence for each plot type is terminated by byte
value 255. There is assumed to be at least one plot type. No more than 10
plot types may be defined without causing a buffer overflow. The terminator
of the last command sequence must be followed by byte value 254.

  The object models follow the plot type definitions at the first word
aligned address at least 4 bytes ahead of the '254' terminator. There is
assumed to be at least one object. No more than 64 ground objects, 64 bit
objects and 32 ship objects may be defined without causing a buffer overflow.

  Each object definition begins with explosion data (see below for format).
The size of this data is variable: 4 bytes + 36 bytes per explosion line.

  Most of the object's attributes are defined in the 12 bytes immediately
following the explosion data, beginning with its type (see below for format).
The start address will be naturally word-aligned.

  The object's vertex coordinates are defined immediately after the rest of
its attributes (see 1.2.4 for format). The size of this data is 1 byte +
3 bytes per vertex. The start address will be naturally word-aligned.

  The object's clip distance and polygon data (see below for format) follow
at the first word-aligned address after the vertex data. The size of this
data is 5 bytes + 2 bytes per polygon + 1 byte per polygon side.

  The object's collision data (see below for format) follows at the first
word-aligned address after the polygon data. The size of this data is 12
bytes + 28 bytes per collision box. There is assumed to be at least one
collision box.

  There is a gap of 4 bytes between the last collision box and the start of
the next object definition or the end-of-file marker (word value 99). The
start address will be naturally word-aligned.

In summary, the layout of a polygonal objects set is as follows:

 * Plot type definitions
 * For each object:
   * Explosion data
   * Miscellaneous attributes
   * Vertex data
   * Clip distance
   * Polygon data
   * Collision data

Plot type definitions:

  There can be no more than 10 plot type definitions. This is an arbitrary
internal limit since 4 bits are allocated for the plot type in an object's
attributes.

  A plot command may be one or two bytes in size. The second byte usually
specifies a polygon group to be plotted if the result of a normal vector test
is favourable, but the group number is instead encoded in bits 0-4 of the
first byte if the action is 0 (always plot facing polygons).

  Although the same plot types are shared by all object models, the
polygon group and vector test numbers in plot commands are interpreted with
reference to the object being plotted. The number of polygon groups and
vector tests for an object depends upon its polygon definitions.

  If a command's action is greater than 0 then bits 0-4 specify a vector
test. This is the index of a polygon in group 7, derived from the order of
polygon definition. The game calculates the z component of the normal vector
of the plane on which the polygon sits. If the result is negative then the
vector test fails; otherwise it passes. The outcome affects all polygons in
the group to be plotted.

  If a command's action is less than 3 and the specified polygon group was
not culled then normal vectors will also be calculated for individual
polygons to determine whether they are facing the camera.

|  Offset | Size | Data
|---------|------|--------------------------------------------------------------
|       0 |    1 | Bits 0-4: Operand - vector test governing whether or not to cull the polygon group (if action is not 0) 
|         |      | OR
|         |      | polygon group number (0..6) to be plotted unconditionally (if action is 0).
|         |      | Bits 5-7: Action - specifies conditions for culling the group and whether to also cull individual backfacing polygons.
|       1 |    1 | Polygon group no. (0..6) to be plotted if action is not 0.

Plot command actions:

|  Number | Action
|---------|---------------------------------------------------------------
|       0 | Always plot facing polygons in the group
|       1 | Plot facing polygons in the group if the vector test passes
|       2 | Plot facing polygons in the group if the vector test fails
|       3 | Plot all polygons in the group if the vector test passes
|       4 | Plot all polygons in the group if the vector test fails
|    5..6 | Illegal (but behave like action 4)
|       7 | End of commands for plot type if operand is 31, else illegal

Explosions:

|  Offset | Size | Data
|---------|------|---------------------------------------
|       0 |    4 | Last line number in explosion (signed)
|       4 |    4 | Undefined
|       8 |  36n | Explosion line data (see format below)
|       8 |   36 | Line 0
|      44 |   36 | Line 1...

Explosion line:

|  Offset | Size | Data
|---------|------|-----------------------
|     0   |    4 | x0 coordinate (signed)
|     4   |    4 | y0 coordinate (signed)
|     8   |    4 | z0 coordinate (signed)
|     12  |    4 | x1 coordinate (signed)
|     16  |    4 | y1 coordinate (signed)
|     20  |    4 | z1 coordinate (signed)
|     24  |    4 | Colour 1
|     28  |    4 | Colour 2
|     32  |    4 | Number of groups

  The x and y coordinates are sign-inverted upon loading.

Object attributes:

|  Offset  | Size | Data
|----------|------|---------------------------------------------------------
|        0 |    1 | Type (0 = Ground, 1 = Bit, 2 = Ship)
|        1 |    1 | Coordinates scale (base-2 logarithm: 0..2)
|        2 |    1 | Number of static vertices (minimum is 1) before all coordinates are rotated around the Z axis. 0 means no rotation.
|        3 |    1 | Bits 0-3: y size of collision area in map tiles (ground objects only)
|          |      | Bits 4-7: x size of collision area in map tiles (ground objects only)
|        4 |    2 | x clip size/2 (unsigned, ship or ground objects only)
|        6 |    2 | y clip size/2 (unsigned, ship or ground objects only)
|        8 |    1 | Score/25 (ship or ground objects only)
|        9 |    1 | Number of hits a ground object can take
|          |      |   OR
|          |      | minimum altitude/262144 for ship objects 13-15
|       10 |   1  | Explosion style (ship or ground objects only)
|       11 |   1  | Bits 0-3: Plot type
|          |      | Bits 4-7: Highest polygon group number (0..6)

Vertexes:

  A left-handed coordinate system is used to specify vertex positions. Ground
level is the xy plane with z=0 and all negative z coordinate values are
above ground.

  A vertex's coordinates are encoded as base-2 logarithmic offsets from the
position of the preceding vertex, therefore the order of vertex definition is
significant. The first vertex's coordinates are relative to the object's
centre. The offset range is +/-32 in each dimension.

|  Offset | Size | Data
|---------|------|---------------------------------------
|       0 |    1 | Number of vertices in object
|       1 |   3n | Vertex definitions (see format below)
|       1 |    3 | Vertex 0
|       4 |    3 | Vertex 1..

Vertex definition:

|  Offset | Size | Data
|---------|------|--------------------------------------------------------------
|       0 |    1 | x offset from previous vertex (85..90 or 96..104 or 110..115)
|       1 |    1 | y offset from previous vertex
|       2 |    1 | z offset from previous vertex

Interpretation of coordinate offsets:

|  Coordinate offset | Interpretation
|--------------------|--------------------------------
|            85...90 | -2 to the power of (90-n)
|            96...99 | -1 / (2 to the power of (n-95)
|                100 | 0
|          101...104 | +1 / (2 to the power of (105-n)
|          110...115 | +2 to the power of (n-110)

Clip distance and polygons:

  Polygons are defined as a set of vertex indices and a colour for flat-
shading. Polygons have the same number of vertexes as sides, and they are
assumed to have at least 3 sides. Vertex indices are derived from the order
of vertex definition.

|  Offset | Size | Data
|---------|------|--------------------------------------------------------
|       0 |    4 | Clip distance (signed). Polygons may need to be clipped if the object is closer to the camera.
|       4 |    1 | Number of polygons in object
|       5 |      | Polygon definitions (see format below)...

Polygon definition:

|  Offset | Size | Data
|---------|------|----------------------------------------------------------
|       0 |    1 | Bits 0-3: Number of sides (n)
|         |      | Bits 4-6: Group number (0..6 = Polygon group, 7 = Vector test group)
|         |      | Bit 7: Most significant bit of the colour index (see 8.3)
|       1 |    1 | First vertex index
|       2 |    1 | Second vertex index...
|     1+n |    1 | Least significant 8 bits of the colour index (see 8.3)

Collision boxes:

  If x0 > x1 or y0 > y1 or z0 > z1 then the reversed coordinate pair will be
swapped to ensure the collision box does not have a negative size in any
dimension.

  The 7 least significant bits of coordinate values are discarded to allow
them to be encoded as immediate constants in ARM instructions. This is
equivalent to signed division rounding toward negative infinity (not zero).
If the value is greater than 32,767 or less than -32,640 then the 9 least
significant bits will be discarded. Values greater than 131,071 or less than
-130,560 are illegal because they cannot be encoded as an 8 bit value
left-shifted by 9 bits.

|  Offset | Size | Data
|---------|------|-----------------------------------
|       0 |    4 | Last collision box number (signed)
|       4 |    8 | Undefined
|      12 |  28n | Collision boxes (see format below)
|      12 |   28 | Collision box 0
|      40 |   28 | Collision box 1...

Collision box:

|  Offset | Size | Data
|---------|------|------------------------------------------
|       0 |    1 | Type (unused)
|       1 |    3 | Undefined
|       4 |    4 | x0 coordinate (signed; -130560 to 131071)
|       8 |    4 | y0 coordinate (signed; -130560 to 131071)
|      12 |    4 | z0 coordinate (signed; -130560 to 131071)
|      16 |    4 | x1 coordinate (signed; -130560 to 131071)
|      20 |    4 | y1 coordinate (signed; -130560 to 131071)
|      24 |    4 | z1 coordinate (signed; -130560 to 131071)

8.3 Palette file
----------------
  The game's palette files map the 320 logical colours used for polygonal
objects to the 256 colours which are actually available in screen mode 13.
The extra 64 logical colours are used for flashing lights and different ship
liveries.

  When decompressed, the content is interpreted as follows.

|  Offset | Size | Data
|---------|------|----------------------------------------------------
|       0 |  256 | Static colours (standard 256 colours)
|     256 |    4 | Player's engine (4 colours)
|     260 |    4 | Fighter's engine (4 colours)
|     264 |    4 | Cruiser's engine (4 colours)
|     268 |    4 | Super fighter's engine (4 colours)
|     272 |    4 | Enemy ships fast flashing lights (4 colours)
|     276 |    4 | Friendly ships fast flashing lights (4 colours)
|     280 |    4 | Player's ship fast flashing lights (4 colours)
|     284 |    4 | Ground object 1 medium flashing lights (4 colours)
|     288 |    4 | Ground object 2 medium flashing lights (4 colours)
|     292 |    4 | Miscellaneous medium flashing lights (4 colours)
|     296 |    4 | Miscellaneous medium flashing lights (4 colours)
|     300 |   20 | Player's ship livery

  Physical colours are encoded using an additive RGB model with 4 bits
per colour component. However, the 2 least-significant bits of each
component (the 'tint' bits) must be equal for all three components.

Bit  | 7      | 6      | 5      | 4      | 3      | 2      | 1      | 0
-----|--------|--------|--------|--------|--------|--------|--------|-------
Role | B high | G high | G low  | R high | B low  | R low  | T high | T low

```
Red component:   (R high << 3) + (R low << 2) + (T high << 1) + T low
Green component: (G high << 3) + (G low << 2) + (T high << 1) + T low
Blue component:  (B high << 3) + (B low << 2) + (T high << 1) + T low
```
('<< n' means 'left-shift by n binary places'.)

For example, 42 (binary 00101010):
```
B high 0, G high 0, G low 1, R high 0, B low 1, R low 0, T high 1, T low 0
Red:   0010 = 2/15 = 0.13
Green: 0110 = 6/15 = 0.4
Blue:  0110 = 6/15 = 0.4
```
This is similar to the SVG/HTML/CSS colour named 'Teal' (0.0, 0.50, 0.50).

-----------------------------------------------------------------------------
9   Program history
-------------------

0.01 (02 Dec 2016)

- First public release.

0.02 (11 Dec 2016)

- Simplified error handling by validating all command-line arguments for
  single file mode before loading any palette file specified by a switch.

- Corrected the description of the syntax for single file mode (an input
  file name isn't required unless an output file name is specified).

- Added a few more assertions and const-qualifiers.

0.03 (17 Apr 2017)

- Duplicate vertices are now removed unless a command-line switch is
  specified to force them to be kept.

- Overlapping coplanar polygons can now be clipped to prevent z-fighting.

- False colours can optionally be assigned to polygons for debugging.

0.04 (29 Aug 2018)

- Z coordinate values are now sign-inverted during conversion in order to
  fix the appearance of assymetric models. This has the useful side-effect
  that ground objects are no longer defined in the negative Z coordinate range.

- Renamed the one_coin object model as three_coin because collecting it
  awards the player three credits. (There is no one-credit coin in the game.)

- Fixed an error in the coplanar polygon clipping algorithm: when deciding
  whether or not to subdivide a back polygon, intersections with an edge of
  the front polygon are no longer ignored when coincident with one of the back
  polygon's corners. This manifested as overlapping polygons in the ground_9
  model of the input file 'Earth1'.

- Fixed arithmetic error in a point-in-polygon subroutine used by the
  coplanar polygon clipping code, which caused points outside polygons with
  near-horizontal top edges to be wrongly be reported as inside. The following
  models were affected: three_coin, ten_coin, life_coin, fifty_coin,
  twenty_coin, atg_coin, ground_10,11,13..16,18 (warrior), mothership (space),
  and ground_13 (space).

- Fixed a bug where the input file was unintentionally closed after
  reading from the standard stream.

- Added support for specifying numeric ranges of object models and logical
  colours.

- Added support for raw (uncompressed) input.

- Removed a restriction that polygons were only clipped against other
  polygons in the same group.

- Generation of human-readable material names by SF3KtoObj is now optional
  when a palette file is specified instead of being enabled automatically.
  This is to align with ChocToObj, which cannot use palette files.

- Moved various helper functions into CBLibrary and created a new library
  from the generic 3D model subroutines of SF3KtoObj.

- The two makefiles now import lists of source objects from a common file.

Version 0.05 (31 Aug 2018)

- Fixed a bug where any new vertices created by clipping polygons were
  omitted from the OBJ-format output if SF3KtoObj was invoked with the
  switches -clip -unused.

Version 0.06 (17 Nov 2018)

- Adapted to use 3DObjLib, CBUtilLib, StreamLib and GKeyLib instead of the
  monolithic CBLibrary previously required. The is_switch function can be
  found in CBUtilLib therefore a local version of it was deleted.

Version 0.07 (21 Apr 2020)

- Failure to close the output stream is now detected and treated like any
  other error since data may have been lost.

Version 0.08 (23 May 2024)

- Added Makefile and updated README for compiling on Linux.

Version 0.09 (11 Apr 2025)

- Dogfooding the _Optional qualifier.

Version 0.10 (13 Apr 2025)
- Use the renamed output_primitives_get_(colour|material) types.

-----------------------------------------------------------------------------
10   Compiling the software
---------------------------

  Source code is only supplied for the command-line programs. To compile
and link the code you will also require an ISO 9899:1999 standard 'C'
library and four of my own libraries: 3dObjLib, CBUtilLib, StreamLib and
GKeyLib. These are available separately from https://github.com/chrisbazley

  Three make files are supplied:

1. 'Makefile' is intended for use with GNU Make and the GNU C Compiler on Linux.

2. 'NMakefile' is intended for use with Acorn Make Utility (AMU) and the
   Norcroft C compiler supplied with the Acorn C/C++ Development Suite.

3. 'GMakefile' is intended for use with GNU Make and the GNU C Compiler on RISC OS.

  The APCS variant specified for the Norcroft compiler is 32 bit for
compatibility with ARMv5 and fpe2 for compatibility with older versions of
the floating point emulator. Generation of unaligned data loads/stores is
disabled for compatibility with ARMv6. When building the code for release,
it is linked with RISCOS Ltd's generic C library stubs ('StubsG').

  Before compiling the library for RISC OS, move the C source and header
files with .c and .h suffixes into subdirectories named 'c' and 'h' and
remove those suffixes from their names. You probably also need to create
'o', 'd' and 'debug' subdirectories for compiler output.

  The only platform-specific code is the PATH_SEPARATOR and EXT_SEPARATOR
macro definitions in misc.h. These must be defined according to the file
path convention on the the target platform (e.g. '\\' and '.' for DOS or
Windows).

-----------------------------------------------------------------------------
11  Licence and Disclaimer
--------------------------

  These programs are free software; you can redistribute them and/or modify
them under the terms of the GNU General Public Licence as published by the
Free Software Foundation; either version 2 of the Licence, or (at your
option) any later version.

  These programs are distributed in the hope that they will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public Licence for
more details.

  You should have received a copy of the GNU General Public Licence along
with these programs; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.

  These programs use CBLibrary and 3dObjLib, which are (C) 2018 Christopher
Bazley. These libraries and their use are covered by the GNU Lesser General
Public Licence.

-----------------------------------------------------------------------------
12  Credits
-----------

  SF3KtoObj and SF3KtoMtl were designed and programmed by Christopher Bazley.

  My information on the Wavefront Object and Material library file formats
came from Paul Bourke's copies of the file format specifications for the
Advanced Visualizer software (Copyright 1995 Alias|Wavefront, Inc.):
  http://paulbourke.net/dataformats/obj/
  http://paulbourke.net/dataformats/mtl/

  Some colour names were taken from a survey run by the web comic XKCD. The
data is in the public domain, available under a Creative Commons licence:
https://blog.xkcd.com/2010/05/03/color-survey-results/
Thanks to Keith McKillop for suggesting this source.

  The game Star Fighter 3000 is (C) FEDNET Software 1994, 1995.

-----------------------------------------------------------------------------
13  Contact details
-------------------

  Feel free to contact me with any bug reports, suggestions or anything else.

  Email: mailto:cs99cjb@gmail.com

  WWW:   http://starfighter.acornarcade.com
