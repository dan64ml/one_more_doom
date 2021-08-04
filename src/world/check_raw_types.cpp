#include "wad_raw_types.h"

static_assert(sizeof(wad::WadHeader) == 12, "Wrong size of WadHeader");
static_assert(sizeof(wad::WadDirectoryEntry) == 16, "Wrong size of WadDirectoryEntry");
static_assert(sizeof(wad::WadVertex) == 4, "Wrong size of WadVertex");
static_assert(sizeof(wad::WadLineDef) == 14, "Wrong size of WadLineDef");
static_assert(sizeof(wad::WadSideDef) == 30, "Wrong size of WadSideDef");
static_assert(sizeof(wad::WadSector) == 26, "Wrong size of WadSector");
static_assert(sizeof(wad::WadThing) == 10, "Wrong size of WadThing");
