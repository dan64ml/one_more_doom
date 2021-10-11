#ifndef SOBJ_UTILS_H_
#define SOBJ_UTILS_H_

#include "sobj_types.h"

#include "world/world_types.h"

namespace sobj {

// I have to change some int variables to double. Epsilon for comparing
const double kEps = 0.0001;

// Moves sector's ceiling in the given direction with speed until reach the dest.
// If crush == true cause damage
MoveResult MoveCeiling(world::Sector* sec, double speed, double dest, bool crush, MoveDirection dir);
// Like MoveCeiling()
MoveResult MoveFloor(world::Sector* sec, double speed, double dest, bool crush, MoveDirection dir);

// Adjusts the positions of all mobjs that touch the sector. It's necessary after changing floor/ceiling
// heights.
// Returns true if something doesn't fit to new height. If crush == true such objects take damage.
// NB!!! Sometimes, especially if crush == false and there is unfit mobj, we need to set the sector
// height back by calling this foo with old sector's params...
bool ChangeSectorHeight(world::Sector* sec, bool crush);

// Updates mobj's opening (*_z variables) and changes z if it's necessary.
// Returns false if the mobj doesn't fit the space
bool AdjustMobjHeight(mobj::MapObject* mobj);

} // namespace sobj

#endif  // SOBJ_UTILS_H_
