#ifndef MAP_OBJECT_
#define MAP_OBJECT_

#include <cstdint>
#include <string>

#include "mobj_flags.h"
#include "renderer/bam.h"
#include "fsm.h"

namespace world {
  class World;
  class Line;
  class SubSector;
}


namespace mobj {

const int kMaxRadius = 32;
const double kGravity = 1.0;

const int kMissileRange = 32 * 64;

const int kWeaponHeight = 42; // Why??!

/*=========================================================================
Need access to:
- BlockMap class data
- FastBsp
===========================================================================*/
struct MapObject {
 public:
  //MapObject(world::World* world) : world_(world) {}
  MapObject(const id::mobjinfo_t& info);
  //MapObject() {}  // TODO!!!!!
  //MapObject(id::mobjtype_t type) : fsm_(id::mobjinfo[type]) {}  // TODO!!!!!

  // World coordinates of the object
  int x;
  int y;
  int z;  

  // View direction
  rend::BamAngle angle;

  // Sizes of the mobj for collisions detection
  // NB! Sizes for drawing are diffrent and must be get from texture sizes
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
  //int dropoff_z;

  // Current subsector
  world::SubSector* ss;

  world::World* world_;

  // For debug
  int type;

  // Returns false if the object should be deleted
  virtual bool TickTime();

  std::string GetSpriteName(int vp_x, int vp_y) const;
  int GetHealth() const { return health_; }

  void CauseDamage(int damage);

 private:
  struct Opening {
    int ceiling;
    int floor;
    int dropoff;

    world::SubSector* ss;
  };
  
 protected:
  // Invoked if the object run into sth. XYMove() will be interrupted
  // immediately if return false.
  // Defines common action after hit, e.g. sliding for player, missile explosion etc...
  virtual bool RunIntoAction();
  // Contains logic for object speed reducing
  virtual void SlowDown();

  // Applied to each touched MapObject until return false
  virtual bool InfluenceObject(MapObject*) { return true; }
  
  // Applied to each crossed line until return false.
  // False means it's impossible to cross the line.
  virtual bool ProcessLine(const world::Line* line);
  // It looks like a candidate to be virtual...
  virtual void ZMove();

  FSM fsm_;

  int health_;
  int spawn_health_;
  int pain_chance_;

 private:
  void MoveObject();
  void XYMove();

  // Check the position, interact with items and move
  // the object to the new position if possible (and return true)
  // false if moving impossible
  bool TryMoveTo(int new_x, int new_y);
  // Iterates over all mobjs and lines in current and adjacent BlockMaps
  bool CheckPosition(int new_x, int new_y, Opening& op);

  void UpdateOpening(Opening& op, const world::Line* line);
  // Change current subsector
  bool ChangeSubSector(world::SubSector* new_ss);

  const double kMaxMove = 30;
  const int kMaxStepSize = 24;
};

} // namespace mobj

#endif  // MAP_OBJECT_
