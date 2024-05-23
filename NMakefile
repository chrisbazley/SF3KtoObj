# Project:   SF3KtoObj

# Tools
CC = cc
Link = link

# Toolflags:
CCCommonFlags = -c -depend !Depend -IC: -throwback -fahi -apcs 3/32/fpe2/swst/fp/nofpr -memaccess -L22-S22-L41
CCflags = $(CCCommonFlags) -DNDEBUG -Otime
CCDebugFlags = $(CCCommonFlags) -g -DUSE_CBDEBUG -DFORTIFY -DDEBUG_OUTPUT
Linkflags = -aif
LinkDebugFlags = $(Linkflags) -d

include MakeCommon

DebugObjectsObj = $(addprefix debug.,$(ObjectListObj))
ReleaseObjectsObj = $(addprefix o.,$(ObjectListObj))
DebugObjectsMtl = $(addprefix debug.,$(ObjectListMtl))
ReleaseObjectsMtl = $(addprefix o.,$(ObjectListMtl))
DebugLibs = C:o.Stubs Fortify:o.fortify C:o.CBDebugLib C:debug.CBUtilLib C:debug.3dObjLib C:debug.GKeyLib C:debug.StreamLib
ReleaseLibs = C:o.StubsG C:o.CBUtilLib C:o.3dObjLib C:o.GKeyLib C:o.StreamLib

# Final targets:
all: SF3KtoMtl SF3KtoObj SF3KtoMtlD SF3KtoObjD

SF3KtoObj: $(ReleaseObjectsObj)
	$(Link) $(LinkFlags) -o $@ $(ReleaseObjectsObj) $(ReleaseLibs)

SF3KtoObjD: $(DebugObjectsObj)
	$(Link) $(LinkDebugFlags) -o $@ $(DebugObjectsObj) $(DebugLibs)

SF3KtoMtl: $(ReleaseObjectsMtl)
	$(Link) $(LinkFlags) -o $@ $(ReleaseObjectsMtl) $(ReleaseLibs)

SF3KtoMtlD: $(DebugObjectsMtl)
	$(Link) $(LinkDebugFlags) -o $@ $(DebugObjectsMtl) $(DebugLibs)

# User-editable dependencies:
.SUFFIXES: .o .c .debug
.c.o:; $(CC) $(CCflags) -o $@ $<
.c.debug:; $(CC) $(CCDebugFlags) -o $@ $<

# Static dependencies:

# Dynamic dependencies:
