#ifndef MAP_OBJECT_
#define MAP_OBJECT_

//#include "renderer/plane_utils.h"
#include <cstdint>

namespace mobj {

// The original DOOM flags, no point to change...
enum MapObjectFlag : std::uint32_t {
    // Call P_SpecialThing when touched.
    MF_SPECIAL		= 1,
    // Blocks.
    MF_SOLID		= 2,
    // Can be hit.
    MF_SHOOTABLE	= 4,
    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR		= 8,
    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP	= 16,                    

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH		= 32,
    // Will try to attack right back.
    MF_JUSTHIT		= 64,
    // Will take at least one step before attacking.
    MF_JUSTATTACKED	= 128,
    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING	= 256,
    // Don't apply gravity (every tic),
    //  that is, object will float, keeping current height
    //  or changing it actively.
    MF_NOGRAVITY	= 512,

    // Movement flags.
    // This allows jumps from high places.
    MF_DROPOFF		= 0x400,
    // For players, will pick up items.
    MF_PICKUP		= 0x800,
    // Player cheat. ???
    MF_NOCLIP		= 0x1000,
    // Player: keep info about sliding along walls.
    MF_SLIDE		= 0x2000,
    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT		= 0x4000,
    // Don't cross lines
    //   ??? or look at heights on teleport.
    MF_TELEPORT		= 0x8000,
    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE		= 0x10000,	
    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED		= 0x20000,
    // Use fuzzy draw (shadow demons or spectres),
    //  temporary player invisibility powerup.
    MF_SHADOW		= 0x40000,
    // Flag: don't bleed when shot (use puff),
    //  barrels and shootable furniture shall not bleed.
    MF_NOBLOOD		= 0x80000,
    // Don't stop moving halfway off a step,
    //  that is, have dead bodies slide down all the way.
    MF_CORPSE		= 0x100000,
    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT		= 0x200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL	= 0x400000,
    
    // On picking up, count this item object
    //  towards intermission item total.
    MF_COUNTITEM	= 0x800000,

    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF_SKULLFLY		= 0x1000000,

    // Don't spawn this object
    //  in death match mode (e.g. key cards).
    MF_NOTDMATCH    	= 0x2000000,

    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // If 0x4 0x8 or 0xc,
    //  use a translation table for player colormaps
    MF_TRANSLATION  	= 0xc000000,
    // Hmm ???.
    MF_TRANSSHIFT	= 26
};

/*=========================================================================
Need access to:
- BlockMap class data

===========================================================================*/
struct MapObject {
  // World coordinates of the object
  int x;
  int y;
  int z;

  // View direction
  // TODO: select angle units...
  //rend::BamAngle angle;

  // Sizes of the mobj
  int height;
  int radius;

  // Original DOOM flags
  uint32_t flags;

  // Momentums, in fact speed in strange units
  double mom_x = 0;
  double mom_y = 0;
  double mom_z = 0;

  // The narrowest part over all contacted Sectors.
  // Calculated during checking lines intersection
  int floor_z;
  int ceiling_z;
  // The lowest floor. Necessary for falling check
  int dropoff_z;

  bool TimeTick();
  
 private:
  void MoveObject();
  void XYMove();
  void ZMove();

  // Check the position, interact with items and move
  // the object to the new position if possible (and return true)
  // false if moving impossible
  bool TryMoveTo(int new_x, int new_y);
  // Iterates over all mobjs and lines in current and adjacent BlockMaps
  bool CheckPosition(int new_x, int new_y);

  // Invoked if the object run into sth. XYMove() will be interrupted
  // immediately if return false
  virtual bool RunIntoAction();
  // Contains logic for object speed reducing
  virtual void SlowDown();



  // Applied to each touched MapObject until return false
  virtual bool InfluenceObject(MapObject) {}
  
  // moves this object if the position (x, y) is reachable
  // and returns true, returns false otherwise
  // Applyes InfluenceObject()...
  virtual bool TryPosition(int x, int y) {}

  virtual bool DamageObject(MapObject source, int damage) {}

  const double kMaxMove = 30;
  const int kMaxStepSize = 24;
};

} // namespace mobj

#endif  // MAP_OBJECT_
