# Project:   SF3KtoObj

# Tools
CC = gcc
Link = gcc

# Toolflags:
CCCommonFlags = -c -Wall -Wextra -Wsign-compare -pedantic -std=c99 -MMD -MP -MF $*.d -o $@
CCFlags = $(CCCommonFlags) -DNDEBUG -O3
CCDebugFlags = $(CCCommonFlags) -g -DUSE_CBDEBUG -DDEBUG_OUTPUT -DFORTIFY
LinkCommonFlags = -o $@
LinkFlags = $(LinkCommonFlags) $(addprefix -l,$(ReleaseLibs))
LinkDebugFlags = $(LinkCommonFlags) $(addprefix -l,$(DebugLibs))

include MakeCommon

DebugObjectsObj = $(addsuffix .debug,$(ObjectListObj))
ReleaseObjectsObj = $(addsuffix .o,$(ObjectListObj))
DebugObjectsChoc = $(addsuffix .debug,$(ObjectListChoc))
ReleaseObjectsChoc = $(addsuffix .o,$(ObjectListChoc))
DebugObjectsMtl = $(addsuffix .debug,$(ObjectListMtl))
ReleaseObjectsMtl = $(addsuffix .o,$(ObjectListMtl))
DebugLibs = CBUtildbg Streamdbg GKeydbg 3dObjdbg
ReleaseLibs = CBUtil Stream GKey 3dObj

# Final targets:
all: SF3KtoMtl SF3KtoObj SF3KtoMtlD SF3KtoObjD

SF3KtoObj: $(ReleaseObjectsObj)
	$(Link) $(ReleaseObjectsObj) $(LinkFlags)

SF3KtoObjD: $(DebugObjectsObj)
	$(Link) $(DebugObjectsObj) $(LinkDebugFlags)

SF3KtoMtl: $(ReleaseObjectsMtl)
	$(Link) $(ReleaseObjectsMtl) $(LinkFlags)

SF3KtoMtlD: $(DebugObjectsMtl)
	$(Link) $(DebugObjectsMtl) $(LinkDebugFlags)


# User-editable dependencies:
.SUFFIXES: .o .c .debug
.c.debug:
	$(CC) $(CCDebugFlags) $<
.c.o:
	$(CC) $(CCFlags) $<

# Static dependencies:

# Dynamic dependencies:
# These files are generated during compilation to track C header #includes.
# It's not an error if they don't exist.
-include $(addsuffix .d,$(ObjectListObj))
-include $(addsuffix D.d,$(ObjectListObj))
-include $(addsuffix .d,$(ObjectListMtl))
-include $(addsuffix D.d,$(ObjectListMtl))
