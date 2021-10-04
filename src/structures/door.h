#ifndef DOOR_H_
#define DOOR_H_

#include "structure_object.h"

#include "world/world_types.h"

namespace sobj {

enum class DoorType {
  kOpen,
  kClose,
  kOpenThenClose,
  kCloseThenOpen
};

class Door : public StructureObject {
 public:
  Door(world::World* w, world::Sector* s, DoorType type, int speed, bool wait_obstacle = false);

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

 private:
  world::World* world_;
  world::Sector* sector_;
  DoorType type_;

  // Both moving have the same speed
  int move_speed_;
  // Initial direction. After kWait direction reverses. 1 == move up
  int move_direction_;
  
  // Closed door height
  int floor_level_;
  // Opened door height depends on neighbor sectors
  int door_top_level_;
  // Pausa length in ticks.
  // NB! 0 means that first door position was reached.
  int wait_counter_ = 42;

  // Some door types should wait until obstacles disappear
  // But most doors just return back.
  bool wait_obstacle_ = false;

  // Move ceiling. Returns true when ceiling reached stop level
  //bool MoveCeiling();

  bool CheckObstacles(int ceiling_height);
};

} // sobj

#endif  // DOOR_H_
