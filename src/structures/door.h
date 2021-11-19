#ifndef DOOR_H_
#define DOOR_H_

#include "structure_object.h"

#include "sobj_types.h"

#include "world/world_types.h"

namespace sobj {

enum class DoorType {
  kNormal,
  kClose30ThenOpen,
  kClose,
  kOpen,
  kRaiseIn5Mins,
  kBlazeRaise,
  kBlazeOpen,
  kBlazeClose
};

class Door : public StructureObject {
 public:
  Door(world::World* w, world::Sector* s, world::Line* l, DoorType type);
  ~Door();

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

 private:
  const double kDoorSpeed = 2.0;
  const int kDoorWaitTime = 150;

  world::World* world_;
  world::Sector* sector_;
  world::Line* line_;
  DoorType type_;

  double speed_;
  // Opened door height depends on neighbor sectors
  double top_pos_;
  MoveDirection direction_;

  // Wait time in ticks.
  int wait_time_;
  // Counter
  int wait_counter_;
  
  //bool CanChangeHeight(int ceiling_height);
};

} // sobj

#endif  // DOOR_H_
