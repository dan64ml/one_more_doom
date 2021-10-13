#ifndef MAP_OBJECT_
#define MAP_OBJECT_

#include <cstdint>
#include <string>

#include "mobj_flags.h"
#include "renderer/bam.h"
#include "fsm.h"
#include "info.h"

namespace world {
  class World;
  class Line;
  class SubSector;
}

namespace mobj {

enum CardType {
  kBlueCard,
  kYellowCard,
  kRedCard,
  kBlueScull,
  kYellowScull,
  kRedScull,

  kCardsNumber
};

const int kMaxRadius = 32;
const double kGravity = 1.0;

const int kMissileRange = 32 * 64;
const int kMeleeRange = 64;

const int kWeaponHeight = 42; // Why??!

/*=========================================================================
Need access to:
- BlockMap class data
- FastBsp
===========================================================================*/
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

  int speed;
  
  // Original DOOM flags
  uint32_t flags;
  // Original DOOM type
  id::mobjtype_t mobj_type;

  // Momentums, in fact speed in strange units
  double mom_x = 0;
  double mom_y = 0;
  double mom_z = 0;

  // The narrowest part over all contacted Sectors.
  // Calculated during checking lines intersection
  double floor_z;
  double ceiling_z;

  // Current subsector
  // TODO: !!!! CHANGE THE NAME !!!
  world::SubSector* ss;

  world::World* world_;


  // Returns false if the object should be deleted
  virtual bool TickTime();

  std::string GetSpriteName(int vp_x, int vp_y) const;
  int GetHealth() const { return health_; }

  void CauseDamage(int damage);
  // Structure objects influence in a bit different way. Ref to ChangeSectorHeight().
  void DamageBySobj(int damage);

  // In fact this foo makes sense only for player
  virtual bool IsCardPresent([[maybe_unused]] CardType c) const { return false; }

  // Checks current position and updates floor_z, ceiling_z and dropoff_z;
  void UpdateOpening();

 private:
  double tmp_ceiling;
  double tmp_floor;
  double tmp_dropoff;

  std::vector<world::Line*> spec_lines_;

 protected:
  // Invoked if the object run into sth. XYMove() will be interrupted
  // immediately if return false.
  // Defines common action after hit, e.g. sliding for player, missile explosion etc...
  //  For others - just stop moving (???)
  virtual bool RunIntoAction();

  // Contains logic for object speed reducing
  virtual void SlowDown();

  // Applied to each touched MapObject until return false
  // NB! Check for MF_SPECIAL flag
  virtual bool InfluenceObject(MapObject*) { return true; }
  
  // Applied to each crossed line until return false.
  // False means it's impossible to cross the line.
  virtual bool ProcessLine(world::Line* line);

  // Applied to each crossed special line
  virtual void ProcessSpecialLine([[maybe_unused]] world::Line* line) {}

  // It looks like a candidate to be virtual...
  virtual void ZMove();

  virtual void CallStateFunction([[maybe_unused]] id::FuncId foo_id) {}

  FSM fsm_;

  int health_;
  int spawn_health_;
  int pain_chance_;

 private:
  void MoveObject();
  void XYMove();

  // Check the position, interact with items and move
  // the object to the new position if it's possible (and return true)
  // false if moving impossible
  bool TryMoveTo(double new_x, double new_y);

  // Iterates over all mobjs and lines in current and adjacent BlockMaps
  // Iterating lines updates opeining (tmp_* values)
  bool CheckPosition(double new_x, double new_y);

  // Updates tmp_* variables
  void UpdateOpening(const world::Line* line);
  // Change current subsector
  bool ChangeSubSector(world::SubSector* new_ss);

  // Flag to delete the mobj
  bool delete_me_ = false;

  const double kMaxMove = 30;
  const int kMaxStepSize = 24;

  friend class FSM;
};

} // namespace mobj

#endif  // MAP_OBJECT_
