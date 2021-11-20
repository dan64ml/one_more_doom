// The base class for all types of map objects.
// Objects of this class can be put in the game world,
// can be drawn by the renderer and support animation.
// This functionality is enough for almost all fixed 
// objects.

#ifndef MAP_OBJECT_
#define MAP_OBJECT_

#include <string>

#include "mobj_flags.h"
#include "mobj_defs.h"

#include "renderer/bam.h"
#include "fsm.h"
#include "info.h"

namespace world {
  class World;
  class Line;
  class SubSector;
}

namespace mobj {

struct MapObject {
 public:
  MapObject(id::mobjtype_t type);

  // World coordinates of the object
  double x;
  double y;
  double z;

  // View direction
  rend::BamAngle angle;

  // Sizes of the mobj for collisions detection
  // NB! Sizes for drawing are diffrent and must be get from texture sizes
  double height;
  double radius;

  double speed;
  
  // Original DOOM flags
  uint32_t flags;
  // Original DOOM type
  id::mobjtype_t mobj_type;

  // Current subsector
  world::SubSector* ss_;

  // Returns false if the object should be deleted
  virtual bool TickTime();

  // Returns sprite name considering the view point
  std::string GetSpriteName(int vp_x, int vp_y) const;

  int GetHealth() const { return health_; }

  // Just created mobj should be "tied" to game world
  virtual void TieToMap(world::World* world, world::SubSector* ss, bool keep_z = false);

  void CauseDamage(int damage);
  // Structure objects influence in a bit different way.
  void DamageBySobj(int damage);

  // Returns false if the mobj doesn't fit the space, otherwise changes z coordinate.
  // Base class uses simplified algorithm, because shootable mobjs shouldn't appear here.
  virtual bool AdjustMobjHeight();

  // In fact this foo makes sense only for player, but it requires when monsters
  // try to open doors
  virtual bool IsCardPresent([[maybe_unused]] CardType c) const { return false; }

 protected:
  FSM fsm_;

  int health_;
  int spawn_health_;
  int pain_chance_;

  // Flag to delete the mobj
  bool delete_me_ = false;

  // The narrowest part over all contacted Sectors.
  // Calculated during checking lines intersection
  double floor_z_;
  double ceiling_z_;

  world::World* world_;

  // Change current subsector
  bool ChangeSubSector(int new_x, int new_y);

  // TODO: Candidate to be removed
  virtual void CallStateFunction([[maybe_unused]] id::FuncId foo_id) {}

  friend class FSM;
};

} // namespace mobj

#endif  // MAP_OBJECT_
